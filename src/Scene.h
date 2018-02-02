#pragma once
#include "QuadTree.h"
#include <string>
#include "RefCount.h"

class WIPSprite;
class WIPCamera;
class WIPScene;

//1.we dont surpport mulitple world
//2.world is a container of so many objects
//3.scene is just a stage of worlds
class WIPWorld
{
public:
	WIPWorld();
	~WIPWorld();

private:
	std::vector<WIPSprite*> _objects;
	std::vector<WIPCamera*> _cameras;
};



//场景全局只有一个，它只负责做一件事：加载一个level，创建所有object，run所有object
//场景持有摄像机,摄像机分为两个空闲摄像机链表和激活摄像机链表，相机激活通过active来决定
//场景中持有游戏中的所有object在一个list，更新和查询机制同相，active决定是否更新；特别的，
//场景还有一个持久list，用于存放那些生命周期跨越关卡的objects，这些objects也参与更新和查询，
//但是不会随着改变关卡而被销毁，所以object引用其他object是不推荐的，
//但是每次关卡load完成的时候，都会调用object的start函数，此时可以做一些初始化工作，
//同样关卡销毁时也会调用end方法
//一个关卡就是一坨数据，本类就负责加载这些数据，然后执行这些数据所创建对象的组件
//对于用户自定义的脚本组件在C++都被归类为一个ScriptComponent
//为了保证灵活性，Scene应该有粒度细小的接口，load_level应该被一个Loader来处理。
class WIPScene1 :public FRefCountedObject
{
public:
	void create_sprite();
	void create_camera();


private:
	

	typedef HashLink<WIPSprite> object_list_t;
	std::list<WIPCamera*> _cameras;

	object_list_t _objects;
	u32 _object_num = 0;
	object_list_t _persistent_objects;
	u32 _persistent_object_num = 0;
};

class WIPScene : public FRefCountedObject
{
public:
	WIPScene():initilized(false)
	{
	}
	~WIPScene();

	void init(float world_sizex, float world_sizey, int max_depth);
	void add_sprite(TRefCountPtr<WIPSprite> sprite);
	//delete related_scene may detroy iterator.
	void remove_sprite(TRefCountPtr<WIPSprite> sptrite,bool op_related_scene=true);
	WIPCamera* create_camera(f32 w, f32 h, f32 sw, f32 sh, int iww, int iwh);
	void update_sprite(TRefCountPtr<WIPSprite> sprite);
	void sort_by_y(std::vector<TRefCountPtr<WIPSprite>> v);
	void update_zorder_by_type_tag(std::string type_tag,f32 min_z=0.2f,f32 max_z=0.8f);
	void update_zorder_by_type_tag(TRefCountPtr<const WIPSprite> s, std::string type_tag, f32 min_z = 0.2f, f32 max_z = 0.8f);
	void update(f32 dt);
	void fix_update(f32 dt);
	void init_components();
	void destroy_components();
	WIPSprite* get_sprite_by_tag(const std::string& name) const;
	void clear();
	//todo
	void change_level(){}
	bool initilized;
	float world_size_x,world_size_y;
	WIPQuadTree* quad_tree;
	std::vector<TRefCountPtr<WIPSprite>> objects;
	std::vector<WIPCamera*> cameras;
};