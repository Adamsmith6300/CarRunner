#include "Entity.h"

Entity::Entity()
:	pPhysHolder(new PhysHolder())
{
	mPosition = { 0.0f, 0.0f, 0.0f };
	mRight = { 1.0f, 0.0f, 0.0f };
	mUp = { 0.0f, 1.0f, 0.0f };
	mLook = { 0.0f, 0.0f, 1.0f };

	updateHPos();
}

Entity::Entity(POS position, POS right, POS up, POS look)
:	pPhysHolder(new PhysHolder()),
	mPosition(position),
	mRight(right),
	mUp(up),
	mLook(look)
{
	updateHPos();
}

void Entity::walk(float d) {

}

void Entity::Strafe(float d) {

}

DIRV Entity::GetPosition()const
{
	return XMLoadFloat3(&mPosition);
}

POS Entity::GetPosition3f()const
{
	return mPosition;
}

void Entity::SetPosition(float x, float y, float z)
{
	mPosition = POS(x, y, z);
	updateHPos();
}

void Entity::SetPosition(const POS& v)
{
	mPosition = v;
	updateHPos();
}

void Entity::updateHPos() {
	mHeadPos.x = mPosition.x;
	mHeadPos.y = mPosition.y + headOffset;
	mHeadPos.z = mPosition.z;
}

POS Entity::getHPos() {
	return mHeadPos;
}

DIRV Entity::GetRight()const
{
	return XMLoadFloat3(&mRight);
}

POS Entity::GetRight3f()const
{
	return mRight;
}

DIRV Entity::GetUp()const
{
	return XMLoadFloat3(&mUp);
}

POS Entity::GetUp3f()const
{
	return mUp;
}

DIRV Entity::GetLook()const
{
	return XMLoadFloat3(&mLook);
}

POS Entity::GetLook3f()const
{
	return mLook;
}

//PhysHolder
PhysHolder* Entity::GetPhysHolder() const
{
	return pPhysHolder;
}

void Entity::decrementJump()
{
	if(--jumps < 0.0f)
		return;
	pPhysHolder->Jump();
}

void Entity::resetJump(bool j)
{
	if (j)
		jumps = MAXJUMPS;
}

PhysHolder::PhysHolder()
{
	mVelocity = { 0.0f, 0.0f, 0.0f };
	mIntent = { 0.0f, 0.0f, 0.0f };
}

POS PhysHolder::getVelocity() const
{
	return mVelocity;
}

void PhysHolder::setVelocity(POS velocity)
{
	mVelocity = velocity;
}

POS PhysHolder::getIntent() const
{
	return mIntent;
}

void PhysHolder::setIntent(POS intent)
{
	mIntent = intent;
}

void PhysHolder::Jump()
{
	mIntent.y = 1.0f;
}
