#pragma once
#include "RenderResource.h"
#include <map>
#include <string>
#include <list>
#include "FileSystem.h"
#include "ResourceManager.h"
#include "Logger.h"
#include "RBMath/Inc/Vector2.h"


using std::map;
using std::string;
using std::list;

enum class BufferType
{
	E_STATIC_DRAW,
	E_DYNAMIC_DRAW,
	E_STREAM_DRAW
};

class WIPDynamicRHI
{
protected:
	map<string,WIPVertexShader*> _vertex_shaders;
	map<string,WIPPixelShader*> _pixel_shaders;

	WIPViewPort* _active_view_port;

	int _debug_buffer_size;
	unsigned char* _debug_vertex_buffer;
	int _debug_update_count;
public:

	WIPVertexShader* get_vertex_shader(const string& name);
	WIPPixelShader* get_pixel_shader(const string& name);

public:
	static WIPDynamicRHI* get_rhi();

/*
	WIPDynamicRHI(){}
	virtual ~WIPDynamicRHI()=0;
*/
	virtual void init() = 0;
	virtual void post_init(){}
	virtual void shutdown() = 0;


	virtual WIPTexture2D* RHICreateTexture2D(uint32 SizeX, uint32 SizeY,  void* data,uint8 Format=0, uint32 NumMips=0, uint32 NumSamples=0, uint32 Flags=0) = 0;
	virtual void update_texture(WIPTexture2D* texture, void* data) const = 0;
	virtual void update_subrect_texture(WIPTexture2D* texture, int x, int y, int w, int h, void* data) const = 0;
	virtual WIPVertexShader*  RHICreateVertexShader(const char* text) const=0;
	virtual WIPPixelShader*  RHICreatePixelShader(const char* text) const=0;
	virtual WIPBoundShader*  RHICreateBoundShader(WIPVertexShader* vs,WIPPixelShader* ps) const=0;
	virtual WIPVertexBuffer* RHICreateVertexBuffer(unsigned int size,void* data,BufferType tp)=0;
	virtual void* lock_vertex_buffer(WIPVertexBuffer* buffer) const=0;
	virtual void unlock_vertex_buffer(WIPVertexBuffer* buffer) const=0;
	virtual WIPIndexBuffer* RHICreateIndexBuffer(unsigned int size,void* data,BufferType tp)=0;
	virtual void* lock_index_buffer(WIPIndexBuffer* buffer) const=0;
	virtual void unlock_index_buffer(WIPIndexBuffer* buffer) const=0;
	virtual WIPVertexFormat* RHICreateVertexFormat()=0;
	virtual WIPViewPort* RHICreateViewPort(int x, int y, int w, int h) = 0;
	virtual WIPViewPort* change_viewport(WIPViewPort* viewport) = 0;

	virtual void set_uniform4f(const char* uniform_name,const RBColorf& c)=0;
	virtual void set_uniform_texture(const char* uniform_name,int loc,const WIPBaseTexture* texture)=0;
	virtual void set_shader(const WIPBoundShader* shader)=0;
	virtual void set_vertex_format(const WIPVertexFormat* vf) const=0;
	virtual void set_vertex_buffer(const WIPVertexBuffer* vb) const=0;
	virtual void set_index_buffer(const WIPIndexBuffer* ib) const=0;
	virtual void draw_triangles(int count,int offset)const=0;
	virtual void enable_depth_test()const = 0;
	virtual void disable_depth_test()const = 0;
	virtual void set_depth_write(bool val) const = 0;
	virtual void enable_blend() const=0;
	virtual void disable_blend() const = 0;
	virtual void set_blend_function() const = 0;

	virtual void begin_debug_context() = 0;
	virtual void change_debug_color(const RBColorf& color) = 0;
	virtual void debug_draw_aabb2d(const RBVector2& minp, const RBVector2& maxp, const class WIPCamera* cam) = 0;
	virtual void debug_draw_box(const RBVector2* v,const class WIPCamera* cam) = 0;
	virtual void debug_submit()=0;
	virtual void end_debug_context() = 0;


};


extern WIPDynamicRHI* g_rhi;

class WIPCamera;
class WIPScene;
class WIPSprite;

class WIPRender
{
public:
	virtual void init(const WIPCamera* cam = nullptr) = 0;
	virtual void render(const WIPCamera* cam)=0;
	virtual void destroy()=0;
	const WIPCamera* camera;
};

