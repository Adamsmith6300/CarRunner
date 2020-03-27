#pragma once

#include "../Physics/PhysicsEntity.h"
#include "MathHelper.h"
using namespace DirectX;

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

	//world matrix used for collision stuff
	XMFLOAT4X4 World = MathHelper::Identity4x4();
	POS boundingboxminvertex;
	POS boundingboxmaxvertex;
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

	void calcAABB(std::vector<POS> boxVerts);
	POS getCenter()const;

	void walk(float d);
	void Strafe(float d);

	PhysicsEntity* GetPhysHolder()const;
};