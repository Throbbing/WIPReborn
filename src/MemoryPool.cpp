#include "..\\RBMath\\Inc\\RBMathUtilities.h"
#include "MemoryPool.h"
#include <memory>


//实现一个malloc：http://www.inf.udec.cl/~leo/Malloc_tutorial.pdf
RBPoolAllctor::RBPoolAllctor()
{
  printf("construct pool\n");
  _bactive = false;
  //64bit默认选择8 byte对齐
  //http://stackoverflow.com/questions/21219130/is-8-byte-alignment-for-double-type-necessary
  //glibc malloc.c对32bit系统对齐为8,64则为16
  //stdlib malloc未要求对齐：http://www.cplusplus.com/reference/cstdlib/malloc/
  //选择必须为8的倍数
  //new出来的内存则是最大object对齐的：http://en.cppreference.com/w/cpp/memory/new/operator_new
  _alignment = 8;
  _total_alloced = 0;
  mm = 0;
}

RBPoolAllctor::~RBPoolAllctor()
{

}

INI_RESULT RBPoolAllctor::init()
{
#ifdef REPORT
  printf("init pool\n");
#endif
  _bactive = true;
  return true;
}

INI_RESULT RBPoolAllctor::shutdown()
{
#ifdef REPORT
  printf("shotdown pool\n");
#endif
  _bactive = false;
  return true;
}

void RBPoolAllctor::new_pool(void** header, size_t single_size,const char* name,u32 hash)
{
#ifdef REPORT
  printf("get a new pool\n");
#endif
  expand_free_list(single_size, header);
  _links.push_back(_LinkHeader(single_size,*header,hash,name));
  //report();
}

void* RBPoolAllctor::alloc(void** header, size_t single_size,u32 hash)
{
  if (NULL == NEXT_NODE(*header))
  {
    expand_free_list(single_size, &NEXT_NODE(*header));
#ifdef REPORT
    printf("NOTE:Memory pool expanded at %p\n", NEXT_NODE(*header));
#endif
  }
  void *r_header = *header;
  *header = NEXT_NODE(*header);

  if (hash)
  {
    
    std::list<_LinkHeader>::iterator iter;
    for (iter = _links.begin(); iter != _links.end();)
    {
      if (hash == iter->hash)
      {
        if (!iter->using_header)
        {
          mm++;
          iter->using_header = r_header;
        }
        else
        {
          NEXT_NODE(iter->last_using) = r_header;
          mm++;
        }
        iter->last_using = r_header;
        NEXT_NODE(iter->last_using) = NULL;
      }
      ++iter;
    }
  }
  return r_header;
}

void RBPoolAllctor::free(void** header, void *p, size_t single_size,u32 hash)
{
  if (hash)
  {
    std::list<_LinkHeader>::iterator iter;
    for (iter = _links.begin(); iter != _links.end();)
    {
      if (hash == iter->hash)
      {
        if (!iter->using_header)
          printf("pointer %d has been broken!\n", p);
        else
        {
          void* pp = iter->using_header;
          void* temp = NULL;
          void* pre = NULL;
          while (NULL != pp)
          {
            temp = NEXT_NODE(pp);
            if (p == pp)
            {
              if (pp == iter->using_header)
              {
                iter->using_header = temp;

              }
              else
              {
                NEXT_NODE(pre) = temp;

              }
              break;
            }
            pre = pp;
            pp = temp;
          }
        }
      }
      ++iter;
    }
  }

  NEXT_NODE(p) = *header;
  *header = p;


}

void RBPoolAllctor::reduce(void** header,size_t single_size)
{

  void* p = *header;
  /*
  std::list<_LinkHeader>::iterator iter;
  for (iter = _links.begin(); iter != _links.end();)
  {
    if (p == iter->header)
      iter = _links.erase(iter);
    else
      ++iter;
  }
  */
  void* temp = NULL;
  while (NULL != p)
  {
    temp = NEXT_NODE(p);
    free_aligned(p);
    _total_alloced -= (single_size+_alignment+sizeof(void*));
    p = temp;
  }
  *header = NULL;
  expand_free_list(single_size, header,1);
  
}

//4/single_size/4
void RBPoolAllctor::expand_free_list(int single_size, void** header, int n /* = g_pool_expand_node_number = 32 */)
{

  CHECK(*header == NULL);
  /*     p
  //XXX4/XXXXXXXXXXXXXXXX.../XXXX
  */
  void* p = NEW_NODE(single_size);
#ifdef REPORT
  printf("return : %02X | \n", p);
#endif
  *header = (void*)p;
  for (int i = 0; i<n - 1; ++i)
  {
    NEXT_NODE(p) = NEW_NODE(single_size);
    p = NEXT_NODE(p);
#ifdef REPORT
    printf("return : %02X | \n", p);
#endif
  }
  NEXT_NODE(p) = NULL;
#ifdef REPORT
  printf("\n");
  report(*header, single_size);
#endif
}

