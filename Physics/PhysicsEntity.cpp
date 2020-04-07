#include "PhysicsEntity.h"

POS PhysicsEntity::getVelocity() const
{
	return mVelocity;
}

void PhysicsEntity::setVelocity(POS velocity)
{
	mVelocity = velocity;
}

POS PhysicsEntity::getIntent() const
{
	return mIntent;
}

void PhysicsEntity::setIntent()
{
	mIntent = { 0.0f, 0.0f, 0.0f };
}

float PhysicsEntity::getAngle() const
{
	return fAngle;
}

void PhysicsEntity::setAnglePositive()
{
	fAngle += 3.14f / 4.0f;
}

void PhysicsEntity::setAngleNegative()
{
	fAngle -= 3.14f / 4.0f;
}

void PhysicsEntity::setAngle(float input)
{
	fAngle += input;
}

void PhysicsEntity::setZIntentPositive()
{
	++mIntent.z;
}

void PhysicsEntity::setZIntentNegative()
{
	--mIntent.z;
}

void PhysicsEntity::setXIntentPositive()
{
	++mIntent.x;
}

void PhysicsEntity::setXIntentNegative()
{
	--mIntent.x;
}

void PhysicsEntity::decrementJump()
{
	if (--fJumps < 0.0f)
		return;
	mIntent.y = 1.0f;
}

void PhysicsEntity::resetJump(bool j)
{
	if(j)
		fJumps = fInitJumps;
}