#pragma once

#include "../../Common/Entity.h"

class SkullAI {

private:

	Entity* parentSkull;
	POS posAccumulator = { 0.0f, 0.0f, 0.0f };
	bool updated = false;

public:
	SkullAI(Entity* parentSkull);
	~SkullAI() {};

	void PrintDescription(); // testing purposes
	void UpdatePos(const POS& newPos);
	POS GetPosAccumulator();
	bool isInRange(Entity* player) const;

	Entity* CalcClosest(Entity* p1, Entity* p2);
	float CalcDistance(Entity* player);
	POS CalcMove(Entity* target);


};