void *RBPoolAllctor::allocate_aligned(size_t tsize)
{
  CHECK(_alignment>1);
  CHECK(RBMath::is_pow_2(_alignment));
  //对齐还必须是指针长的倍数
  CHECK((_alignment%sizeof(void*) == 0));
  //stdlib的mollac分配的内存并没有要求对齐：http://www.cplusplus.com/reference/cstdlib/malloc/
  //但是有的实现可能是已经对齐过的：
  //MSDN:In Visual C++, this is the alignment that's required for a double, or 8 bytes. In code that targets 64-bit platforms, it’s 16 bytes.
  // The storage space pointed to by the return value is guaranteed to be suitably aligned for storage of any type of object.
  //也就是最大尺度object的对齐，即double
  size_t size = tsize + _alignment;
  size_t origin_address = (size_t)::malloc(size);
  _total_alloced += size;
#ifdef REPORT
  printf("original block:%02X ~ %02X ", origin_address, origin_address + size);
#endif
  std::memset((void*)origin_address, 0, size);
  if (!origin_address)
    return nullptr;
  size_t tmask = (_alignment - 1);
  size_t misaligment = (origin_address & tmask);
  //如果本来已经按照8字节对齐，调整是8字节，地址前面将留空8字节（至少要留下1字节），
  //所以在这种实现下，即使本来分配的内存也是对齐的也存在浪费，因为至少留下的1字节是必须的
  size_t adjust = _alignment - misaligment;
  CHECK(adjust>0);

  size_t aligned_address = origin_address + adjust;

  //save alignment adjustment 
  u8 *adjust_ptr = (u8*)aligned_address;
  CHECK(adjust<256);
  adjust_ptr[-1] = (u8)adjust;

  //save next address
  //XXX4/XXXXXXXXXXXXXXXX.../XXXX
  return (void*)aligned_address;
}

//free aligned_address located on '\'(/XXX4\XXXXXXXXXX.../XXXX)
void RBPoolAllctor::free_aligned(void *p)
{
  size_t aligned_address = (size_t)p;
  u8 *adjust_ptr = (u8*)aligned_address;
  size_t adjust = (size_t)adjust_ptr[-1];

  size_t origin_address = aligned_address - adjust;

  ::free((void*)origin_address);

}


void RBPoolAllctor::set_alignment(size_t val)
{
  CHECK(RBMath::is_pow_2(val) && val != 1);
  _alignment = val;
}


void RBPoolAllctor::report(void* header, size_t single_size,u32 hash,bool show_links)
{
  if (!header)
  {
    printf("Pass a NULL header.\n");
    return;
  }

  void* uheader = NULL;
  std::string name;
  std::list<_LinkHeader>::iterator iter;
  for (iter = _links.begin(); iter != _links.end();)
  {
    if (hash == iter->hash)
    {
      uheader = iter->using_header;
      name = iter->name;
    }
    ++iter;
  }

  void* p = uheader;
  if (!p)
  {
    printf("class [%s] alloc no objects\n",name.c_str());
    return;
  }
  size_t ad = (size_t)p;
  size_t adjust = ((u8*)ad)[-1];
  CHECK(adjust > 0 && adjust <= _alignment);
  size_t od = ad - adjust;
  void* nextp = p;
  int total_node = 0;
  printf("links:\n");
  while (nextp)
  {
    total_node++;

    void* nex = (void*)*(size_t*)((size_t)nextp + single_size);
    if (show_links)
    {
      printf("%d %02X --%d--> %02X ",total_node, nextp, single_size + sizeof(size_t), nex);
      size_t adj = ((u8*)nextp)[-1];
      printf("%d ", adj);
      printf("| %02X --%d--> %02X \n", (size_t)nextp - adj, single_size + sizeof(size_t), (size_t)nextp + single_size + sizeof(size_t));
    }
    nextp = nex;
  }
  mm--;
  printf("total nodes:[%d] \n", total_node);
  size_t sz = total_node*(single_size + sizeof(size_t) + _alignment);
  printf("B : %d\n",sz);
  size_t a = (sz >> 20);
  size_t b = (((sz >> 10) - ((sz >> 20) << 10)));
  size_t c = (sz - (b << 10) - (a << 20));
  printf("total memory : [%d M %d K %d B]\n", a, b, c);


}

void RBPoolAllctor::report(bool show_links)
{
  for (auto link:_links)
  {
    printf("class [%s]:\n", link.name.c_str());
    printf("single size: [%d]:\n", link.single_size);
    report(link.using_header, link.single_size,link.hash,show_links);
    printf("-----\n");
  }

  size_t a = (_total_alloced >> 20);
  size_t b = (((_total_alloced >> 10) - ((_total_alloced >> 20) << 10)));
  size_t c = (_total_alloced - (b << 10) - (a << 20));
  printf("System memory used:%d M %d K %d B\n", a, b, c);
  printf("=============\n");

}