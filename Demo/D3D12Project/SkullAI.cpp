#include "SkullAI.h"

SkullAI::SkullAI(Entity* parentSkull) {
	this->parentSkull = parentSkull;
	SetInitPos();
}

SkullAI::SkullAI(Entity* leftSkull, Entity* rightSkull) {
	this->leftSkull = leftSkull;
	this->rightSkull = rightSkull;
}

void SkullAI::PrintDescription() {
	OutputDebugString(L"X: ");
	OutputDebugStringA(std::to_string((int) parentSkull->GetPosition3f().x).c_str());
	OutputDebugString(L", Y: ");
	OutputDebugStringA(std::to_string((int) parentSkull->GetPosition3f().y).c_str());
	OutputDebugString(L", Z: ");
	OutputDebugStringA(std::to_string((int) parentSkull->GetPosition3f().z).c_str());
	OutputDebugString(L"\n");
}

void SkullAI::SetInitPos() {
	initialPos = parentSkull->GetPosition3f();
}

void SkullAI::ResetPos() {
	parentSkull->SetPosition(initialPos);

}

POS SkullAI::CalcTeleportLocation(Entity* target) {
	int decision = rand() % 3 + 1;
	//int decision = 1;
	POS teleLoc = {0.0f, 0.0f, 0.0f};
	POS displacement = {0.0f, 0.0f, 0.0f};

	switch (decision) {
		case 1: // skull will be in front of target
			teleLoc.x = target->GetPosition3f().x;
			teleLoc.y = target->GetPosition3f().y + 1.0f;
			teleLoc.z = target->GetPosition3f().z + 2.0f;
			break;
		case 2: // skull will be right-front of the target
			teleLoc.x = target->GetPosition3f().x + 2.0f;
			teleLoc.y = target->GetPosition3f().y + 1.0f;
			teleLoc.z = target->GetPosition3f().z + 2.0f;
			break;
		case 3: // skull will be left-front of the target
			teleLoc.x = target->GetPosition3f().x - 2.0f;
			teleLoc.y = target->GetPosition3f().y + 1.0f;
			teleLoc.z = target->GetPosition3f().z + 2.0f;
			break;
		default:
			break;
	}

	displacement.x = teleLoc.x - parentSkull->GetPosition3f().x;
	displacement.y = teleLoc.y - parentSkull->GetPosition3f().y;
	displacement.z = teleLoc.z - parentSkull->GetPosition3f().z;

	/*OutputDebugStringA(std::to_string(displacement.x).c_str());
	OutputDebugString(L", ");
	OutputDebugStringA(std::to_string(displacement.y).c_str());
	OutputDebugString(L", ");
	OutputDebugStringA(std::to_string(displacement.z).c_str());
	OutputDebugString(L"\n");*/

	return displacement;
}

void SkullAI::SetSkullReady(bool ready) {
	this->ready = ready;
}

bool SkullAI::isReady() const {
	return this->ready;
}

void SkullAI::SetSkullMoved(bool moved) {
	this->moved = moved;
}

bool SkullAI::isMoved() const {
	return this->moved;
}

bool SkullAI::isInRange(Entity* player) const {
	
	if (player->GetPosition3f().z <= parentSkull->GetPosition3f().z + 10.0f 
		&& player->GetPosition3f().z >= parentSkull->GetPosition3f().z - 10.0f) {
		return true;
	}
	return false;
}

Entity* SkullAI::CalcClosest(Entity* p1, Entity* p2) {
	// checking one "active" player 
	if (p2 == nullptr) {
		if (isInRange(p1)) { // player 1 is closer
			return p1;
		}
	}
	// checking both active players
	else {
		if ((CalcDistance(p1) > CalcDistance(p2))) { // player 2 is closer
			return p2;
		}
		else if ((CalcDistance(p1) < CalcDistance(p2))) { // player 1 is closer
			return p1;
		}
	}
	return nullptr;
}

float SkullAI::CalcDistance(Entity* player) {
	float xdif = fabsf(player->GetPosition3f().x - parentSkull->GetPosition3f().x);
	float zdif = fabsf(player->GetPosition3f().z - parentSkull->GetPosition3f().z);

	float xpow = pow(xdif, 2);
	float zpow = pow(zdif, 2);

	return sqrtf(xpow+zpow);
}

