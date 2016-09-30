#include "Survival.h"
#include "../../../GameObject.h"
#include "../../../../Managers/Scene/Scene.h"
#include "../../../../Managers/Factory/ObjectFactory.h"
#include "../Enemies/Base/BaseEnemy.h"
#include "../../../../Managers/StateManager/States/GamePlayState.h"
#include "../../UIComponent.h"
#include "../../../../Managers/InputManager/InputManager.h"
#include "../../../../Managers/AudioManager/AudioManager.h"
#define MAX_ENEMIES_ON_SCREEN 75
#define TIME_BETWEEN_WAVES 15.0f
#define TIME_BETWEEN_ENEMIES_SPAWN 0.3f
#define TIMED_WAVE_INTIAL_TIME 60.0f
#define FADE_IN_TIME 7.0f
void Survival::Initialize()
{
	m_currentWave = 0;
	m_hpWave = rand() % 4 + 7;
	m_timedWave = rand() % 5 + 11;
	for (int i = 0; i < 3; i++)
		m_enemiesLeft[i] = 0;
	m_enemiesName[0] = "Grunt";
	m_enemiesName[1] = "Archer";
	m_enemiesName[2] = "GruntCommander";
	m_eventName = "EnemyHasDied";
	EventManager::GetInstance()->AddEvent(m_eventName);
	EventManager::GetInstance()->RegisterListener(m_eventName, this);
	m_timeBetweenWaves = TIME_BETWEEN_WAVES;
	m_timedWaveTimer = TIMED_WAVE_INTIAL_TIME;
	m_goldForWave = 5;
	m_isHPWave = false;
	m_isTimedWave = false;
	m_waveOver = false;
	m_spawnPosIndex = 0;
	m_inbetweenEnemies = TIME_BETWEEN_ENEMIES_SPAWN;
	m_aliveEnemies = 0;
	(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[0]))->GetText()->SetText("Wave: " + to_string(m_currentWave));
	(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[2]))->SetActive(true);
	m_infoOffTimer = 5.0f;
	m_infoIsOn = false;
	m_fadeInTimer = 0.0f;
	for (auto itr = m_paramaters.begin(); itr != m_paramaters.end(); itr++)
	{
		std::string temp = itr->first;
		std::string delim = "-";
		temp = temp.substr(0, temp.find(delim));
		if (temp == "Location")
			m_spawnPositions.push_back(itr->second);
	}
	for (int i = 0; i < CLOSEST_POINTS_SIZE; i++)
		m_closestPos[i] = m_spawnPositions[0];
}

void Survival::Update()
{
	UpdateTimers();
	if (!m_isTimedWave)
	{
		if (m_enemiesLeft[0] <= 0 && m_enemiesLeft[1] <= 0 && m_enemiesLeft[2] <= 0 && !m_waveOver && m_aliveEnemies <= 0)
		{
			WaveFinished();
			m_waveOver = true;
		}
	}
	else
	{
		if (!m_waveOver && m_aliveEnemies <= 0 && m_timedWaveTimer <= 0.0f)
		{
			WaveFinished();
			m_waveOver = true;
		}
	}
	if (m_waveOver && m_timeBetweenWaves <= 0.0f)
	{
		NewWave();
	}
	if (!m_waveOver && m_inbetweenEnemies <= 0.0f && !m_isTimedWave)
	{
		SpawnMoreEnemies();
		m_inbetweenEnemies = TIME_BETWEEN_ENEMIES_SPAWN;
	}
	else if (!m_waveOver && m_inbetweenEnemies <= 0.0f && m_timedWaveTimer > 0.0f)
	{
		SpawnMoreEnemies();
		m_inbetweenEnemies = TIME_BETWEEN_ENEMIES_SPAWN;
	}
	if (m_infoIsOn && m_infoOffTimer <= 0.0f)
	{
		m_infoOffTimer = 5.0f;
		m_infoIsOn = false;
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[2]))->SetActive(false);
	}
	if ((InputManager::GetInstance()->GetKeyboard()->IsKeyDown(DIK_LCONTROL) || InputManager::GetInstance()->GetKeyboard()->IsKeyDown(DIK_RCONTROL)) && InputManager::GetInstance()->GetKeyboard()->WasKeyPressedThisFrame(DIK_NUMPAD4))
	{
		WaveFinished();
		NewWave();
	}
}