class UIRender :public WIPRender
{
public:
	virtual void init(const WIPCamera* cam)
	{
		camera = cam;

		vb = g_rhi->RHICreateVertexBuffer(1024, 0, BufferType::E_DYNAMIC_DRAW);
		unsigned int data[] = { 0, 1, 3, 1, 2, 3 };
		ib = g_rhi->RHICreateIndexBuffer(6 * sizeof(unsigned int), data, BufferType::E_STATIC_DRAW);
		vf = g_rhi->RHICreateVertexFormat();
		vf->add_float_vertex_attribute(2);
		vf->add_float_vertex_attribute(2);

		bound_shader = g_rhi->RHICreateBoundShader(
			g_rhi->get_vertex_shader("text_vs"), g_rhi->get_pixel_shader("ui")
			);
		bound_shader_pic = g_rhi->RHICreateBoundShader(
			g_rhi->get_vertex_shader("text_vs"), g_rhi->get_pixel_shader("ui_pic")
			);

	}

	virtual void render(const WIPCamera* cam)
	{

	}

	void render_box(int px, int py, int w, int h, const RBColorf& c)
	{
		f32 vert[] = {
			-0.88f, -0.88f, 0.f, 1.f,
			-0.88f, -0.3f, 0.f, 0.f,
			0.88f, -0.3f, 1.f, 0.f,
			0.88f, -0.88f, 1.f, 1.f
		};

		void* p = g_rhi->lock_vertex_buffer(vb);
		memcpy(p, vert, sizeof(f32) * 16);
		g_rhi->unlock_vertex_buffer(vb);
		g_rhi->disable_depth_test();
		g_rhi->enable_blend();
		g_rhi->set_blend_function();
		g_rhi->set_shader(bound_shader);
		g_rhi->set_index_buffer(ib);
		g_rhi->set_vertex_buffer(vb);
		g_rhi->set_vertex_format(vf);
		g_rhi->set_uniform4f("in_color", c);
		g_rhi->draw_triangles(6 , 0);

		g_rhi->enable_depth_test();
	}

	void render_pic(int px, int py, int w, int h, const WIPTexture2D* tex);

	virtual void destroy()
	{

	}
	
	WIPVertexBuffer* vb;
	WIPIndexBuffer* ib;
	WIPVertexFormat* vf;
	WIPBoundShader* bound_shader;
	WIPBoundShader* bound_shader_pic;

};

class WorldRender : public WIPRender
{
public:
	virtual void init(const WIPCamera* cam = nullptr);
	void set_world(const WIPScene* scene);
	virtual void render(const WIPCamera* cam);
	virtual void destroy();
	void culling(const WIPCamera* cam);
	int _pack_sprites_blend(void *mem, int n, int offset_n, const WIPCamera& cam, bool& change_texture);
	int _pack_sprites_opaque(void *mem, int n, int offset_n, const WIPCamera& cam, bool& change_texture);
	/*
	03
	12
	*/
	void _pack_index(void *mem, int n)
	{
		unsigned int* p = (unsigned int*)mem;
		unsigned int s[6];
		int k = -1;
		int off = 0;
		while (n--)
		{
			++k;
			s[0] = 0 + off;
			s[1] = 1 + off;
			s[2] = 3 + off;
			s[3] = 1 + off;
			s[4] = 2 + off;
			s[5] = 3 + off;
			off += 4;
			memcpy(p, s, 6 * sizeof(unsigned int));
			p += 6;
		}
	}
	static bool comp_less(const WIPSprite* lhs, const WIPSprite* rhs);
	static bool comp_greater(const WIPSprite* lhs, const WIPSprite* rhs);
	void sort_by_texture();
	void sort_by_zorder();
	WIPIndexBuffer* index_buffer;
	WIPVertexBuffer* vertex_buffer;
	WIPVertexFormat* vertex_format;

	const WIPScene* scene_ref;

	int vertex_buffer_size;
	//maybe use for multithread!
	unsigned char* cpu_vertex_buffer;

	vector<const WIPSprite*> opaque_objects;
	vector<const WIPSprite*> blend_objects;

	float* pack_mem;

	WIPTexture2D* _pre_texture;

	class WIPBoundShader* bound_shader_opaque;
	class WIPBoundShader* bound_shader_translucent;
	class WIPBoundShader* bound_shader_mask;
};

