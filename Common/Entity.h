#pragma once
#include "../Physics/PhysicsEntity.h"

const float MAXJUMPS = 1.0f;

class Entity
{
private:
	POS mPosition;
	POS mHeadPos;
	float headOffset = 3;
	POS mRight;
	POS mUp;
	POS mLook;

	PhysicsEntity* pPhysHolder = nullptr;
	float jumps = MAXJUMPS;

public:

	Entity();
	Entity(POS position, POS right, POS up, POS look);
	~Entity() {};

	DIRV GetPosition()const;
	POS GetPosition3f()const;
	void SetPosition(float x, float y, float z);
	void SetPosition(const POS& v);
	void updateHPos();
	POS getHPos();

	DIRV GetRight()const;
	POS GetRight3f()const;
	DIRV GetUp()const;
	POS GetUp3f()const;
	DIRV GetLook()const;
	POS GetLook3f()const;

	void walk(float d);
	void Strafe(float d);

	PhysicsEntity* GetPhysHolder()const;
};