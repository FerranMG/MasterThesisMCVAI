#include "UnitEntity.h"
#include "SquadEntity.h"
#include "SquadManager.h"

using namespace BWAPI;
using namespace std;

UnitEntity::UnitEntity(BWAPI::Unitset::iterator unit)
{
	//TODO - SEE WHAT OTHER INFO I NEED TO GET FROM THE u IN THE CONSTRUCTOR
	m_unitIterator = unit;
	m_unit = *unit;


	m_lastActionTilePos = Position(-1, -1);

	m_lastUnitState = UnitState();

	m_lastUnitAction = COUNT;
	m_lastSquadAction = COUNT;

	m_canIssueNextAction = true;
	m_frameCount = 0;
}


UnitEntity::~UnitEntity()
{

}


void UnitEntity::update()
{
	SquadEntity* currentSquad = getSquad();
	assert(currentSquad);

	if(!currentSquad->getIsEnemySquad())
	{
		checkDirectEnemy();

		setSquadAction(currentSquad->getCurrentAction());

		++m_frameCount;

		Action unitAction = calculateUnitAction();
		assert(unitAction != COUNT);
		setUnitAction(unitAction);

	}
}


void UnitEntity::assignToSquad(SquadEntity* squadEntity)
{
	m_squad = squadEntity;
	setIsEnemyUnit(m_squad->getIsEnemySquad());
}


SquadEntity* UnitEntity::getSquad() const
{
	return m_squad;
}


void UnitEntity::setSquadAction(Action action)
{
	m_squadAction = action;
}


void UnitEntity::setUnitAction(Action action)
{
	m_unitAction = action;
}


BWAPI::Unitset::iterator UnitEntity::getUnitIterator() const
{
	return m_unitIterator;
}


Action UnitEntity::calculateUnitAction()
{
	//TODO - eg, if there's an enemy unit attacking this unit and it will die, then flee, regarding of what squad action says
	//POSSIBLY USE SOME SORT OF LEARNING. TAKE THE ACTION FROM A Q-MAP
	
	return m_squadAction;
}


void UnitEntity::applyCurrentUnitAction()
{
	performBasicChecks();

	switch(m_unitAction)
	{
	case ATTACK:
		{
			attackClosestEnemyUnit();
			//{
			//	//TODO
			//int mapWidth = Broodwar->mapWidth() * TILE_SIZE;
			//int mapHeight = Broodwar->mapHeight() * TILE_SIZE;
			//	//if(QLearningMgr::getInstance()->getCurrentAction() == COUNT)
			//	{
			//		if(x < mapWidth >> 1 && y < mapHeight >> 1) //TOP LEFT
			//		{
			//			u->move(Position(x + 10, y));
			//		}
			//		else if(x > mapWidth >> 1 && y < mapHeight >> 1) //TOP RIGHT
			//		{
			//			u->move(Position(x, y + 10));
			//		}
			//		else if(x > mapWidth>> 1 && y > mapHeight >> 1) //BOTTOM RIGHT
			//		{
			//			u->move(Position(x - 10, y));
			//		}
			//		else //BOTTOM LEFT
			//		{
			//			u->move(Position(x, y - 10));
			//		}
			//	}
			//}

		}
		break;

	case HOLD:
		{
			holdPosition();

		}
		break;

		//case FLEE:
		//{
		//	//TODO - set a proper fleeing point
		//	Position fleeingPoint = Position(0, 0);
		//	if(squad->m_lastActionTilePos == Position(-1, -1) || squad->m_lastActionTilePos != fleeingPoint)
		//	{
		//		u->move(0, 0);
		//		squad->m_lastActionTilePos = fleeingPoint;
		//	}
		//}
		//break;

	case ATTACK_SURROUND:
		{
			attackSurround();
		}
		break;

	default:
		{

		}
		break;
	}
}