class DebugRender :public WIPRender
{
public:
	virtual void init(const WIPCamera* cam = nullptr);
	virtual void render(const WIPCamera* cam);
	virtual void destroy();
};

#include "ft2build.h"
#include "freetype.h"
#include "ftglyph.h"
#include "ftoutln.h"
#include "fttrigon.h"

class TextRender : public WIPRender
{
public:
	class TextNode
	{
	public:

		TextNode(int tw,int th,int ta,int ofx,int ofy,int tx,int ty) :pre_node(nullptr), nex_node(nullptr),
		text_width(tw),text_height(th),text_advance(ta),offx(ofx),offy(ofy),texture_x(tx),texture_y(ty){}
		TextNode* pre_node;
		TextNode* nex_node;
		int text_height;
		int text_width;
		int text_advance;
		int offx;
		int offy;

		int texture_x;
		int texture_y;


		wchar_t id;
	};

	void _pack_index(void *mem, int n)
	{
		unsigned int* p = (unsigned int*)mem;
		unsigned int s[6];
		int k = -1;
		int off = 0;
		while (n--)
		{
			++k;
			s[0] = 0 + off;
			s[1] = 1 + off;
			s[2] = 3 + off;
			s[3] = 1 + off;
			s[4] = 2 + off;
			s[5] = 3 + off;
			off += 4;
			memcpy(p, s, 6 * sizeof(unsigned int));
			p += 6;
		}
	}

