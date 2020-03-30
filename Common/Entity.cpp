#include "Entity.h"

Entity::Entity()
:	pPhysHolder(new PhysicsEntity())
{
	mPosition = { 0.0f, 0.0f, 0.0f };
	mRight = { 1.0f, 0.0f, 0.0f };
	mUp = { 0.0f, 1.0f, 0.0f };
	mLook = { 0.0f, 0.0f, 1.0f };

	updateHPos();
}

Entity::Entity(POS position, POS right, POS up, POS look)
:	pPhysHolder(new PhysicsEntity()),
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

void Entity::calcAABB(std::vector<POS> boxVerts)
{
	//OutputDebugString(L"Calculating AABB");
	POS minVertex = POS(FLT_MAX, FLT_MAX, FLT_MAX);
	POS maxVertex = POS(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	XMMATRIX world = XMLoadFloat4x4(&World);
	//Loop through the 8 vertices describing the bounding box
	for (UINT i = 0; i < 8; i++)
	{
		//Transform the bounding boxes vertices to the objects world space
		XMVECTOR Vert = XMVectorSet(boxVerts[i].x, boxVerts[i].y, boxVerts[i].z, 0.0f);
		Vert = XMVector3TransformCoord(Vert, world);

		//Get the smallest vertex 
		minVertex.x = min(minVertex.x, XMVectorGetX(Vert));    // Find smallest x value in model
		minVertex.y = min(minVertex.y, XMVectorGetY(Vert));    // Find smallest y value in model
		minVertex.z = min(minVertex.z, XMVectorGetZ(Vert));    // Find smallest z value in model

		//Get the largest vertex 
		maxVertex.x = max(maxVertex.x, XMVectorGetX(Vert));    // Find largest x value in model
		maxVertex.y = max(maxVertex.y, XMVectorGetY(Vert));    // Find largest y value in model
		maxVertex.z = max(maxVertex.z, XMVectorGetZ(Vert));    // Find largest z value in model
	}

	//Store Bounding Box's min and max vertices
	boundingboxminvertex = {minVertex.x, minVertex.y, minVertex.z};
	boundingboxmaxvertex = {maxVertex.x, maxVertex.y, maxVertex.z};

	//std::wostringstream ss;
	//ss << minVertex.x << " " << minVertex.y << " " << minVertex.z << std::endl;
	//OutputDebugString(ss.str().c_str());
}

POS Entity::getCenter() const
{
	POS center = {boundingboxmaxvertex.x - boundingboxminvertex.x /2, boundingboxmaxvertex.y - boundingboxminvertex.y / 2, boundingboxmaxvertex.z - boundingboxminvertex.z / 2};
	center = { boundingboxminvertex.x + center.x, boundingboxminvertex.y + center.y, boundingboxminvertex.z + center.z };

	return center;
}

//PhysHolder
PhysicsEntity* Entity::GetPhysHolder() const
{
	return pPhysHolder;
}
