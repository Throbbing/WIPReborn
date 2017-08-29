#pragma once
#include "imgui.h"
#include "Object.h"
#include "RBMath/Inc/Vector2.h"

//sender自己处理何时发送什么事件，一旦发送事件马上就要根据contex
//中一个sender和event type找到接受事件object组，对每一个Object调用on_event
//对比sender和event_type，call event_handler.
//订阅的时候，创建event_handler，是便于自己调用自己的成员函数

class IMButton : public WIPObject
{
public:
  struct Param
  {
    string_hash name;
    RBVector2I position;
  };
  WIPOBJECT(IMButton, WIPObject);
  IMButton(const char* name);
  void update();
  string_hash name_;
};

class A : public WIPObject
{
public:
  WIPOBJECT(A, WIPObject);
  A();
  void push(string_hash tp, void* data);
  void no_sender_push(string_hash tp,void* data);
};

class B : public WIPObject
{
public:
  WIPOBJECT(B, WIPObject);
  B(){}
  void push1(string_hash tp, void* data)
  {
    printf("B recieved\n");
  }
};

void test_imui_wrap();

