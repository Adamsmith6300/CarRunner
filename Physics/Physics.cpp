#include "Physics.h"

const float gravity = -1
		  , jumpForce = 2;

bool Physics::YPhysics(POS& pos, PhysHolder* e, float deltaTime)
{
	POS velocity = e->getVelocity()
	  , intent = e->getIntent();

	Gravity(pos, velocity, deltaTime);
	Jump(pos, velocity, intent, deltaTime);
	bool resetJump = Ground(pos, velocity);

	e->setVelocity(velocity);
	e->setIntent(intent);
	return resetJump;
}

void Physics::Gravity(POS& pos, POS& velocity, float deltaTime)
{
    // v = m/s² * s
	velocity.y += gravity * deltaTime;
    // d = m/s * s
	pos.y += velocity.y * deltaTime;
}

void Physics::Jump(POS& pos, POS& velocity, POS& intent, float deltaTime)
{
	velocity.y += intent.y * jumpForce;
	pos.y += velocity.y * deltaTime;
	intent.y = 0.0f;
}

bool Physics::Ground(POS& pos, POS& velocity)
{
	if (pos.y >= 0)
		return false;
	pos.y = 0;
	velocity.y = 0;
	return true;
}


