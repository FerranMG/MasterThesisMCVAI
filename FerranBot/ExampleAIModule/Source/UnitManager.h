#pragma once
#include <BWAPI.h>
#include "Common.h"
#include "Defines.h"
#include <vector>

class UnitEntity;
class SquadEntity;

class UnitManager
{
public:
	static UnitManager* getInstance();
	~UnitManager();

	UnitEntity* createUnit(BWAPI::Unitset::iterator u);
	std::vector<UnitEntity*> getUnitEntitiesBySquad(SquadEntity* squadEntity);

	void update();

	void removeAllUnitEntities();

	std::vector<UnitEntity*> getAllUnitEntities() const;

	void calculateNumEnemyUnitsKilledByFriendly();

	static int s_friendlyUnitsAlive;
	static int s_enemyUnitsAlive;
	static int s_killedEnemyUnitsOld;
	static int s_killedEnemyUnits;


	static int s_killedEnemyUnitsByFriendly;

private:
	static UnitManager* instance;
	UnitManager();
	void removeDeadUnits();
	//bool checkNoHitPoints(const UnitEntity* unitEntity);
	std::vector<UnitEntity*> m_allUnitEntities;

	int getNumEnemyUnits();
	int getNumFriendlyUnits();
	int getNumEnemyUnitsKilled();
};