	TextNode* get_node(wchar_t wch)
	{
		
		map<wchar_t,TextNode*>::iterator s = search_container.find(wch);
		if (s == search_container.end())
		{
			
			FT_Glyph glyph;
			FT_Load_Glyph(ftface, FT_Get_Char_Index(ftface, wch), FT_LOAD_DEFAULT);
			FT_Get_Glyph(ftface->glyph, &glyph);
			FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);


			FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
			FT_Bitmap& bitmap = bitmap_glyph->bitmap;
			int width = bitmap.width;
			int height = bitmap.rows;
			int advancex = ftface->glyph->advance.x / 64.f;
			int offx = bitmap_glyph->left;
			int offy = bitmap_glyph->top - height;
			
			bool ret = insert_node(width, height, bitmap.buffer,advancex,offx,offy,wch);
			if (ret)
			{
				FT_Done_Glyph(glyph);
				return lru_header->nex_node;
			}
			else
			{
				//overflowed!
				if (lru_header->pre_node != lru_header)
				{
					TextNode* node = lru_header->pre_node;
					if (!lru_header->nex_node)
					{
						LOG_NOTE("lll");
					}
					map<wchar_t, TextNode*>::iterator tail = search_container.find(node->id);
					if (tail != search_container.end())
					{
						update_cache(node->texture_x, node->texture_y, 
							width, height, bitmap.buffer);

						search_container.erase(tail);
						remove_node_link_tail();

						node->id = wch;
						node->offx = offx;
						node->offy = offy;
						node->text_advance = advancex;
						node->text_height = height;
						node->text_width = width;

						//insert a new node
						if (search_container.find(node->id) == search_container.end())
						{
							search_container[node->id] = node;
						}
						insert_node_to_link_front(node);

						FT_Done_Glyph(glyph);

						return lru_header->nex_node;
					}
					else
					{
						LOG_ERROR("No such a node,Wrong!");
					}
				}
			}

		}
		else
		{
			int le = get_link_len();
			remove_node_link(s->second);
			insert_node_to_link_front(s->second);
			int l2 = get_link_len();
			if (le !=l2)
			{
				LOG_NOTE("wtf!");
			}
			return lru_header->nex_node;
		}
		return nullptr;
	}

	int get_link_len()
	{
		TextNode* p = lru_header;
		int ret = -1;
		while (p)
		{
			ret++;
			p = p->nex_node;
		}
		return ret;
	}

	bool insert_node(int w, int h, unsigned char* data, int advancex,int offx,int offy,wchar_t wch)
	{
		if (used >= total)
		{
			texture_change = true;
			return false;
		}
		int x = (used % nx) * text_w;
		int y = (used / nx) * text_h;
		TextNode* node = new TextNode(w, h, advancex, offx, offy, 0, 0);
		node->id = wch;
		node->texture_x = x;
		node->texture_y = y;
		if (search_container.find(node->id) == search_container.end())
		{
			search_container[node->id] = node;
		}
		else
		{
			LOG_ERROR("Find an unfind node!");
		}
		insert_node_to_link_front(node);
		update_cache(x, y, w, h, data);
		used++;
		return true;
	}

	TextNode* remove_node_link_tail()
	{
		if (lru_header->nex_node == lru_header)
			return nullptr;
		auto* tail = lru_header->pre_node;
		tail->pre_node->nex_node = nullptr;
		lru_header->pre_node = tail->pre_node;
		return tail;
	}

	void remove_node_link(TextNode* node)
	{
		auto* next = node->nex_node;
		auto* prev = node->pre_node;
		node->nex_node = nullptr;
		node->pre_node = nullptr;
		if (next == nullptr)
		{
			lru_header->pre_node = prev;
			prev->nex_node = nullptr;
			return;
		}
		next->pre_node = prev;
		prev->nex_node = next;
	}

	void insert_node_to_link_front(TextNode* node)
	{
		if (lru_header->nex_node == node)
		{
			return;
		}
		if (lru_header->nex_node)
		{
			lru_header->nex_node->pre_node = node;
			node->nex_node = lru_header->nex_node;
			node->pre_node = lru_header;
			lru_header->nex_node = node;
		}
		else
		{
			lru_header->nex_node = node;
			node->pre_node = lru_header;
			node->nex_node = nullptr;
			lru_header->pre_node = node;
		}
	}

	TextRender(int in_cache_w = 1024, int in_cache_h = 1024) :
		cache_w(in_cache_w), cache_h(in_cache_h){}
	virtual void init(const WIPCamera* cam = nullptr)
	{
		nx = 0;
		ny = 0;
		total = 0;
		used = 0;
		lru_header = new TextNode(-1, -1, -1, -1, -1,-1,-1);
		lru_header->nex_node = nullptr;
		lru_header->pre_node = lru_header;
		lru_header->id = 0;

		texture_change = false;
		text_to_render = 0;

		//flag = 2 set a GL_RED texture.
		text_lut_buffer = g_rhi->RHICreateTexture2D(cache_w, cache_h , 0, 0, 0, 0, 2);
		text_cache = new unsigned char[cache_w*cache_h];
		memset(text_cache, 0, cache_h*cache_w);
		vb = g_rhi->RHICreateVertexBuffer(cache_w * cache_h, 0, BufferType::E_DYNAMIC_DRAW);
		text_vertex_buffer = new unsigned char[cache_w * cache_h * sizeof(f32)];
		memset(text_vertex_buffer, 0, cache_w * cache_h*sizeof(f32));

		unsigned int data[] = {0,1,3,1,2,3};
		int n = (cache_w * cache_h) / (sizeof(f32) * 4);
		ib = g_rhi->RHICreateIndexBuffer(6*sizeof(unsigned int)*n, 0, BufferType::E_DYNAMIC_DRAW);
		vf = g_rhi->RHICreateVertexFormat();
		vf->add_float_vertex_attribute(2);
		vf->add_float_vertex_attribute(2);

		bound_shader = g_rhi->RHICreateBoundShader(
			g_rhi->get_vertex_shader("text_vs"),g_rhi->get_pixel_shader("text")
			);

		if (FT_Init_FreeType(&ftlibrary))
		{
			LOG_ERROR("Init freetype failed!");
			return;
		}
		//大小要乘64
		//FT_Set_Char_Size( m_FT_Face , 0 , m_w << 6, 96, 96);
	}

	void load_font(const char* name,int w,int h)
	{
		nx = cache_w / w;
		ny = cache_h / h;
		total = nx*ny;
		used = 0;
		auto handler = g_res_manager->load_resource(name, EFont);
		if(FT_New_Memory_Face(ftlibrary, (FT_Byte*) handler->ptr, handler->size, 0, &ftface))
		//if (FT_New_Face(ftlibrary, name, 0, &ftface))
		{
			g_res_manager->free(handler,handler->size);
			LOG_ERROR("%s load failed!",name);
			return;
		}
		//g_res_manager->free(handler,handler->size);

		FT_Error e;
		e = FT_Select_Charmap(ftface, FT_ENCODING_UNICODE);
		if (e)
		{
			printf("load ttf failed!");
			return;
		}
		//e = FT_Set_Pixel_Sizes(ftface, w, h);
		//if (e)
		//{
			//return;
		//}
		text_w = w;
		text_h = h;

		f32 vert[] = {
			-0.5f,-0.5f,0.f,1.f,
			-0.5f,0.5f,0.f,0.f,
			0.5f,0.5f,1.f,0.f,
			0.5f,-0.5f,1.f,1.f
		};

		void* p = g_rhi->lock_vertex_buffer(vb);
		memcpy(p, vert, sizeof(f32)*16);
		g_rhi->unlock_vertex_buffer(vb);

		FT_Set_Char_Size(ftface, (text_w-1) << 6, (text_h-1) << 6, 72, 72);
	
	}

	void render_text(int px, int py, const wchar_t* chs, int len,int maxw, const WIPCamera* cam);

	virtual void render(const WIPCamera* cam);
	void load_text(const wchar_t* chs,int len=3)
	{
		int l = 0;
		for (int i = 0; i < len; ++i)
		{
			FT_Glyph glyph;
			FT_Load_Glyph(ftface, FT_Get_Char_Index(ftface, chs[i]), FT_LOAD_RENDER);

			if (FT_Get_Glyph(ftface->glyph, &glyph))
			{
				LOG_ERROR("get glyph failed!");
				return;
			}
			//FT_Render_Glyph(ftface->glyph, FT_RENDER_MODE_LCD);//FT_RENDER_MODE_NORMAL  ); 
			FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
			FT_BitmapGlyph bitmap_glyph = (FT_BitmapGlyph)glyph;
			FT_Bitmap& bitmap = bitmap_glyph->bitmap;
			int width = bitmap.width;
			int height = bitmap.rows;
			/*
			this is an alignment issue in gl.by default,the texture row is aligned by 4 bytes.
			so your subimage must has with of times of 4 bytes,or you get a wrong copy.
			and you can use glPixelStorei(GL_UNPACK_ALIGNMENT, 1) to set alignment to 1, but it make run time slow.
			so you can also, like blow, make a new mem row aligned by 4 manually.
			but this is just a example to solve this problem,
			we use a big CPU mem to cache the text data and upload it when rendering.
			*/
#if 0
			unsigned char* s = new unsigned char[((width + 3) / 4 * 4)*height];
			for (int j = 0; j < height;++j)
			{
				for (int i = 0; i < (width + 3) / 4 * 4; ++i)
				{

					s[((width + 3) / 4 * 4)*j+i] = bitmap.buffer[width*j + i];
				}
			}
			g_rhi->update_subrect_texture(text_lut_buffer, 1, 1, (width+3)/4*4,height, s);

			delete[] s;
#endif

			//draw at (x,y)
			//text left bottom origin lb
			//dlb.y = y - (height - bearingY)
			//dlb.x = x + bearX
			//next x = x + advance
			//given a line max_height(>height)
			//next y = y + max_height
			//given a max_width
			//if meet '\n' y next
			update_cache(l, 0, width, height, bitmap.buffer);
			l += width;

			g_rhi->update_texture(text_lut_buffer, text_cache);
		}
	}



	void update_cache(int x,int y,int w,int h,unsigned char* data)
	{
		unsigned char* p = text_cache;
		p += x+y*cache_w;
		unsigned char* pp = data;

		for (int i = 0; i < h; i++)
		{
			memcpy(p, pp, w);
			p += cache_w;
			pp += w;
		}

		
	}

	virtual void destroy()
	{
		for (int i = 0; i < search_container.size(); ++i)
		{
			delete search_container[i];
		}
		FT_Done_Face(ftface);
		FT_Done_FreeType(ftlibrary);
		delete[] text_cache;
	}
	WIPTexture2D* text_lut_buffer;
	unsigned char* text_vertex_buffer;
	unsigned char* text_cache;
	int cache_w, cache_h;
	int nx, ny;
	int total, used;
	int text_w, text_h;

	//ad-hoc use map to chage to hash_map
	map<wchar_t, TextNode*> search_container;
	TextNode* lru_header;

	FT_Library ftlibrary;
	FT_Face    ftface;
	WIPVertexBuffer* vb;
	WIPIndexBuffer* ib;
	WIPVertexFormat* vf;
	WIPBoundShader* bound_shader;

	bool texture_change;
	int text_to_render;
};