#pragma once
#include "imgui.h"
#include "Object.h"
#include "RBMath/Inc/Vector2.h"
#include <map>

//sender自己处理何时发送什么事件，一旦发送事件马上就要根据contex
//中一个sender和event type找到接受事件object组，对每一个Object调用on_event
//对比sender和event_type，call event_handler.
//订阅的时候，创建event_handler，是便于自己调用自己的成员函数

class IMButton : public WIPObject
{
public:
  struct Param
  {
    Param() :name(0), pass(true)
    {}
    string_hash name;
    RBVector2I position;
    bool pass;
  };
  WIPOBJECT(IMButton, WIPObject);
  IMButton(const char* name);
  void update(string_hash tp, void* data);
  string_hash name_;
};

class IMPopup : public WIPObject
{
public:
  WIPOBJECT(IMPopup, WIPObject);
};

class IMCheckBox :public WIPObject
{
public:
  WIPOBJECT(IMCheckBox, WIPObject);
  IMCheckBox(std::string name,bool check = false):
    _name(name), v(check)
  {
  }
  void signal_for_init()
  {
    send_event(get_string_hash("check_box_change"), (void*)&v);

  }
  void update(string_hash tp, void* data)
  {
    if (ImGui::Checkbox(_name.data(), &v))
    {
      send_event(get_string_hash("check_box_change"), (void*)&v);
    }

  }
  std::string _name;
  bool v;
};

class IMMainMenuBar : public WIPObject
{
public:
  typedef std::vector<std::string> menu_item_type;
  struct _Menu
  {
    _Menu(const std::string& n)
    {
      name = n;
    }
    std::string name;
    menu_item_type items;
  };
  typedef std::vector<_Menu*> menu_type;
  WIPOBJECT(IMMainMenuBar, WIPObject);
  IMMainMenuBar()
  {

  }
  void add_menu_item(const std::string& mname,const std::string& name)
  {
    _Menu* ex = find_menu(mname);
    if (!ex)
    {
      _Menu* m = new _Menu(mname);
      m->items.push_back(name);
      _menus.push_back(m);
    }
    else
    {
      bool fd = find_menu_item(name, ex->items);
      if (!fd)
        ex->items.push_back(name);
    }
  }
  void update(string_hash tp, void* data)
  {
    if (ImGui::BeginMainMenuBar())
    {
      menu_type::iterator it1 = _menus.begin();
      for (; it1 != _menus.end(); ++it1)
      {
        menu_item_type::iterator it2 = (*it1)->items.begin();
        if (ImGui::BeginMenu((*it1)->name.data()))
        {
          for (; it2 != (*it1)->items.end(); ++it2)
          {
            std::string s = (*it1)->name + "." +(*it2);
            if (ImGui::MenuItem((*it2).data()))
            {
              printf("send event %s|%u\n", s.data(),get_string_hash(s.data()));
              send_event(get_string_hash(s.data()), 0);
            }
          }
          ImGui::EndMenu();
        }
      }
      ImGui::EndMainMenuBar();
    }
  }

  

  _Menu* find_menu(const std::string& name)
  {
    for (int i = 0; i < _menus.size(); ++i)
    {
      if (_menus[i]->name == name)
        return _menus[i];
    }
    return nullptr;
  }
  
  bool find_menu_item(const std::string& name,menu_item_type& mi)
  {
    for (int i = 0; i < mi.size(); ++i)
    {
      if (mi[i] == name)
        return true;
    }
    return false;
  }

  menu_type _menus;
};

class A : public WIPObject
{
public:
  WIPOBJECT(A, WIPObject);
  A();
  void push(string_hash tp, void* data);
  void no_sender_push(string_hash tp,void* data);
  void check_box(string_hash tp, void* data)
  {
    bool c = *((bool*)data);
    printf("check box : %s\n",c?"true":"false");
  }
};

class B : public WIPObject
{
public:
  WIPOBJECT(B, WIPObject);
  B(){}
  void push1(string_hash tp, void* data)
  {
    IMButton::Param* p = static_cast<IMButton::Param*>(data);
    if (!p->pass)
      return;
    p->pass = false;
    printf("B receive->Button hash:%u|position:(%d,%d)\n", p->name, p->position.x, p->position.y);
  }
  void handle_menu(string_hash tp, void* data)
  {
    printf("get %u\n",tp);
  }
};

void test_imui_wrap();

