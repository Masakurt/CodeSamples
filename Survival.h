#pragma once
#include "../../Script.h"
#include <vector>
#include <string>
#define CLOSEST_POINTS_SIZE 3
class Survival : public Script
{
public:
	void Initialize();
	void Update();
	void Shutdown();
	Component* Clone();
	void OnEvent(std::string _eventName);
	void AddSpawnPosition(XMFLOAT3 _pos);
private:
	unsigned int m_currentWave, m_hpWave, m_timedWave;
	std::vector<XMFLOAT3> m_spawnPositions;
	XMFLOAT3 m_closestPos[CLOSEST_POINTS_SIZE];
	int m_enemiesLeft[3], m_goldForWave, m_spawnPosIndex, m_aliveEnemies;
	std::string m_enemiesName[3], m_eventName;
	float m_timeBetweenWaves, m_timedWaveTimer, m_inbetweenEnemies, m_infoOffTimer, m_bgmVolume, m_fadeInTimer;
	bool m_isHPWave, m_isTimedWave, m_waveOver, m_infoIsOn;
	//Functions
	void UpdateTimers();
	void SpawnNewEnemy(int _id = 0);
	void NewWave();
	void WaveFinished();
	void CalculateWave();
	void SpawnMoreEnemies();
	void ClosestPoints();
	bool IsCloserToPlayer(XMFLOAT3 _testPos);
	bool VectorNotEqual(XMFLOAT3 _lhs, XMFLOAT3 _rhs);
	bool NotInList(XMFLOAT3 _pos);
	XMFLOAT3 VectorSubtract(XMFLOAT3 _lhs, XMFLOAT3 _rhs);
	float VectorDot(XMFLOAT3 _lhs, XMFLOAT3 _rhs);
};