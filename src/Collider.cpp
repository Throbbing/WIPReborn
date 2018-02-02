#include "Sprite.h"
#include "PhysicsManager.h"

WIPCollider*  WIPCollider::create_collider(std::vector<RBVector2>& poly)
{
	return nullptr;
}

WIPCollider* WIPCollider::create_collider(TRefCountPtr<WIPSprite> m, WIPCollider::_CollisionTypes tp,f32 sx,f32 sy)
{

	WIPCollider* ret;
	ret = new WIPCollider();
	ret->set_host(m);
	ret->_polygon_shape = new b2PolygonShape();
	ret->set_mesh_box(sx, sy);
	if (m)
	{
		ret->reset_polygon_vertices(m, 4);
	}
	else
	{
		ret->_polygon_shape->SetAsBox(1, 1);
	}
	ret->_body = g_physics_manager->create_body((b2BodyType)tp);
	ret->_body->SetFixedRotation(true);
	// Define the dynamic body fixture.
	b2FixtureDef fixtureDef;
	fixtureDef.shape = ret->_polygon_shape;
	//fixtureDef.isSensor = true;
	// Set the box density to be non-zero, so it will be dynamic.
	fixtureDef.density = 1.0f;

	// Override the default friction.
	fixtureDef.friction = 0.3f;

	// Add the shape to the body.
	ret->_body->CreateFixture(&fixtureDef);
	return ret;
}

void WIPCollider::set_active(bool v)
{
	bool _active = v;
	if (!_active)
	{
		//_body->setType(b2_staticBody);
	}
	else
	{
		//_body->(b2_dynamicBody);
	}

}

void WIPCollider::reset_body_type(_CollisionTypes type)
{
	
	switch (type)
	{
	case E_RIGIDBODY:
		_body->SetType(b2_dynamicBody);
		_active = true;
		break;
	case E_STATIC_RIGIDBODY:
		_body->SetType(b2_staticBody);
		_active = true;
		break;
	case E_NO_PHYSICS:
		_active = false;
		break;
	case E_GHOST:
	default:
		_body->SetType(b2_kinematicBody);
		_active = true;
		break;
	}
	

}

WIPCollider::WIPCollider() :_cb_scale_x(1.0f), _cb_scale_y(1.0f), _active(true)
{
}

WIPCollider::~WIPCollider()
{
	//g_physics_manager->delete_body(_body);
}

void WIPCollider::destroy()
{
	g_physics_manager->delete_body(_body);
}

void WIPCollider::reset_polygon_vertices(TRefCountPtr<WIPSprite> s, i32 n)
{
	if (!s->_render)
		return;
	RBVector2 vert[4];
	b2Vec2 v1[4];
	RBVector2 scale2(s->_transform->scale_x, s->_transform->scale_y);
	RBVector2 scale3(_cb_scale_x, _cb_scale_y);

	s->get_anchor_vertices(vert);
	vert[0] *= scale2*scale3;
	vert[1] *= scale2*scale3;
	vert[2] *= scale2*scale3;
	vert[3] *= scale2*scale3;
	v1[0].x = vert[3].x;
	v1[0].y = vert[3].y;

	v1[1].x = vert[0].x;
	v1[1].y = vert[0].y;

	v1[2].x = vert[2].x;
	v1[2].y = vert[2].y;

	v1[3].x = vert[1].x;
	v1[3].y = vert[1].y;
	_polygon_shape->Set(v1, 4);

}

void WIPCollider::reset_polygon_position(f32 x, f32 y)
{
	_body->SetTransform(b2Vec2(x, y), _body->GetAngle());
}

void WIPCollider::reset_polygon_density(f32 density)
{
}

void WIPCollider::recreate_fixture()
{
	b2Fixture* fixture = _body->GetFixtureList();
	if (fixture)
	{
		_body->DestroyFixture(fixture);
		b2FixtureDef fixturedef;
		fixturedef.userData = this->host_object;
		fixturedef.shape = _polygon_shape;
		_body->CreateFixture(&fixturedef);
	}
}

void WIPCollider::reset_body_rad(f32 rad)
{
	_body->SetTransform(_body->GetPosition(), rad);
}

void WIPCollider::reset_body_restitution(f32 v)
{

}

void WIPCollider::reset_body_friction(f32 v, _FrictionTypes type)
{
	/*
	switch (type)
	{
	case _FrictionTypes::E_DYNAMIC:
		_body->dynamicFriction = v;
		break;
	case _FrictionTypes::E_STATIC:
		_body->staticFriction = v;
		break;
	default:
		break;
	}
	*/

}

void WIPCollider::update_out(WIPMesh& mesh, f32& rad, f32& x, f32 &y)
{



}

void WIPCollider::update_in(WIPMesh* mesh, f32 rad, f32 x, f32 y)
{


}

void WIPCollider::add_force(f32 x, f32 y)
{
	_body->ApplyForceToCenter(b2Vec2(x,y),true);
}

i32 WIPCollider::get_collision_list_size()
{
	int s = 0;
	for (b2ContactEdge* ce = _body->GetContactList(); ce; ce = ce->next)
	{
		b2Contact* c = ce->contact;
		s++;
	}
	return s;
}

b2Body* WIPCollider::get_collision_index_body(i32 i)
{
	return nullptr;
}

bool WIPCollider::is_collision_list_empty()
{
	//use listener
	return false;
}

void WIPCollider::set_density(f32 density)
{

	//_body->setShape(_polygon_shape);
}

f32 WIPCollider::get_speed_x()
{
	return 0;
}

f32 WIPCollider::get_speed_y()
{
	return 0;

}

void WIPCollider::set_sprite(TRefCountPtr<WIPSprite> sprite)
{
	if (_body)
		_body->SetUserData((WIPSprite *)sprite);
}

void WIPCollider::on_begin_contact(const WIPSprite* s)
{
	CHECK(s != this->host_object);
	_contact_objects.insert(s);
	WIPSprite* o = this->host_object;
	for (auto i : o->tick_components)
	{
		i->on_begin_contact(s);
	}
}

void WIPCollider::on_end_contact(const WIPSprite* s)
{
	CHECK(s != this->host_object);
	_contact_objects.remove(s);
	WIPSprite* o = this->host_object;
	for (auto i : o->tick_components)
	{
		i->on_end_contact(s);
	}
}