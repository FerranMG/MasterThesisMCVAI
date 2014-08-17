#include "UnitEntity.h"
#include "SquadEntity.h"
#include "SquadManager.h"

using namespace BWAPI;
using namespace std;

UnitEntity::UnitEntity(BWAPI::Unitset::iterator unit)
{
	m_unitIterator = unit;
	m_unit = *unit;

	m_directEnemy = nullptr;
	m_squad = nullptr;
	m_squadAction = COUNT;
	m_unitAction = UNIT_COUNT;

	m_currentActionTilePos = Position(-1, -1);
	m_lastActionTilePos = Position(-1, -1);
	m_currentUnitAttacked = nullptr;
	m_lastUnitAttacked = nullptr;

	m_isEnemyUnit = false;

	m_lastUnitState = UnitState();

	m_lastUnitAction = UNIT_COUNT;
	m_lastSquadAction = COUNT;

	m_canIssueNextAction = true;
	m_frameCount = 0;

	m_hasUnitStartedAttack = false;
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
		++m_frameCount;

		if(canIssueNextAction())
		{
			checkDirectEnemy();

			Action squadAction = currentSquad->getCurrentAction();
			setSquadAction(squadAction);

			Action lastSquadAction = currentSquad->getLastAction();
			setLastSquadAction(lastSquadAction);
		}
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


void UnitEntity::setUnitAction(UnitAction action)
{
	m_unitAction = action;
}


BWAPI::Unitset::iterator UnitEntity::getUnitIterator() const
{
	return m_unitIterator;
}


void UnitEntity::applyCurrentUnitAction()
{
	performBasicChecks();

	switch(m_unitAction)
	{
		case SQUAD_ACTION:
		{
			applySquadActionToUnit();
		}
		break;

		case UNIT_ATTACK:
		{
			attackClosestEnemyUnit();
		}
		break;

		case UNIT_HOLD:
		{
			holdPosition();

		}
		break;

		case UNIT_FLEE:
		{
			Position fleeingPoint = Position(0, 0);
			if(m_directEnemy)
			{
				Position enemyPos = m_directEnemy->getPosition();
				Position currentPos = getUnit()->getPosition();
				int xDiff = std::abs(enemyPos.x - currentPos.x);
				int yDiff = std::abs(enemyPos.y - currentPos.y);

				if(xDiff < yDiff) //closer in the x axis than in the y axis => get away in the x axis
				{
					if(currentPos.x < enemyPos.x)
					{
						fleeingPoint.x = 0;
						fleeingPoint.y = currentPos.y;
					}
					else
					{
						fleeingPoint.x = Broodwar->mapWidth();
						fleeingPoint.y = currentPos.y;
					}
				}
				else
				{
					if(currentPos.y < enemyPos.y)
					{
						fleeingPoint.x = currentPos.x;
						fleeingPoint.y = 0;
					}
					else
					{
						fleeingPoint.x = currentPos.x;
						fleeingPoint.y = Broodwar->mapHeight();
					}
				}
			}

			if(m_currentActionTilePos == Position(-1, -1) || m_currentActionTilePos != fleeingPoint)
			{
				getUnit()->move(fleeingPoint);
				m_currentActionTilePos = fleeingPoint;
				m_frameCount = 0;
			}
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
	else if(Common::computeSqDistBetweenPoints(m_currentActionTilePos, getUnit()->getPosition()) > 10000)
	{
		getUnit()->move(m_currentActionTilePos);
	}
}

void UnitEntity::holdPosition()
{
	int tileX = getUnit()->getPosition().x / TILE_SIZE;
	int tileY = getUnit()->getPosition().y / TILE_SIZE;


	if(m_currentActionTilePos == Position(-1, -1) || m_currentActionTilePos != Position(tileX, tileY))
	{
		getUnit()->holdPosition();
		m_currentActionTilePos = Position(tileX, tileY);
	}

	if(m_directEnemy)
	{
		if(Common::computeSqDistBetweenPoints(m_directEnemy->getPosition(), getUnit()->getPosition()) > 10000)
		{
			getUnit()->move(m_directEnemy->getPosition());
		}
	}
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
	if(getSquad()->getEnemySquad()->getNumUnits() <= 0)
	{
		return false;
	}

	if(m_unitAction == UNIT_ATTACK
		|| (m_unitAction == SQUAD_ACTION && m_squadAction == ATTACK))
	{
		//Remove info from old FLEE or HOLD actions
		if(m_frameCount <= 5)
		{
			m_canIssueNextAction = false;
			return false;
		}

		m_frameCount = 0;
		//m_lastActionTilePos = Position(-1, -1);

		SquadEntity* enemySquad = m_squad->getEnemySquad();
			
		if(enemySquad->getNumUnits() <= 0)
		{
			//No enemy units on sight, can send new action
			m_canIssueNextAction = true;
		}
		else
		{

			//When the unit has finished an attack animation, then we can send a new action
			//bool isUnitAttacking = m_unit->isAttacking();
			bool isAttackFrame = m_unit->isAttackFrame();

			if(isAttackFrame)
			{
				m_hasUnitStartedAttack = true;
			}

			if(!isAttackFrame && m_hasUnitStartedAttack) //The unit has finished an attack animation
			{
				m_canIssueNextAction = true;
				m_hasUnitStartedAttack = false;
				m_frameCount = 0;
			}
		}
	}
	else if(m_unitAction == SQUAD_ACTION && (m_squadAction == ATTACK_SURROUND || m_squadAction == ATTACK_HALF_SURROUND))
	{
		if(m_frameCount <= 5)
		{
			m_canIssueNextAction = false;
			return false;
		}

		m_frameCount = 0;
		//Position posToMove = getSquad()->getActionTilePosForSurround(this);
		Position posToMove = m_currentActionTilePos;
		//const bool reachedSurroundPos = posToMove == m_lastActionTilePos;
		const bool reachedSurroundPos = posToMove == getUnit()->getPosition();
		const bool isUnitAttacking = m_unit->isAttacking();
		const bool isAttackFrame = m_unit->isAttackFrame();

		if(reachedSurroundPos && (isUnitAttacking || isAttackFrame))
		{
			m_canIssueNextAction = true;
		}
	}
	else if(m_unitAction == UNIT_HOLD || m_unitAction == UNIT_FLEE
		|| (m_unitAction == SQUAD_ACTION && m_squadAction == HOLD))
	{
		//Remove info from old ATTACK action
		m_lastUnitAttacked = NULL;

		if(m_unitAction == UNIT_HOLD && m_frameCount > 5)
		{
			m_canIssueNextAction = true;
			m_frameCount = 0;
		}
		else if(m_unitAction == UNIT_FLEE && m_frameCount > 10)
		{
			m_canIssueNextAction = true;
			m_frameCount = 0;
		}
		else if(m_frameCount > 5)
		{
			m_canIssueNextAction = true;
			m_frameCount = 0;
		}
	}
	else
	{
		m_canIssueNextAction = true;
	}

	if(m_canIssueNextAction)
	{
		m_lastActionTilePos = m_currentActionTilePos;
	}

	return m_canIssueNextAction;
}


UnitState UnitEntity::getCurrentUnitState() const
{
	UnitState state;

	//assert(m_directEnemy);
	setHealthInUnitState(state);
	setDpsInUnitState(state);
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


UnitAction UnitEntity::getLastUnitAction() const
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
	int unitIndexToAttack = -1;
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

	if(unitIndexToAttack >= 0 && enemySquad->getNumUnits() > unitIndexToAttack)
	{
		m_directEnemy = enemySquad->getSquadUnits()[unitIndexToAttack];
		m_currentActionTilePos = enemySquad->getSquadUnits()[unitIndexToAttack]->getPosition();
	}
	else
	{
		m_directEnemy = nullptr;
		m_currentActionTilePos = Position(-1, -1);
	}
}

void UnitEntity::setHealthInUnitState(UnitState& state) const
{
	if(!m_directEnemy)
	{
		state.setAvgHealth(NA);
		return;
	}


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

void UnitEntity::setDpsInUnitState(UnitState& state) const
{
	if(!m_directEnemy)
	{
		state.setAvgDps(NA);
		return;
	}

	if(Common::getUnitDps(m_unit) < Common::getUnitDps(m_directEnemy) * 0.8)
	{
		state.setAvgDps(LOW);
	}
	else if(Common::getUnitDps(m_unit) >= Common::getUnitDps(m_directEnemy) * 0.8
		&&  Common::getUnitDps(m_unit) < Common::getUnitDps(m_directEnemy) * 1.2)
	{
		state.setAvgDps(MID);
	}
	else
	{
		state.setAvgDps(HIGH);
	}
}

void UnitEntity::setDistanceInUnitState(UnitState& state) const
{
	if(!m_directEnemy)
	{
		state.setSqDistToClosestEnemyGroup(NA);
		return;
	}

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
	if(!m_directEnemy)
	{
		state.m_enemyHitPoints = 0;
		return;
	}

	state.m_enemyHitPoints = m_directEnemy->getHitPoints();
}

void UnitEntity::setLastUnitActionInUnitState(UnitState& state) const
{
	state.m_lastUnitAction = m_lastUnitAction;
}

void UnitEntity::setLastSquadActionInUnitState(UnitState& state) const
{
	state.m_lastSquadAction = m_lastSquadAction;
}

bool UnitEntity::canIssueNextAction() const
{
	return m_canIssueNextAction;
}

UnitAction UnitEntity::getCurrentUnitAction() const
{
	return m_unitAction;
}

Action UnitEntity::getCurrentSquadAction() const
{
	return m_squadAction;
}

void UnitEntity::setLastUnitAction(UnitAction action)
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

void UnitEntity::setLastUnitState(UnitState stateNew)
{
	m_lastUnitState = stateNew;
}

void UnitEntity::setCurrentUnitToAttack(BWAPI::Unit enemyUnit)
{
	m_currentUnitAttacked = enemyUnit;
}

void UnitEntity::applySquadActionToUnit()
{
	switch(m_squadAction)
	{
		case ATTACK:
		{
			Unit unitToAttack = getSquad()->getUnitToAttack(this);
			if(unitToAttack != m_lastUnitAttacked)
			{
				getUnit()->attack(unitToAttack);
				m_lastUnitAttacked = unitToAttack;
			}
			else if(Common::computeSqDistBetweenPoints(m_currentActionTilePos, getUnit()->getPosition()) > 10000)
			{
				getUnit()->move(m_currentActionTilePos);
			}
		}
		break;

		case ATTACK_SURROUND:
		case ATTACK_HALF_SURROUND:
		{
			if(getSquad()->hasNewSurroundPositions())
			{
				m_currentActionTilePos = Position(-1, -1);
			}
			Position posToMove = m_currentActionTilePos;

			if(posToMove != Position(-1, -1)) //has a pos assigned
			{
				static bool attackIssuedForFirstTime = false;
				if(posToMove != getUnit()->getPosition()) //current position is not the desired one
				{
						attackIssuedForFirstTime = false;
						getUnit()->move(posToMove);
				}
				else //pos is the same
				{
					if(!attackIssuedForFirstTime) //if it has arrived to the position, but hasn't started attacking, attack
					{
						getUnit()->attack(posToMove);
						attackIssuedForFirstTime = true;
					}
				}
			}
			else //doesn't have a pos assigned
			{
				posToMove = getSquad()->calculateActionTilePosForSurround(this);

				m_currentActionTilePos = posToMove;
				getUnit()->move(posToMove);
			}
		}
		break;

		case HOLD:
		{
			getUnit()->holdPosition();
		}
		break;
	}
}

bool UnitEntity::mustUpdateCurrentAction()
{
	if(m_squadAction == ATTACK_SURROUND || m_squadAction == ATTACK_HALF_SURROUND)
	{
		//Position posToMove = getSquad()->getActionTilePosForSurround(this);

		if(getSquad()->hasNewSurroundPositions())
		{
			return true;
		}
		else if(m_currentActionTilePos == getUnit()->getPosition())
		{
			return true;
		}
	}

	return false;
}

void UnitEntity::resetCurrentActionTilePos()
{
	 m_currentActionTilePos = Position(-1, -1);
}
