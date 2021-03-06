#pragma once
#include <BWAPI.h>
#include "SquadEntity.h"
#include "Defines.h"


class SquadManager
{
public:
	static SquadManager* getInstance();
	~SquadManager();

	void update();

	void addSquad(SquadEntity* squadEntity);
	void addEnemySquad(SquadEntity* squadEntity);
	std::vector<SquadEntity*>* getSquads() const;
	std::vector<SquadEntity*>*  getEnemySquads() const;

	void removeAllSquads();
	
	//public because have to be deleted at ::onEnd
	std::vector<SquadEntity*>* m_squads;
	std::vector<SquadEntity*>* m_enemySquads;
	
	BWAPI::Unitset* m_differentEnemyUnits;
	BWAPI::Unitset m_differentFriendlyUnits;

private:
	static SquadManager* instance;
	SquadManager();
};