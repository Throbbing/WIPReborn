#include "UIWrap.h"
#include "Object.h"

IMButton::IMButton(const char* name)
{
  name_ = get_string_hash(name);
}

void IMButton::update(string_hash tp, void* data)
{
  if (ImGui::Button("a"))
  {
    Param param;
    param.name = name_;
    param.position.x = ImGui::GetCursorPosX();
    param.position.y = ImGui::GetCursorPosY();
    printf("send->Button hash:%u\nposition:(%d,%d)\n", param.name, param.position.x, param.position.y);
    //send "button_push" event(in the function actually call reciever's handler immediately.)
    send_event(get_string_hash("button_push"), &param);
  }
}

A::A()
{
  subscribe_event(get_string_hash("button_push"), WIP_EVENT_HANDLER(A, no_sender_push));
}

void A::push(string_hash tp, void* data)
{
  IMButton::Param* p = static_cast<IMButton::Param*>(data);
  printf("receive->Button hash:%u\nposition:(%d,%d)\n",p->name,p->position.x,p->position.y);

}

void A::no_sender_push(string_hash tp, void* data)
{
  printf("no sender pushed!\n");
}

void test_imui_wrap()
{
  IMButton* imbt = new IMButton("rinis");
  A* a = new A();
  a->subscribe_event(imbt, get_string_hash("button_push"), WIP_EVENT_HANDLER_OUT(A, push, a));

}