void Survival::Shutdown()
{
	EventManager::GetInstance()->UnregisterListener(m_eventName, this);
}

Component * Survival::Clone()
{
	return new Survival(*this);
}

void Survival::OnEvent(std::string _eventName)
{
	if (_eventName == m_eventName)
	{
		m_aliveEnemies--;
	}
}
//Adds a spawn location to the script so it can put enemies at that location
void Survival::AddSpawnPosition(XMFLOAT3 _pos)
{
	m_spawnPositions.push_back(_pos);
}
//Updates All timers in class
void Survival::UpdateTimers()
{
	float dTime = static_cast<float>(GameTime::GetInstance()->DeltaTime());
	//If wave is over update time between wave and fade in the sound for the next wave
	if (m_aliveEnemies <= 0 && m_waveOver)
	{
		m_timeBetweenWaves -= dTime;
		if (m_timeBetweenWaves <= 7.5f && m_timeBetweenWaves >= 7.0f)
			AudioManager::GetInstance()->PlayUnique(WAVECOUNTDOWN);
		if (m_timeBetweenWaves < FADE_IN_TIME && m_fadeInTimer > 0.0f)
		{
			m_fadeInTimer -= dTime;
			if (m_bgmVolume != 0.0f)
			{
				float tempBGMVolume, percentFadeIn;
				percentFadeIn = 1 - (m_fadeInTimer / FADE_IN_TIME);
				tempBGMVolume = 0.05 + ((m_bgmVolume - 0.05f) * percentFadeIn);
				AudioManager::GetInstance()->SetBGMVolume(tempBGMVolume);
			}
		}
		//Change the color of the count down based how much is left
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[0]))->GetText()->SetColor(XMFLOAT4(0.0f, 0.0f, 0.0f, 1));
		if (m_timeBetweenWaves < (TIME_BETWEEN_WAVES * 0.33f))
		{
			(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[1]))->GetText()->SetColor(XMFLOAT4(1, 0, 0, 1));
			XMFLOAT4 color = XMFLOAT4(0.196078449f, 0.803921640f, 0.196078449f, 1.000000000f);
			(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[0]))->GetText()->SetColor(color);
		}
		else if (m_timeBetweenWaves < (TIME_BETWEEN_WAVES * 0.66f))
		{
			(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[1]))->GetText()->SetColor(XMFLOAT4(1, 1, 0, 1));
			XMFLOAT4 color = XMFLOAT4(0.501960814f, 0.501960814f, 0.501960814f, 1.000000000f);
			(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[0]))->GetText()->SetColor(color);
		}
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[1]))->GetText()->SetText(to_string(static_cast<int>(m_timeBetweenWaves)));
	}
	//If it is a timed wave and not a normal one Update timer and change the color
	if (m_isTimedWave)
	{
		m_timedWaveTimer -= dTime;
		if (m_timedWaveTimer < 0.0f)
			m_timedWaveTimer = 0.0f;
		if (m_timedWaveTimer < 10.0f)
			(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[1]))->GetText()->SetColor(XMFLOAT4(1, 0, 0, 1));
		else if (m_timedWaveTimer < 20.0f)
			(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[1]))->GetText()->SetColor(XMFLOAT4(1, 1, 0, 1));
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[1]))->GetText()->SetText(to_string(static_cast<int>(m_timedWaveTimer)));
	}
	//Update the Time until a new enemy is place so they wont overlap
	if (!m_waveOver)
		m_inbetweenEnemies -= dTime;
	//Information update
	if (m_infoIsOn)
		m_infoOffTimer -= dTime;
}

