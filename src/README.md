# 记录

-	仅game object的生命周期？（使用智能指针）
-	所有object属于同一个world，所有的创建删除都属于world，scene只是world的一个“窗口”.
-	不同component之间的相互调用，直接调用与消息.
-	string替换hash
-	优化:WIPPhysicsManeger::update
				WIPSprite::rotate_to
				WIPSprite::translate_to
						WIPSprite::update_world(节点储存结构已优化。)
-	四叉树优化：在object中记录自己所在四叉树位置，先检测该结点是否变化了四叉树区域，若没变就只更新位置，不操作四叉树；否则删除结点重新插入。注意大部分对象是不会变化四叉树区域的。
-	Collider和Mesh分离，Collider使用参考:
	```cpp
	void Collider::CreateRigidbody2D()
	{
		b2BodyDef bodyDef;
		bodyDef.type = IsKinematic ? b2_kinematicBody : b2_dynamicBody;
		bodyDef.userData = this;
		bodyDef.bullet = (Continuous_Collision?);
		bodyDef.linearDamping = LinearDrag;
		bodyDef.angularDamping = AngularDrag;
		bodyDef.gravityScale = GravityScale;
		bodyDef.fixedRotation = FixedAngle;
		bodyDef.allowSleep = SleepingMode != kNeverSleep2D;
		bodyDef.awake = m_SleepingMode != kStartAsleep2D;;
		//Fetch pose from transform (&bodyDef.position, &bodyDef.angle);
		// Create the body.
		_body = box2d_world->CreateBody (&bodyDef);
		// Calculate the collider body mass.
	}
	b2FixtureDef fixtureDef;
	fixtureDef.isSensor = true;
	```
	包含物理的组件可以有多个collider？userdata设置为Collider本身！采用变换更新你更新collider而不是重新创建fixture，重新创建fixture会使得某些东西失效。