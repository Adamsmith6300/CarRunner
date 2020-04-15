#pragma once

#include "../../Common/Entity.h"

class SkullAI {

private:

	Entity* parentSkull;

	POS initialPos = {};
	POS nextDest = {};

	bool moved = false; // check if skull has been moved
	bool ready = true; // check if skull is ready to move

public:
	//SkullAI() {}
	SkullAI(Entity* parentSkull);
	~SkullAI() {};

	void PrintDescription(); // testing purposes
	void SetInitPos();
	void ResetPos();
	bool isInRange(Entity* player) const;

	void SetSkullReady(bool ready);
	bool isReady() const;
	void SetSkullMoved(bool moved);
	bool isMoved() const;

	Entity* CalcClosest(Entity* p1, Entity* p2);
	float CalcDistance(Entity* player);
	POS CalcMove(Entity* target);


};