void UnitEntity::attackClosestEnemyUnit()
{
	Unit unitToAttack;
	int maxDist = std::numeric_limits<int>::max();
	bool foundUnit = false;
	for(vector<SquadEntity*>::iterator enemySquad = SquadManager::getInstance()->getEnemySquads()->begin(); enemySquad != SquadManager::getInstance()->getEnemySquads()->end(); enemySquad++)
	{
		Unitset enemyUnits = (*enemySquad)->getSquadUnits();
		for(Unitset::iterator eUnit = enemyUnits.begin(); eUnit != enemyUnits.end(); eUnit++)
		{
			int dist = Common::computeSqDistBetweenPoints(m_unitIterator->getPosition(), eUnit->getPosition());
			if(dist < maxDist)
			{
				maxDist = dist;
				unitToAttack = *eUnit;
				foundUnit = true;
			}
		}
	}

	if(foundUnit)
	{
		m_unitIterator->attack(unitToAttack);
	}

}

void UnitEntity::holdPosition()
{
	int tileX = getUnit()->getPosition().x / TILE_SIZE;
	int tileY = getUnit()->getPosition().y / TILE_SIZE;


	if(m_lastActionTilePos == Position(-1, -1) || m_lastActionTilePos != Position(tileX, tileY))
	{
		getUnit()->holdPosition();
		m_lastActionTilePos = Position(tileX, tileY);
	}
}

void UnitEntity::attackSurround()
{
	//TODO
}

bool UnitEntity::performBasicChecks() const
{
	Unitset::iterator u = getUnitIterator();
	// Ignore the unit if it no longer exists
	// Make sure to include this block when handling any Unit pointer!
	if ( !u->exists() )
	{
		return false;
	}

	// Ignore the unit if it has one of the following status ailments
	if ( u->isLockedDown() || u->isMaelstrommed() || u->isStasised() )
	{
		return false;
	}

	// Ignore the unit if it is in one of the following states
	if ( u->isLoaded() || !u->isPowered() || u->isStuck() )
	{
		return false;
	}

	// Ignore the unit if it is incomplete or busy constructing
	if ( !u->isCompleted() || u->isConstructing() )
	{
		return false;
	}

	return true;
}

bool UnitEntity::getIsEnemyUnit() const
{
	return m_isEnemyUnit;
}

void UnitEntity::setIsEnemyUnit(bool yes)
{
	m_isEnemyUnit = yes;
}

//Check if, given the last action that was issued, a new one can be processed
//This is in order not to send too many actions in a short interval of time.
//Sort of like let the actions already issued have a consequence before sending a new one.
bool UnitEntity::checkCanIssueNextAction()
{
	if(m_unitAction == ATTACK)
	{
		//Remove info from old FLEE or HOLD actions
		m_frameCount = 0;
		m_lastActionTilePos = Position(-1, -1);

		SquadEntity* enemySquad = m_squad->getEnemySquad();
			
		if(enemySquad->getNumUnits() <= 0)
		{
			//No enemy units on sight, can send new action
			m_unitAction = COUNT;
			m_canIssueNextAction = true;
		}
		else
		{
			//When some unit has started to attack, then we can send a new action
			bool isUnitAttacking = m_unit->isAttacking();
			//for(Unitset::iterator u = m_squad->getSquadUnits().begin(); u != m_squad->getSquadUnits().end(); ++u)
			//{
				//if(u->isAttacking())
				//{
				//	isAnyUnitAttacking = true;
				//	break;
				//}
			//}

			if(isUnitAttacking)
			{
				m_unitAction = COUNT;
				m_canIssueNextAction = true;
			}
		}
	}
	else if(m_unitAction == ATTACK_SURROUND)
	{
		//SquadEntity* enemySquad = m_squad->getEnemySquad();
		//Position enemyPos = enemySquad->getAvgPosition();

		//if(Common::computeSqDistBetweenPoints(m_posToSurround, enemyPos) > 50 * 50)
		//{
		//	m_unitAction = COUNT;
		//	m_canIssueNextAction = true;
		//}
		//else
		//{
		//	m_canIssueNextAction = false;
		//}
	}
	else if(m_unitAction == HOLD || m_unitAction == FLEE)
	{
		//Remove info from old ATTACK action
		m_lastUnitAttacked = NULL;

		if(m_frameCount > 5)
		{
			m_unitAction = COUNT;
			m_canIssueNextAction = true;
			m_frameCount = 0;
		}
	}
	else
	{
		m_canIssueNextAction = true;
	}

	return m_canIssueNextAction;
}


