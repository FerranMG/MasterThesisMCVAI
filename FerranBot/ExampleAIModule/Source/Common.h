#pragma once

#include <BWAPI.h>
#include "assert.h"
#include "Defines.h"


typedef enum x{ATTACK				= 0, //Direct attack
			   HOLD					= 1, 
			   FLEE					= 2,
			   ATTACK_SURROUND		= 3,
			   COUNT				= 4} Action;


typedef enum y{NA		= -1, //Not Available
			   LOW		= 0,
			   MID		= 1,
			   HIGH		= 2} Group;

struct State
{
	State();

	//AvgDpsXHealth and avgHealth allow to easily get how dangerous an enemy group is. 
	Group  m_avgDpsXHealthGroup; //sum(dps X health) / numUnits. //TODO - 3 or 4 groups
	Group  m_avgHealthGroup; //LOW, MID, HIGH
	Group  m_distToClosestEnemyGroup; //CLOSE, MID-RANGE, FAR
	int    m_hitPoints;
	int    m_enemyHitPoints;
	Action m_action; //Last taken action

	//Transform int to Group
	void setAvgDpsXHealth(Group num);
	void setAvgHealth(Group health);
	void setSqDistToClosestEnemyGroup(Group dist);
} ;


struct UnitState
{
	UnitState();

	//AvgDpsXHealth and avgHealth allow to easily get how dangerous an enemy group is. 
	Group  m_avgDpsXHealthGroup; //sum(dps X health) / numUnits.
	Group  m_avgHealthGroup; //LOW, MID, HIGH
	Group  m_distToClosestEnemyGroup; //CLOSE, MID-RANGE, FAR
	int    m_hitPoints;
	int    m_enemyHitPoints;
	Action m_unitAction;
	Action m_squadAction;

	//Transform int to Group
	void setAvgDpsXHealth(Group num);
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

