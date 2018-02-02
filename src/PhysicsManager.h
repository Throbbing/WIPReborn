#pragma once

#include "Box2d/Box2D.h"
#include "RBBasedata.h"
#include "Vector2.h"
#include "RefCount.h"

class WIPScene;


class WIPPhysicsManager : public FRefCountedObject
{
public:
	static WIPPhysicsManager* instance();
	//create a space,but now we don't support it
	void create_space(){}

	b2PolygonShape* create_polygon();
	b2Body* create_body(b2BodyType tp);

	void delete_body(b2Body* b);

	bool startup();
	void update(WIPScene* scene,f32 dt);
	void shutdown();

	void set_gravity(const RBVector2& g);
	void set_debug_camera(const class WIPCamera* cam);
protected:
	WIPPhysicsManager();
	~WIPPhysicsManager();

private:
	class WIPBox2dDebugDraw : public b2Draw
	{
	public:
		void DrawPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
		void DrawSolidPolygon(const b2Vec2* vertices, int32 vertexCount, const b2Color& color);
		void DrawCircle(const b2Vec2& center, float32 radius, const b2Color& color);
		void DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
		void DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
		void DrawTransform(const b2Transform& xf);
		void DrawPoint(const b2Vec2& p, float32 size, const b2Color& color);

		const class WIPCamera* cam;
	};
	class WIPBox2dSpriteContactListener : public b2ContactListener
	{
		void BeginContact(b2Contact* contact);
		void EndContact(b2Contact* contact);
	};
	b2World* _phy_space;
	i32 velocityIterations, positionIterations;
	WIPBox2dDebugDraw debug_draw;
	WIPBox2dSpriteContactListener contact_listener;
	
};

extern WIPPhysicsManager* g_physics_manager;