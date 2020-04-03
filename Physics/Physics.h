#include "../Common/Entity.h"

#define POS DirectX::XMFLOAT3

class Physics
{
public:
	static void XYZPhysics(POS& pos, PhysicsEntity* e, float deltaTime, float velX, float velZ);
	static bool collisionCheck(Entity* firstEntity, Entity* secondEntity);
	static void handleCollision(Entity* firstEntity, Entity* secondEntity, POS& pos, float deltatime);
	static XMFLOAT3 makeCeil(XMFLOAT3 first, XMFLOAT3 second);
	static XMFLOAT3 makeFloor(XMFLOAT3 first, XMFLOAT3 second);
private:
	static constexpr const float gravity = -1.0f,
								 jumpForce = 2.0f,
								 pi_h = 1.57f,
								 speedForce = 1.0f,
								 velocitycap = 1.0f,
								 friction = 0.3f;

	static void XZPhysics(POS& pos, POS& velocity, POS& intent, float angle, float velX, float velZ);
	static void CalcAngleIntent(POS& velocity, POS& intent, float angle);
	static void SpeedLimit(POS& velocity);
	static void ChangePos(POS& pos, POS& velocity, float velX, float velZ);

	static bool YPhysics(POS& pos, POS& velocity, POS& intent, float deltaTime);
	static void Gravity(POS& pos, POS& velocity, float deltaTime);
	static void Jump(POS& pos, POS& velocity, POS& intent, float deltaTime);
	static bool Ground(POS& pos, POS& velocity);
};