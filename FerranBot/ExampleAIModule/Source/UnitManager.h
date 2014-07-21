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

private:
	static UnitManager* instance;
	UnitManager();
	void removeDeadUnits();
	//bool checkNoHitPoints(const UnitEntity* unitEntity);
	std::vector<UnitEntity*> m_allUnitEntities;

};