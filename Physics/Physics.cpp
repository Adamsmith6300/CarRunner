#include "Physics.h"

void Physics::XYZPhysics(POS& pos, PhysicsEntity* e, float deltaTime, float velX, float velZ)
{
	POS velocity = e->getVelocity()
		, intent = e->getIntent();
	float angle = e->getAngle();

	XZPhysics(pos, velocity, intent, angle, velX, velZ);
	e->resetJump(YPhysics(pos, velocity, intent, deltaTime));

	e->setVelocity(velocity);
	e->setIntent();
}

void Physics::XZPhysics(POS& pos, POS& velocity, POS& intent, float angle, float velX, float velZ)
{
	CalcAngleIntent(velocity, intent, angle);
	SpeedLimit(velocity);
	ChangePos(pos, velocity, velX, velZ);
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

void Physics::ChangePos(POS& pos, POS& velocity, float velX, float velZ)
{
	pos.z += velocity.z * velZ;
	pos.x += velocity.x * velX;
}

bool Physics::YPhysics(POS& pos, POS& velocity, POS& intent, float deltaTime)
{
	Gravity(pos, velocity, deltaTime);
	Jump(pos, velocity, intent, deltaTime);
	return Ground(pos, velocity);
}

void Physics::Gravity(POS& pos, POS& velocity, float deltaTime)
{
    // v = m/s� * s
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
	if (pos.y >= -3)
		return false;
	pos.y = -3;
	velocity.y = 0;
	return true;
}

bool Physics::collisionCheck(Entity* firstEntity, Entity* secondEntity)
{
	//std::wostringstream ss;
	//ss << XMVectorGetX(firstboxmin) << " " << XMVectorGetX(secondboxmin)<< std::endl;
	//ss << "blockmin " << secondEntity.boundingboxminvertex.x << " " << secondEntity.boundingboxminvertex.y << " " << secondEntity.boundingboxminvertex.z << std::endl;
	//ss << "Firstbox center"<< firstEntity.getCenter().x << " " << firstEntity.getCenter().y << " " << firstEntity.getCenter().z << std::endl;
	//ss << "Secondbox center" << secondEntity.getCenter().x << " " << secondEntity.getCenter().y << " " << secondEntity.getCenter().z << std::endl;
	//ss << "Firstbox Vertex " << firstEntity->boundingboxminvertex.x << " " << firstEntity->boundingboxminvertex.y << " " << firstEntity->boundingboxminvertex.z << std::endl;
	//ss << "Secondbox Vertex " << secondEntity->boundingboxminvertex.x << " " << secondEntity->boundingboxminvertex.y << " " << secondEntity->boundingboxminvertex.z << std::endl;
	//ss << "normal " <<normal.x << " " << normal.y << " " << normal.z << std::endl;
	//ss << std::endl;
	//OutputDebugString(ss.str().c_str());

	//Is obj1's max X greater than obj2's min X? If not, obj1 is to the LEFT of obj2
	if (firstEntity->boundingboxmaxvertex.x > secondEntity->boundingboxminvertex.x) {
		//Is obj1's min X less than obj2's max X? If not, obj1 is to the RIGHT of obj2
		if (firstEntity->boundingboxminvertex.x < secondEntity->boundingboxmaxvertex.x) {
			//Is obj1's max Y greater than obj2's min Y? If not, obj1 is UNDER obj2
			if (firstEntity->boundingboxmaxvertex.y > secondEntity->boundingboxminvertex.y) {
				//Is obj1's min Y less than obj2's max Y? If not, obj1 is ABOVE obj2
				if (firstEntity->boundingboxminvertex.y < secondEntity->boundingboxmaxvertex.y) {
					//Is obj1's max Z greater than obj2's min Z? If not, obj1 is IN FRONT OF obj2
					if (firstEntity->boundingboxmaxvertex.z > secondEntity->boundingboxminvertex.z) {
						//Is obj1's min Z less than obj2's max Z? If not, obj1 is BEHIND obj2
						if (firstEntity->boundingboxminvertex.z < secondEntity->boundingboxmaxvertex.z) {
							//If we've made it this far, then the two bounding boxes are colliding
							return true;
						}
					}
				}
			}
		}
	}

	//If the two bounding boxes are not colliding, then return false
	//OutputDebugString(L"No collision \n");
	return false;
}

void Physics::handleCollision(Entity* firstEntity, Entity* secondEntity, POS& pos ,float deltatime)
{
	PhysicsEntity* first = firstEntity->GetPhysHolder();

	XMFLOAT3 firstCenter = firstEntity->getCenter();
	XMFLOAT3 secondCenter = secondEntity->getCenter();

	//debugging output to check if values are correct//
	/*std::wostringstream ss;
	ss << XMVectorGetX(firstboxmin) << " " << XMVectorGetX(secondboxmin)<< std::endl;
	ss << "Firstbox "<<firstboxcenter.x << " " << firstboxcenter.y << " " << firstboxcenter.z << std::endl;
	ss << "Secondbox " << secondboxcenter.x << " " << secondboxcenter.y << " " << secondboxcenter.z << std::endl;
	ss << "normal " <<normal.x << " " << normal.y << " " << normal.z << std::endl;
	ss << std::endl;
	OutputDebugString(ss.str().c_str());*/

	XMFLOAT3 intMin = Physics::makeCeil(firstEntity->boundingboxminvertex, secondEntity->boundingboxminvertex);
	XMFLOAT3 intMax = Physics::makeFloor(firstEntity->boundingboxmaxvertex, secondEntity->boundingboxmaxvertex);

	//area of the intersection of the two boxes
	XMFLOAT3 intersection = { intMax.x - intMin.x,intMax.y - intMin.y,intMax.z - intMin.z };

	float ax = fabs(intersection.x);
	float ay = fabs(intersection.y);
	float az = fabs(intersection.z);

	//Calculating x y and z normals for faces
	float sx = firstCenter.x < secondCenter.x ? -1.0f : 1.0f;
	float sy = firstCenter.y < secondCenter.y ? -1.0f : 1.0f;
	float sz = firstCenter.z < secondCenter.z ? -1.0f : 1.0f;

	//XMFLOAT3 pos = firstEntity.GetPosition3f();

	//checking which face is colliding with and multiplying collision normal of face
	if (ax <= ay && ax <= az) {
		
		if (sx > 0) {
			pos.x += 0.25;
		}
		else {
			pos.x -= 0.25;
		}
	}
	else if (ay <= az) {

		XMFLOAT3 vel = first->getVelocity();
		if (sy > 0) {
			vel.y = 0;
			pos.y += 0.015;
		}
		else {
			vel.y = 0;
			pos.y -= 0.01;
		}	

		first->setVelocity(vel);
		first->resetJump(true);
	}
	else {

		if (sz > 0) {
			pos.z += 0.25;
		}
		else {
			pos.z -= 0.25;
		}
	}
}

XMFLOAT3 Physics::makeCeil(XMFLOAT3 first, XMFLOAT3 second)
{
	XMFLOAT3 result;
	if (second.x > first.x) first.x = second.x;
	if (second.y > first.y) first.y = second.y;
	if (second.z > first.z) first.z = second.z;
	return first;
}

XMFLOAT3 Physics::makeFloor(XMFLOAT3 first, XMFLOAT3 second)
{
	if (second.x < first.x) first.x = second.x;
	if (second.y < first.y) first.y = second.y;
	if (second.z < first.z) first.z = second.z;
	return first;
}



