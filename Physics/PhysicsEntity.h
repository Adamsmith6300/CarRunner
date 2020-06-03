#pragma once
#include "../Common/d3dUtil.h"

#define POS DirectX::XMFLOAT3
#define DIRV DirectX::XMVECTOR

class PhysicsEntity {
private:
	POS mVelocity, mIntent;
	const float fInitJumps;
	float fAngle, fJumps;
public:
	PhysicsEntity(
		POS velocity = { 0.0f, 0.0f, 0.0f },
		POS intent = { 0.0f, 0.0f, 0.0f },
		float angle = 0.0f,
		float jumps = 1.0f
		)
	:	mVelocity(velocity),
		mIntent(intent),
		fAngle(angle),
		fJumps(jumps),
		fInitJumps(jumps)
	{}

	POS getVelocity()const;
	void setVelocity(POS velocity);
	POS getIntent()const;
	void setIntent();
	float getAngle()const;

	void setAnglePositive();
	void setAngleNegative();
	void setAngle(float input);
	void setZIntentPositive();
	void setZIntentNegative();
	void setXIntentPositive();
	void setXIntentNegative();
	void decrementJump();
	void resetJump(bool j);
};