#include "HealthPickUp.h"
#include "../../../GameObject.h"
#include "../Player/PlayerHealth.h"
#include "../../Emitter.h"
#include "../../../../Managers/AudioManager/AudioManager.h"
#include "../../../../Shared/CommonIncludes.h"
#include "../Player/PlayerScript.h"
#include "../../../../Managers/Factory/ObjectFactory.h"
#include "../../../../Managers/Scene/Scene.h"
Component* HealthPickUp::Clone()
{
	return new HealthPickUp(*this);
}

void HealthPickUp::Initialize()
{
	std::vector<Component*> tempVector = { nullptr };
	tempVector = m_pParent->GetComponents(EMITTER);
	m_emiters.reserve(tempVector.size());
	m_player = nullptr;
	m_moveToPlayer = false;
	for (size_t i = 0; i < tempVector.size(); i++)
	{
		m_emiters.push_back(dynamic_cast<Emitter*>(tempVector[i]));
	}
}
void HealthPickUp::Update()
{
	//If i should move towards the player
	if (m_moveToPlayer && m_player)
	{
		XMFLOAT3 plyerPos, myPos;
		plyerPos = m_player->GetTransform().GetPosition();
		myPos = m_pParent->GetTransform().GetPosition();
		plyerPos.y = myPos.y;
		//Move close to player
		m_pParent->GetTransform().MoveTowards(plyerPos, 750.0f * static_cast<float>(GameTime::GetInstance()->DeltaTime()), false);
		float difX, difZ;
		difX = plyerPos.x - myPos.x;
		difZ = plyerPos.z - myPos.z;
		//if im close enough I hit the player
		if ((difX > -5 && difX < 5) && (difZ > -5 && difZ < 5))
		{
			HitPlayer();
		}
	}
}
void HealthPickUp::Shutdown()
{

}
void HealthPickUp::OnTriggerEnter(Collider* coll)
{
	if (coll->GetGameObject()->GetTag() == "Player")
	{
		m_player = coll->GetGameObject();
		m_moveToPlayer = true;
	}
}

void HealthPickUp::HitPlayer()
{
	PlayerHealth *playerHealth = dynamic_cast<PlayerHealth*>(m_player->GetScript("PlayerHealth"));
	//If player is dead or isnt a player "destroy" this objects
	if (!playerHealth || playerHealth->GetCurrHealth() <= 0)
	{
		m_pParent->SetActive(false);
		ObjectFactory::GetInstance().ReturnObject(m_pParent->GetName(), m_pParent);
		return;
	}
	//If player is alivegive hime some health and play a particle effect and return the object
	dynamic_cast<PlayerScript*>(m_player->GetScript("PlayerScript"))->PlayHealthPotionEffect();
	int healAmount = static_cast<int>(playerHealth->GetMaxHealth() * m_percentHealAmount);
	playerHealth->AddToCurrentHealth(healAmount);
	m_pParent->SetActive(false);
	ObjectFactory::GetInstance().ReturnObject(m_pParent->GetName(), m_pParent);
	AudioManager::GetInstance()->PlayUnique(XACT_WAVEBANK_PLAYER_WAVE_BANK::HEALTHPICKUP);
}