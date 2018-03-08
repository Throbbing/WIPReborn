{
WIPSpriteCreator ctor_trs1(10.000000, 1.000000, WIPMaterialType::E_OTHER);
ctor_trs1.texture = 0;
ctor_trs1.world_render = 0;
ctor_trs1.body_tp = WIPCollider::_CollisionTypes::E_GHOST;
ctor_trs1.collider_sx = 1.0f;
ctor_trs1.collider_sy = 1.0f;
auto* man_trans1 = WIPSpriteFactory::create_sprite(ctor_trs1);
man_trans1->set_anchor(0.f, 0.f);
man_trans1->set_tag("fk");
man_trans1->set_type_tag("trigger");
TransformComponent* tc1 = (TransformComponent*)WIPObject::create_tick_component("TransformComponent", man_trans1);
tc1->func_begin = [](void* data, float dt, TransformComponent* t)->void
{
//add your code 
};
tc1->func_end = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
//you can also edit your code in level file. 
};
tc1->func_update = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_contact = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_start = [](void* data, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_end = [](void* data, TransformComponent* t)->void
{
//add your code 
};
man_trans1->add_tick_component(tc1);
scene->load_sprite(man_trans1);
man_trans1->translate_to(3.250000, 1.500000);
man_trans1->rotate_to(20.000000);}
{
WIPSpriteCreator ctor_trs1(10.000000, 1.000000, WIPMaterialType::E_OTHER);
ctor_trs1.texture = 0;
ctor_trs1.world_render = 0;
ctor_trs1.body_tp = WIPCollider::_CollisionTypes::E_GHOST;
ctor_trs1.collider_sx = 1.0f;
ctor_trs1.collider_sy = 1.0f;
auto* man_trans1 = WIPSpriteFactory::create_sprite(ctor_trs1);
man_trans1->set_anchor(0.f, 0.f);
man_trans1->set_tag("fk");
man_trans1->set_type_tag("trigger");
TransformComponent* tc1 = (TransformComponent*)WIPObject::create_tick_component("TransformComponent", man_trans1);
tc1->func_begin = [](void* data, float dt, TransformComponent* t)->void
{
//add your code 
};
tc1->func_end = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
//you can also edit your code in level file. 
};
tc1->func_update = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_contact = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_start = [](void* data, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_end = [](void* data, TransformComponent* t)->void
{
//add your code 
};
man_trans1->add_tick_component(tc1);
scene->load_sprite(man_trans1);
man_trans1->translate_to(3.250000, 1.500000);
man_trans1->rotate_to(0.000000);}
{
WIPSpriteCreator ctor_trs1(10.000000, 1.000000, WIPMaterialType::E_OTHER);
ctor_trs1.texture = 0;
ctor_trs1.world_render = 0;
ctor_trs1.body_tp = WIPCollider::_CollisionTypes::E_GHOST;
ctor_trs1.collider_sx = 1.0f;
ctor_trs1.collider_sy = 1.0f;
auto* man_trans1 = WIPSpriteFactory::create_sprite(ctor_trs1);
man_trans1->set_anchor(0.f, 0.f);
man_trans1->set_tag("fk");
man_trans1->set_type_tag("trigger");
TransformComponent* tc1 = (TransformComponent*)WIPObject::create_tick_component("TransformComponent", man_trans1);
tc1->func_begin = [](void* data, float dt, TransformComponent* t)->void
{
//add your code 
};
tc1->func_end = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
//you can also edit your code in level file. 
};
tc1->func_update = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_contact = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_start = [](void* data, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_end = [](void* data, TransformComponent* t)->void
{
//add your code 
};
man_trans1->add_tick_component(tc1);
scene->load_sprite(man_trans1);
man_trans1->translate_to(3.250000, 1.500000);
man_trans1->rotate_to(90.000000);}
{
WIPSpriteCreator ctor_trs1(10.000000, 1.000000, WIPMaterialType::E_OTHER);
ctor_trs1.texture = 0;
ctor_trs1.world_render = 0;
ctor_trs1.body_tp = WIPCollider::_CollisionTypes::E_GHOST;
ctor_trs1.collider_sx = 1.0f;
ctor_trs1.collider_sy = 1.0f;
auto* man_trans1 = WIPSpriteFactory::create_sprite(ctor_trs1);
man_trans1->set_anchor(0.f, 0.f);
man_trans1->set_tag("fk");
man_trans1->set_type_tag("trigger");
TransformComponent* tc1 = (TransformComponent*)WIPObject::create_tick_component("TransformComponent", man_trans1);
tc1->func_begin = [](void* data, float dt, TransformComponent* t)->void
{
//add your code 
};
tc1->func_end = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
//you can also edit your code in level file. 
};
tc1->func_update = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_contact = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_start = [](void* data, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_end = [](void* data, TransformComponent* t)->void
{
//add your code 
};
man_trans1->add_tick_component(tc1);
scene->load_sprite(man_trans1);
man_trans1->translate_to(3.250000, 1.500000);
man_trans1->rotate_to(60.000000);}
{
WIPSpriteCreator ctor_trs1(10.000000, 1.000000, WIPMaterialType::E_OTHER);
ctor_trs1.texture = 0;
ctor_trs1.world_render = 0;
ctor_trs1.body_tp = WIPCollider::_CollisionTypes::E_GHOST;
ctor_trs1.collider_sx = 1.0f;
ctor_trs1.collider_sy = 1.0f;
auto* man_trans1 = WIPSpriteFactory::create_sprite(ctor_trs1);
man_trans1->set_anchor(0.f, 0.f);
man_trans1->set_tag("fk");
man_trans1->set_type_tag("trigger");
TransformComponent* tc1 = (TransformComponent*)WIPObject::create_tick_component("TransformComponent", man_trans1);
tc1->func_begin = [](void* data, float dt, TransformComponent* t)->void
{
//add your code 
};
tc1->func_end = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
//you can also edit your code in level file. 
};
tc1->func_update = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_contact = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_start = [](void* data, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_end = [](void* data, TransformComponent* t)->void
{
//add your code 
};
man_trans1->add_tick_component(tc1);
scene->load_sprite(man_trans1);
man_trans1->translate_to(3.250000, 1.500000);
man_trans1->rotate_to(1.860000);}
{
WIPSpriteCreator ctor_trs1(10.000000, 1.000000, WIPMaterialType::E_OTHER);
ctor_trs1.texture = 0;
ctor_trs1.world_render = 0;
ctor_trs1.body_tp = WIPCollider::_CollisionTypes::E_GHOST;
ctor_trs1.collider_sx = 1.0f;
ctor_trs1.collider_sy = 1.0f;
auto* man_trans1 = WIPSpriteFactory::create_sprite(ctor_trs1);
man_trans1->set_anchor(0.f, 0.f);
man_trans1->set_tag("fk");
man_trans1->set_type_tag("trigger");
TransformComponent* tc1 = (TransformComponent*)WIPObject::create_tick_component("TransformComponent", man_trans1);
tc1->func_begin = [](void* data, float dt, TransformComponent* t)->void
{
//add your code 
};
tc1->func_end = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
//you can also edit your code in level file. 
};
tc1->func_update = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_contact = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_start = [](void* data, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_end = [](void* data, TransformComponent* t)->void
{
//add your code 
};
man_trans1->add_tick_component(tc1);
scene->load_sprite(man_trans1);
man_trans1->translate_to(3.250000, 1.500000);
man_trans1->rotate_to(3.141590);}
{
WIPSpriteCreator ctor_trs1(1.000000, 1.000000, WIPMaterialType::E_OTHER);
ctor_trs1.texture = 0;
ctor_trs1.world_render = 0;
ctor_trs1.body_tp = WIPCollider::_CollisionTypes::E_GHOST;
ctor_trs1.collider_sx = 1.0f;
ctor_trs1.collider_sy = 1.0f;
auto* man_trans1 = WIPSpriteFactory::create_sprite(ctor_trs1);
man_trans1->set_anchor(0.f, 0.f);
man_trans1->set_tag("eat_peopel");
man_trans1->set_type_tag("trigger");
TransformComponent* tc1 = (TransformComponent*)WIPObject::create_tick_component("TransformComponent", man_trans1);
tc1->func_begin = [](void* data, float dt, TransformComponent* t)->void
{
//add your code 
};
tc1->func_end = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
//you can also edit your code in level file. 
};
tc1->func_update = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_contact = [](void* data, const WIPSprite* s, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_start = [](void* data, TransformComponent* t)->void
{
//add your code 
};
tc1->func_level_end = [](void* data, TransformComponent* t)->void
{
//add your code 
};
man_trans1->add_tick_component(tc1);
scene->load_sprite(man_trans1);
man_trans1->translate_to(-4.500000, -1.860000);
man_trans1->_render->is_visible = false
;man_trans1->rotate_to(0.000000);}
