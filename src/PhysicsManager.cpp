#include "PhysicsManager.h"
#include "Sprite.h"
#include "Render.h"


void WIPPhysicsManager::WIPBox2dSpriteContactListener::BeginContact(b2Contact* contact)
{
	void* bodyUserData1 = contact->GetFixtureA()->GetBody()->GetUserData();
	void* bodyUserData2 = contact->GetFixtureB()->GetBody()->GetUserData();
	CHECK(bodyUserData1&&bodyUserData2);
	{
		WIPSprite* s1 = static_cast<WIPSprite*>(bodyUserData1);
		WIPSprite* s2 = static_cast<WIPSprite*>(bodyUserData2);

		CHECK(s1->_collider);
		CHECK(s2->_collider);

		s1->_collider->on_begin_contact(s2);
		s2->_collider->on_begin_contact(s1);
	}
	
}
void WIPPhysicsManager::WIPBox2dSpriteContactListener::EndContact(b2Contact* contact)
{

	void* bodyUserData1 = contact->GetFixtureA()->GetBody()->GetUserData();
	void* bodyUserData2 = contact->GetFixtureB()->GetBody()->GetUserData();
	CHECK(bodyUserData1&&bodyUserData2);
	{
		WIPSprite* s1 = static_cast<WIPSprite*>(bodyUserData1);
		WIPSprite* s2 = static_cast<WIPSprite*>(bodyUserData2);

		CHECK(s1->_collider);
		CHECK(s2->_collider);

		s1->_collider->on_end_contact(s2);
		s2->_collider->on_end_contact(s1);
	}
	
}



WIPPhysicsManager* WIPPhysicsManager::instance()
{
	static WIPPhysicsManager* _instance;
	if (!_instance)
		_instance = new WIPPhysicsManager();
	return _instance;
}

WIPPhysicsManager::WIPPhysicsManager()
{
	_phy_space = nullptr;
}

WIPPhysicsManager::~WIPPhysicsManager()
{

}

void WIPPhysicsManager::set_debug_camera(const WIPCamera* cam)
{
	debug_draw.cam = cam;
}

void WIPPhysicsManager::delete_body(b2Body* b)
{
	if (b)
	{
		_phy_space->DestroyBody(b);
	}
}

b2PolygonShape* WIPPhysicsManager::create_polygon()
{
	if (_phy_space)
	{
		b2PolygonShape* ret = new b2PolygonShape();
		ret->SetAsBox(1.f, 1.f);
		return ret;
	}
	return NULL;
}

b2Body* WIPPhysicsManager::create_body(b2BodyType tp)
{
	if (!_phy_space)
		return nullptr;
	b2BodyDef bodyDef;
	bodyDef.type = tp;
	bodyDef.position.Set(0.0f, 0.0f);
	b2Body* ret = _phy_space->CreateBody(&bodyDef);
	return ret;
}



bool WIPPhysicsManager::startup()
{
	if (_phy_space)
		delete _phy_space;

	b2Vec2 gravity(0.0f, 0.f);
	_phy_space = new b2World(gravity);
	velocityIterations = 6;
	positionIterations = 2;
	debug_draw.SetFlags(b2Draw::e_shapeBit);
	_phy_space->SetDebugDraw(&debug_draw);
	_phy_space->SetContactListener(&contact_listener);
	return true;
}

void WIPPhysicsManager::set_gravity(const RBVector2& g)
{
	_phy_space->SetGravity(b2Vec2(g.x, g.y));
}

void WIPPhysicsManager::shutdown()
{
	delete _phy_space;
}

