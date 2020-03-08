#include "../Common/Entity.h"

#define POS DirectX::XMFLOAT3

class Physics
{
public:
	static bool YPhysics(POS& pos, PhysHolder* e, float deltaTime);
	static void Gravity(POS& pos, POS& velocity, float deltaTime);
	static void Jump(POS& pos, POS& velocity, POS& intent, float deltaTime);
	static bool Ground(POS& pos, POS& velocity);
};