UnitState UnitEntity::getCurrentUnitState() const
{
	UnitState state;

	assert(m_directEnemy);
	setHealthInUnitState(state);
	setDpsXHealthInUnitState(state);
	setDistanceInUnitState(state);
	setHitPointsInUnitState(state);
	setEnemyHitPointsInUnitState(state);
	setLastUnitActionInUnitState(state);
	setLastSquadActionInUnitState(state);

	return state;
}


UnitState UnitEntity::getLastUnitState() const
{
	return m_lastUnitState;
}


Action UnitEntity::getLastUnitAction() const
{
	return m_lastUnitAction;
}

Action UnitEntity::getLastSquadAction() const
{
	return m_lastSquadAction;
}


void UnitEntity::checkDirectEnemy()
{
	//Find closest enemy
	int index = 0;
	int unitIndexToAttack = 0;
	int minDist = std::numeric_limits<int>::max();

	SquadEntity* enemySquad = m_squad->getEnemySquad();
	Unitset enenmySquadUnitset = enemySquad->getSquadUnits();
	
	for(Unitset::iterator unit = enenmySquadUnitset.begin(); unit != enenmySquadUnitset.end(); unit++)
	{
		Position enemyPos = unit->getPosition();
		int dist = enemyPos.getApproxDistance(m_unit->getPosition());
		if(dist < minDist)
		{
			unitIndexToAttack = index;
			minDist = dist;
		}
	}

	m_directEnemy = enemySquad->getSquadUnits()[unitIndexToAttack];
}

void UnitEntity::setHealthInUnitState(UnitState& state) const
{
	//TODO - revise this
	if(m_unit->getHitPoints() < m_directEnemy->getHitPoints() * 0.8)
	{
		state.setAvgHealth(LOW);
	}
	else if(m_unit->getHitPoints() >= m_directEnemy->getHitPoints() * 0.8
		&&  m_unit->getHitPoints() < m_directEnemy->getHitPoints() * 1.2)
	{
		state.setAvgHealth(MID);
	}
	else
	{
		state.setAvgHealth(HIGH);
	}
}

void UnitEntity::setDpsXHealthInUnitState(UnitState& state) const
{
	if(Common::getUnitDps(m_unit) < Common::getUnitDps(m_directEnemy) * 0.8)
	{
		state.setAvgDpsXHealth(LOW);
	}
	else if(Common::getUnitDps(m_unit) >= Common::getUnitDps(m_directEnemy) * 0.8
		&&  Common::getUnitDps(m_unit) < Common::getUnitDps(m_directEnemy) * 1.2)
	{
		state.setAvgDpsXHealth(MID);
	}
	else
	{
		state.setAvgDpsXHealth(HIGH);
	}
}

void UnitEntity::setDistanceInUnitState(UnitState& state) const
{
	int dist = Common::computeSqDistBetweenPoints(m_unit->getPosition(), m_directEnemy->getPosition());

	if(dist < 10000) //TODO - REVISE THIS
	{
		state.setSqDistToClosestEnemyGroup(LOW);
	}
	else if(dist < 1000000)
	{
		state.setSqDistToClosestEnemyGroup(MID);
	}
	else
	{
		state.setSqDistToClosestEnemyGroup(HIGH);
	}
}

void UnitEntity::setHitPointsInUnitState(UnitState& state) const
{
	state.m_hitPoints = m_unit->getHitPoints();
}

void UnitEntity::setEnemyHitPointsInUnitState(UnitState& state) const
{
	state.m_enemyHitPoints = m_directEnemy->getHitPoints();
}

void UnitEntity::setLastUnitActionInUnitState(UnitState& state) const
{
	state.m_unitAction = m_unitAction;
}

void UnitEntity::setLastSquadActionInUnitState(UnitState& state) const
{
	state.m_squadAction = m_squadAction;
}

bool UnitEntity::canIssueNextAction() const
{
	return m_canIssueNextAction;
}

Action UnitEntity::getCurrentUnitAction() const
{
	return m_unitAction;
}

Action UnitEntity::getCurrentSquadAction() const
{
	return m_squadAction;
}

void UnitEntity::setLastUnitAction(Action action)
{
	m_lastUnitAction = action;
}

void UnitEntity::setLastSquadAction(Action action)
{
	m_lastSquadAction = action;
}

void UnitEntity::setCanIssueNextAction(bool yes)
{
	m_canIssueNextAction = yes;
}