void  WIPPhysicsManager::update(WIPScene* scene, f32 dt)
{

	for (b2Body* b = _phy_space->GetBodyList(); b; b = b->GetNext())
	{
		WIPSprite* s = (WIPSprite*)b->GetUserData();
		if (!s->_render)
			continue;
		if (!s->_collider->_active)
			continue;
		/*
		RBVector2 vert[4];
		b2Vec2 v1[4];
		RBVector2 scale2(s->_transform->scale_x, s->_transform->scale_y);
		s->get_anchor_vertices(vert);
		vert[0] *= scale2;
		vert[1] *= scale2;
		vert[2] *= scale2;
		vert[3] *= scale2;
		v1[0].x = vert[3].x*s->_collider->_cb_scale_x;
		v1[0].y = vert[3].y*s->_collider->_cb_scale_y;

		v1[1].x = vert[0].x*s->_collider->_cb_scale_x;
		v1[1].y = vert[0].y*s->_collider->_cb_scale_y;

		v1[2].x = vert[2].x*s->_collider->_cb_scale_x;
		v1[2].y = vert[2].y*s->_collider->_cb_scale_y;

		v1[3].x = vert[1].x*s->_collider->_cb_scale_x;
		v1[3].y = vert[1].y*s->_collider->_cb_scale_y;
		*/
		//s->_collider->_polygon_shape->Set(v1, 4);
		//lt lb rt rb
		//s->_collider->_polygon_shape->SetAsBox((v1[2].x - v1[0].x)*0.5f, (v1[0].y - v1[1].y)*0.5f, b2Vec2(0,0), 0);

		//b2Fixture* fixture = b->GetFixtureList();
		/*
		if (fixture)
		{
			b->DestroyFixture(fixture);
			b2FixtureDef fixturedef;
			fixturedef.userData = s;
			//fixturedef.isSensor = true;
			fixturedef.shape = s->_collider->_polygon_shape;
			b->CreateFixture(&fixturedef);
		}
		else
		{
			LOG_WARN("No fixture!");
		}
		*/
		b->SetTransform(b2Vec2(s->_transform->world_x, s->_transform->world_y), s->_transform->rotation);
		b->SetAwake(true);
		
	}


	_phy_space->Step(dt, velocityIterations, positionIterations);
	_phy_space->DrawDebugData();


	for (b2Body* b = _phy_space->GetBodyList(); b; b = b->GetNext())
	{
		WIPSprite* s = (WIPSprite*)b->GetUserData();
		if (!s->_render)
			continue;
		if (!s->_collider->_active)
			continue;
		b2Vec2 pos = b->GetPosition();
		f32 rad = b->GetAngle();
		b2Vec2* v = s->_collider->_polygon_shape->m_vertices;
		int count = s->_collider->_polygon_shape->m_count;
		f32 sx = v[0].x / s->_render->mesh.lb.x;
		f32 sy = v[0].y / s->_render->mesh.lb.y;

		s->translate_to(pos.x, pos.y);
		s->rotate_to(rad);

	}

}

WIPPhysicsManager* g_physics_manager = WIPPhysicsManager::instance();


void WIPPhysicsManager::WIPBox2dDebugDraw::DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	RBVector2 verts[4];
	for (int i = 0; i < 4; i++)
	{
		verts[i].x = vertices[i].x;
		verts[i].y = vertices[i].y;
	}
	g_rhi->begin_debug_context();
	g_rhi->change_debug_color(RBColorf::red);
	g_rhi->debug_draw_box(verts, cam);
	g_rhi->debug_submit();
	g_rhi->end_debug_context();
}

void WIPPhysicsManager::WIPBox2dDebugDraw::DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color)
{
	RBVector2 verts[4];
	for (int i = 0; i < 4; i++)
	{
		verts[i].x = vertices[i].x;
		verts[i].y = vertices[i].y;
	}
	g_rhi->begin_debug_context();
	g_rhi->change_debug_color(RBColorf(color.r, color.g, color.b, color.a));
	g_rhi->debug_draw_box(verts, cam);
	g_rhi->debug_submit();
	g_rhi->end_debug_context();
}

void WIPPhysicsManager::WIPBox2dDebugDraw::DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color)
{

}

void WIPPhysicsManager::WIPBox2dDebugDraw::DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color)
{

}

void WIPPhysicsManager::WIPBox2dDebugDraw::DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color)
{

}

void WIPPhysicsManager::WIPBox2dDebugDraw::DrawTransform(const b2Transform& xf)
{

}

void WIPPhysicsManager::WIPBox2dDebugDraw::DrawPoint(const b2Vec2& p, float32 size, const b2Color& color)
{

}