#include "Physics.h"

void Physics::XYZPhysics(POS& pos, PhysicsEntity* e, float deltaTime)
{
	POS velocity = e->getVelocity()
		, intent = e->getIntent();
	float angle = e->getAngle();

	XZPhysics(pos, velocity, intent, angle, deltaTime);
	e->resetJump(YPhysics(pos, velocity, intent, deltaTime));

	e->setVelocity(velocity);
	e->setIntent();
}

void Physics::XZPhysics(POS& pos, POS& velocity, POS& intent, float angle, float deltaTime)
{
	CalcAngleIntent(velocity, intent, angle);
	SpeedLimit(velocity);
	ChangePos(pos, velocity, deltaTime);
}

void Physics::CalcAngleIntent(POS& velocity, POS& intent, float angle)
{
	float z = intent.z * cos(angle) + intent.x * cos(angle + pi_h),
		  x = intent.z * sin(angle) + intent.x * sin(angle + pi_h);

	velocity.z += z * speedForce;
	velocity.x += x * speedForce;
}

void Physics::SpeedLimit(POS& velocity)
{
	// z
	if (velocity.z > friction)
		velocity.z -= friction;
	else if (velocity.z < -friction)
		velocity.z += friction;
	else
		velocity.z = 0;

	if (velocity.z > velocitycap)
		velocity.z = velocitycap;
	else if (velocity.z < -velocitycap)
		velocity.z = -velocitycap;

	// x
	if (velocity.x > friction)
		velocity.x -= friction;
	else if (velocity.x < -friction)
		velocity.x += friction;
	else
		velocity.x = 0;

	if (velocity.x > velocitycap)
		velocity.x = velocitycap;
	else if (velocity.x < -velocitycap)
		velocity.x = -velocitycap;
}

void Physics::ChangePos(POS& pos, POS& velocity, float deltaTime)
{
	pos.z += velocity.z * deltaTime;
	pos.x += velocity.x * deltaTime;
}

bool Physics::YPhysics(POS& pos, POS& velocity, POS& intent, float deltaTime)
{
	Gravity(pos, velocity, deltaTime);
	Jump(pos, velocity, intent, deltaTime);
	return Ground(pos, velocity);
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
}

bool Physics::Ground(POS& pos, POS& velocity)
{
	if (pos.y >= 0)
		return false;
	pos.y = 0;
	velocity.y = 0;
	return true;
}


