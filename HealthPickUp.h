#pragma once
#include "../../Script.h"

class Emitter;
class HealthPickUp : public Script
{
private:
	float m_percentHealAmount = 0.1f;
	std::vector<Emitter*> m_emiters;
	bool m_moveToPlayer = false;
	GameObject *m_player = nullptr;
	void HitPlayer();
public:
	void Initialize();
	void Update();
	void Shutdown();
	Component* Clone();
	void OnTriggerEnter(Collider* coll);
};