void Survival::SpawnNewEnemy(int _id)
{
	//Makes sure new enemy is Valid
	_id %= 3;
	GameObject *enemy = nullptr;
	//Grabs enemy from factoy and then sets up variables it needs and lowers the cunt of that enemy
	ObjectFactory::GetInstance().Instantiate(m_enemiesName[_id], enemy, m_sCurrentScene);

	m_enemiesLeft[_id]--;
	enemy->Init();

	BaseEnemy *tempScript = dynamic_cast<BaseEnemy*>(enemy->GetScript("BaseEnemy"));
	tempScript->SetGoldToDrop(m_goldForWave);
	//Give Every Enemy a Coin
	tempScript->GiveCoin(true);
	//Give event to call when they die
	tempScript->SetEventName(m_eventName);
	int healthIncrease = 0;
	int damageIncrease = 0;
	//increase the health and damage of the new enemy based on waves
	switch (tempScript->GetEnemyType())
	{
	case GRUNT:
	{
		if (m_currentWave < 10)
		{
			healthIncrease = ((m_currentWave - 1) * 25);
			damageIncrease = ((m_currentWave - 1) * 3);
		}
		else
		{
			healthIncrease = static_cast<int>(200 * (1.1f * m_currentWave - 10));
			damageIncrease = static_cast<int>(50 * (1.1f * m_currentWave - 10));
		}
		break;
	}
	case ARCHER:
	{
		if (m_currentWave < 10)
		{
			healthIncrease = ((m_currentWave - 3) * 10);
			damageIncrease = ((m_currentWave - 3) * 2);
		}
		else
		{
			healthIncrease = static_cast<int>(100 * (1.1f * m_currentWave - 10));
			damageIncrease = static_cast<int>(5 * (1.1f * m_currentWave - 10));
		}
		break;
	}
	case GRUNTCOMMANDER:
	{
		if (m_currentWave < 10)
		{
			healthIncrease = ((m_currentWave - 8) * 60);
			damageIncrease = ((m_currentWave - 8) * 15);
		}
		else
		{
			healthIncrease = static_cast<int>(1000 * (1.1f * m_currentWave - 10));
			damageIncrease = static_cast<int>(40 * (1.1f * m_currentWave - 10));
		}
		break;
	}
	default:
		break;
	}
	tempScript->SetDamage(damageIncrease);
	tempScript->SetHealth(healthIncrease);
	if (m_isHPWave)
		tempScript->SetHealthDropRate(100);
	enemy->GetTransform().SetPosition(m_closestPos[m_spawnPosIndex]);
	m_spawnPosIndex++;
	m_spawnPosIndex %= CLOSEST_POINTS_SIZE;
	enemy->SetActive(true);
	m_aliveEnemies++;
}
//Function is called Once when the rest timer is done
void Survival::NewWave()
{
	m_waveOver = false;
	m_currentWave++;
	m_aliveEnemies = 0;
	AudioManager::GetInstance()->SetBGMVolume(m_bgmVolume);
	EventManager::GetInstance()->CallEvent("NewWave");
	//Display the Wave In roman numerals
	int amount[13] = { 1000, 900, 500, 400, 100,90, 50, 40, 10, 9, 5, 4, 1 };
	std::string romanNu[13] = { "M", "CM", "D", "CD", "C", "XC", "L", "XL", "X", "IX", "V", "IV", "I" };
	std::string text = "";
	int tempNumber = m_currentWave;
	(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[1]))->SetActive(false);
	for (size_t i = 0; i < 13; i++)
	{
		while (tempNumber >= amount[i])
		{
			tempNumber -= amount[i];
			text += romanNu[i];
		}
	}
	(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[0]))->GetText()->SetText("Wave: " + text);
	//Checks to see if the cureent wave is special
	if (m_currentWave == m_hpWave)
	{
		m_isHPWave = true;
		m_hpWave += 10;
	}
	if (m_timedWave == m_currentWave)
	{
		m_timedWaveTimer = TIMED_WAVE_INTIAL_TIME;
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[1]))->SetActive(true);
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[1]))->GetText()->SetText(to_string(m_timedWaveTimer));
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[1]))->GetText()->SetColor(XMFLOAT4(1, 1, 1, 1));
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[2]))->SetActive(true);
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[2]))->GetText()->SetPosition(XMFLOAT2(550, 475));
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[2]))->GetText()->SetText(std::string("Survive For: ") + to_string(m_timedWaveTimer) + std::string(" seconds!"));
		m_infoIsOn = true;
		m_isTimedWave = true;
		m_timedWave += 10;
	}
	CalculateWave();
	SpawnMoreEnemies();
}
//Called Once the Wave Is over to set everything to a default state
void Survival::WaveFinished()
{
	EventManager::GetInstance()->CallEvent("KillAllEnemies");
	m_timeBetweenWaves = TIME_BETWEEN_WAVES;
	m_isHPWave = false;
	m_isTimedWave = false;
	m_bgmVolume = AudioManager::GetInstance()->GetBGMVolume();
	if (m_bgmVolume > 0.05f)
	{
		AudioManager::GetInstance()->SetBGMVolume(0.05f);
		m_fadeInTimer = FADE_IN_TIME;
	}
	if (m_currentWave != 0)
	{
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[2]))->SetActive(true);
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[2]))->GetText()->SetColor(XMFLOAT4(0.5411764705882353, 0.0274509803921569, 0.0274509803921569, 1));
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[2]))->GetText()->SetPosition(XMFLOAT2(725, 475));
		(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[2]))->GetText()->SetText(std::string("Wave Completed"));
	}
	m_infoIsOn = true;
	(dynamic_cast<UIComponent*>(m_pParent->GetComponents(USERINTERFACE)[1]))->SetActive(true);
}
//Used to Calculate how many enemies of each type is in the wave
void Survival::CalculateWave()
{
	m_goldForWave = 8 * m_currentWave;
	if (!m_isTimedWave)
	{
		switch (m_currentWave)
		{
		case 1:
		{
			m_enemiesLeft[0] = 25;
			m_enemiesLeft[1] = 0;
			m_enemiesLeft[2] = 0;
			break;
		}
		case 2:
		{
			m_enemiesLeft[0] = 50;
			m_enemiesLeft[1] = 0;
			m_enemiesLeft[2] = 0;
			break;
		}
		case 3:
		{
			m_enemiesLeft[0] = 75;
			m_enemiesLeft[1] = 1;
			m_enemiesLeft[2] = 0;
			break;
		}
		case 4:
		{
			m_enemiesLeft[0] = 100;
			m_enemiesLeft[1] = 7;
			m_enemiesLeft[2] = 0;
			break;
		}
		case 5:
		{
			m_enemiesLeft[0] = 125;
			m_enemiesLeft[1] = 15;
			m_enemiesLeft[2] = 0;
			break;
		}
		case 6:
		{
			m_enemiesLeft[0] = 150;
			m_enemiesLeft[1] = 20;
			m_enemiesLeft[2] = 0;
			break;
		}
		case 7:
		{
			m_enemiesLeft[0] = 175;
			m_enemiesLeft[1] = 25;
			m_enemiesLeft[2] = 0;
			break;
		}
		case 8:
		{
			m_enemiesLeft[0] = 200;
			m_enemiesLeft[1] = 30;
			m_enemiesLeft[2] = 1;
			break;
		}
		case 9:
		{
			m_enemiesLeft[0] = 225;
			m_enemiesLeft[1] = 40;
			m_enemiesLeft[2] = 5;
			break;
		}
		case 10:
		{
			m_enemiesLeft[0] = 250;
			m_enemiesLeft[1] = 50;
			m_enemiesLeft[2] = 10;
			break;
		}
		default:
		{
			m_enemiesLeft[0] = m_currentWave * 30 + 8;
			m_enemiesLeft[1] = m_currentWave * 10 + 6;
			m_enemiesLeft[2] = m_currentWave * 2 + 2;
			if (m_enemiesLeft[2] > 25 && m_currentWave < 50)
				m_enemiesLeft[2] = 25;
			break;
		}
		}
	}
	else
	{
		m_enemiesLeft[0] = INT_MAX;
		m_enemiesLeft[1] = INT_MAX;
		m_enemiesLeft[2] = INT_MAX;
	}
}
//Adds more enemies to the scene based on how many are left and how many closest points there are
void Survival::SpawnMoreEnemies()
{
	ClosestPoints();
	for (size_t i = 0; i < CLOSEST_POINTS_SIZE; i++)
	{
		if ((m_enemiesLeft[0] <= 0 && m_enemiesLeft[1] <= 0 && m_enemiesLeft[2] <= 0) || m_aliveEnemies > MAX_ENEMIES_ON_SCREEN)
			break;
		int randEnemy = -1;
		do
		{
			randEnemy = (rand() % 10);
			if (randEnemy < 7)
				randEnemy = 0;
			else if (randEnemy < 9)
				randEnemy = 1;
			else if (randEnemy < 10)
				randEnemy = 2;
		} while (m_enemiesLeft[randEnemy] <= 0);
		SpawnNewEnemy(randEnemy);
	}
}
//Finds the Closest to the player
void Survival::ClosestPoints()
{
	for (size_t i = 0; i < m_spawnPositions.size(); i++)
	{
		IsCloserToPlayer(m_spawnPositions[i]);
	}
}
//Does the math if a position in the closePosition is closer or not to given position
bool Survival::IsCloserToPlayer(XMFLOAT3 _testPos)
{
	if (!NotInList(_testPos))
		return false;
	XMFLOAT3 playerPos = m_sCurrentScene->GetObjectByTag("Player")->GetTransform().GetPosition();
	for (int i = 0; i < CLOSEST_POINTS_SIZE; i++)
	{
		XMFLOAT3 currPoint = m_closestPos[i];
		XMFLOAT3 lineBetweenTestAndPlayer, lineBetweenCurrPointAndPlayer;
		lineBetweenTestAndPlayer = VectorSubtract(_testPos, playerPos);
		lineBetweenCurrPointAndPlayer = VectorSubtract(currPoint, playerPos);
		float sqDis1, sqDis2;
		sqDis1 = VectorDot(lineBetweenCurrPointAndPlayer, lineBetweenCurrPointAndPlayer);
		sqDis2 = VectorDot(lineBetweenTestAndPlayer, lineBetweenTestAndPlayer);
		if (abs(sqDis1) > abs(sqDis2))
		{
			m_closestPos[i] = _testPos;
			return true;
		}
	}
	return false;
}
//Check to see if its the same vector
bool Survival::VectorNotEqual(XMFLOAT3 _lhs, XMFLOAT3 _rhs)
{
	return ((_lhs.x != _rhs.x) || (_lhs.z != _rhs.z));
}
//Checks to see if the given Pos already in the closest point list
bool Survival::NotInList(XMFLOAT3 _pos)
{
	for (size_t i = 0; i < CLOSEST_POINTS_SIZE; i++)
	{
		if (!VectorNotEqual(_pos, m_closestPos[i]))
			return false;
	}
	return true;
}
//Vector Subtract lhs - rhs
XMFLOAT3 Survival::VectorSubtract(XMFLOAT3 _lhs, XMFLOAT3 _rhs)
{
	XMFLOAT3 returnValue;
	XMStoreFloat3(&returnValue, XMVectorSubtract(XMLoadFloat3(&_lhs), XMLoadFloat3(&_rhs)));
	return returnValue;
}
//Vector Dot
float Survival::VectorDot(XMFLOAT3 _lhs, XMFLOAT3 _rhs)
{
	XMFLOAT3 returnValue;
	XMStoreFloat3(&returnValue, XMVector3Dot(XMLoadFloat3(&_lhs), XMLoadFloat3(&_rhs)));
	return returnValue.x;
}
