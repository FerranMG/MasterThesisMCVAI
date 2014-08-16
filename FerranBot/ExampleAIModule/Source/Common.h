#pragma once

#include <BWAPI.h>
#include "assert.h"
#include "Defines.h"


typedef enum x{ATTACK				= 0, //Direct attack
			   HOLD					= 1, 
			   ATTACK_SURROUND		= 2,
			   ATTACK_HALF_SURROUND = 3,
			   COUNT				= 4} Action;

typedef enum y{SQUAD_ACTION				= 10,
			   UNIT_ATTACK				= 11,
			   UNIT_HOLD				= 12,
			   UNIT_FLEE				= 13,
			   UNIT_COUNT				= 14} UnitAction;;


typedef enum z{NA		= -1, //Not Available
			   LOW		= 0,
			   MID		= 1,
			   HIGH		= 2} Group;

struct State
{
	State();

	//AvgDps and avgHealth allow to easily get how dangerous an enemy group is. 
	Group  m_avgDpsGroup; //sum(dps) / numUnits.
	Group  m_avgHealthGroup; //LOW, MID, HIGH
	Group  m_distToClosestEnemyGroup; //CLOSE, MID-RANGE, FAR
	int    m_hitPoints;
	int    m_enemyHitPoints;
	Action m_action; //Last taken action

	//Transform int to Group
	void setAvgDps(Group num);
	void setAvgHealth(Group health);
	void setSqDistToClosestEnemyGroup(Group dist);
} ;


struct UnitState
{
	UnitState();

	//AvgDps and avgHealth allow to easily get how dangerous an enemy group is. 
	Group  m_avgDpsGroup; //sum(dps) / numUnits.
	Group  m_avgHealthGroup; //LOW, MID, HIGH
	Group  m_distToClosestEnemyGroup; //CLOSE, MID-RANGE, FAR
	int    m_hitPoints;
	int    m_enemyHitPoints;
	UnitAction m_lastUnitAction;
	Action m_lastSquadAction;

	//Transform int to Group
	void setAvgDps(Group num);
	void setAvgHealth(Group health);
	void setSqDistToClosestEnemyGroup(Group dist);

};

class Common
{
public:
	static int computeSqDistBetweenPoints(BWAPI::Position pos1, BWAPI::Position pos2);

	static float getUnitDps(BWAPI::Unitset::iterator unit);
	static float getUnitDps(BWAPI::Unit unit);
};

