#include "SkullAI.h"

SkullAI::SkullAI(Entity* parentSkull) {
	this->parentSkull = parentSkull;
}

void SkullAI::PrintDescription() {
	OutputDebugString(L"X: ");
	OutputDebugStringA(std::to_string((int) parentSkull->GetPosition3f().x).c_str());
	OutputDebugString(L", Y: ");
	OutputDebugStringA(std::to_string((int)parentSkull->GetPosition3f().y).c_str());
	OutputDebugString(L", Z: ");
	OutputDebugStringA(std::to_string((int)parentSkull->GetPosition3f().z).c_str());
	OutputDebugString(L"\n");
}

void SkullAI::UpdatePos(const POS& mov) {
	posAccumulator.x += mov.x;
	posAccumulator.y += mov.y;
	posAccumulator.z += mov.z;
	updated = true;
}

POS SkullAI::GetPosAccumulator() {
	if (updated) {
		return this->posAccumulator;
	}
	return parentSkull->GetStartPosition3f();
}

POS SkullAI::CalcMove(Entity* target) {
	int randDec = rand() % 20 + 1;
	int xMov = rand() % 5 + 2;
	int zMov = rand() % 5 + 2;
	POS moveLoc = { 0.0f, 0.0f, 0.0f };
	POS displacement = { 0.0f, 0.0f, 0.0f };

	if (randDec <= 5) {
		moveLoc.x = target->GetPosition3f().x + xMov;
		moveLoc.z = target->GetPosition3f().z - zMov;
	}
	else if (randDec > 5 && randDec <= 10) {
		moveLoc.x = target->GetPosition3f().x + xMov;
		moveLoc.z = target->GetPosition3f().z + zMov;
	}
	else if (randDec > 10 && randDec <= 15) {
		moveLoc.x = target->GetPosition3f().x - xMov;
		moveLoc.z = target->GetPosition3f().z - zMov;
	}
	else if (randDec > 15) {
		moveLoc.x = target->GetPosition3f().x - xMov;
		moveLoc.z = target->GetPosition3f().z + zMov;
	}

	displacement.x = moveLoc.x - parentSkull->GetPosition3f().x;
	displacement.y = 0.35;
	displacement.z = moveLoc.z - parentSkull->GetPosition3f().z;

	return displacement;
}

bool SkullAI::isInRange(Entity* player) const {
	
	if (player->GetPosition3f().z <= parentSkull->GetStartPosition3f().z + 10.0f 
		&& player->GetPosition3f().z >= parentSkull->GetStartPosition3f().z - 10.0f) {
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

