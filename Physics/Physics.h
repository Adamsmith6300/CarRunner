#include "../Common/Entity.h"

#define POS DirectX::XMFLOAT3

class Physics
{
public:
	static bool YPhysics(POS& pos, PhysHolder* e, float deltaTime);
	static void Gravity(POS& pos, POS& velocity, float deltaTime);
	static void Jump(POS& pos, POS& velocity, POS& intent, float deltaTime);
	static bool Ground(POS& pos, POS& velocity);
	bool collisionCheck(Entity& firstEntity, Entity& secondEntity);
	void handleCollision(Entity& firstEntity, Entity& secondEntity);
	XMFLOAT3 makeCeil(XMFLOAT3 first, XMFLOAT3 second);
	XMFLOAT3 makeFloor(XMFLOAT3 first, XMFLOAT3 second);
};