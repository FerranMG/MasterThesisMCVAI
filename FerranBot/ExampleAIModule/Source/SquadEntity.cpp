#include "SquadEntity.h"
#include "SquadManager.h"
#include "Common.h"
#include <math.h>
#include "UnitManager.h"
#include "UnitEntity.h"

using namespace BWAPI;
using namespace std;

SquadEntity::SquadEntity()
{
	m_lastAction = COUNT;
	m_currentAction = COUNT;
	m_canIssueNextAction = true;
	m_lastState = State();

	m_frameCount = 0;
}


SquadEntity::~SquadEntity()
{
}


void SquadEntity::addUnit(Unitset::iterator u)
{
	UnitEntity* unitEntity = UnitManager::getInstance()->createUnit(u);
	unitEntity->assignToSquad(this);


	m_squadUnits.push_back(u);
	calculateNumUnits();
}


void SquadEntity::addUnits(const BWAPI::Unitset &units)
{
	for(BWAPI::Unitset::iterator it = units.begin(); it != units.end(); it++)
	{
		addUnit(it);
	}
}


BWAPI::Unitset SquadEntity::getSquadUnits() const
{
	return m_squadUnits;
}


void SquadEntity::update()
{
	++m_frameCount;
	removeDeadUnits();
	calculateAvgHealth();
	calculateAvgDps();
	calculateAvgPosition();
	calculateSqDistToClosestEnemyGroup();
	assignCurrentEnemySquad();

	//if(!m_isEnemySquad)
	//{
	//	Unitset squadUnitset = getSquadUnits();
	//	for(Unitset::iterator test0 = squadUnitset.begin(); test0 != squadUnitset.end(); test0++)
	//	{
	//		Position pos = test0->getPosition();
	//	}


	//	for(Unitset::iterator test1 = getSquadUnits().begin(); test1 != getSquadUnits().end(); test1++)
	//	{
	//		Position pos = test1->getPosition();
	//	}
	//	
	//	for(Unitset::iterator test2 = m_squadUnits.begin(); test2 != m_squadUnits.end(); test2++)
	//	{
	//		Position pos = test2->getPosition();
	//	}


	//	for(Unitset::iterator unit = getEnemySquad()->getSquadUnits().begin(); unit != getEnemySquad()->getSquadUnits().end(); unit++)
	//	{
	//		Position pos = unit->getPosition();
	//	}
	//}



	m_lastAction = m_currentAction;

	//checkCanIssueNextAction();
}


State SquadEntity::getCurrentState()
{
	State state;

	//The current enemy squad that this squad has to fight against has already been determined.
	//Here, compare the stats of the current squad and the enemy squad, in order to be able to assign
	//values to the State variable.
	assert(m_enemySquad);
	setHealthInState(state);
 	setDpsInState(state);
	setDistanceInState(state);
	setHitPointsInState(state);
	setEnemyHitPointsInState(state);
	setLastActionInState(state);

	return state;
}


void SquadEntity::setAvgDps(int num)
{
	m_avgDps = num;
}

void SquadEntity::setAvgHealth(int health)
{
	m_avgHealth = health;
}

void SquadEntity::setSqDistToClosestEnemyGroup(int dist)
{
	m_sqDistToClosestEnemyGroup = dist;
}

void SquadEntity::calculateAvgHealth()
{
	int totalHealth = 0;
	for(Unitset::iterator it = m_squadUnits.begin(); it < m_squadUnits.end(); it++)
	{
		if(it->getHitPoints() > 0)
		{
			totalHealth += it->getHitPoints();
		}
	}

	if(m_numUnits > 0)
	{
		setAvgHealth(totalHealth/m_numUnits);
	}
}


void SquadEntity::setLastActionInState(State& state)
{
	state.m_action = m_lastAction;
}


void SquadEntity::calculateAvgDps()
{
	float totalDps = 0;
	for(Unitset::iterator it = m_squadUnits.begin(); it < m_squadUnits.end(); it++)
	{
		totalDps += Common::getUnitDps(it);
	}

	if(m_numUnits > 0)
	{
		setAvgDps((int)totalDps/m_numUnits);
	}
}

void SquadEntity::calculateSqDistToClosestEnemyGroup()
{
	Player enemy = Broodwar->enemy();
	if(!enemy)
	{
		return;
	}
	Unitset enemyUnits = Broodwar->enemy()->getUnits();
	if(enemyUnits.size() > 0)
	{
		Position enemyAvgPos = calculateAvgPosition(enemyUnits);

		int distX = enemyAvgPos.x - m_avgPos.x;
		int distY = enemyAvgPos.y - m_avgPos.y;

		setSqDistToClosestEnemyGroup(distX * distX + distY * distY);		
	}
	else
	{
		setSqDistToClosestEnemyGroup(90000000); //This is to make sure that later dist = HIGH
	}

}

void SquadEntity::calculateAvgPosition()
{
	int posX = 0;
	int posY = 0;
	for(Unitset::iterator it = m_squadUnits.begin(); it < m_squadUnits.end(); it++)
	{
		posX += it->getPosition().x;
		posY += it->getPosition().y;
	}

	if(m_numUnits > 0)
	{
		setAvgPosition(Position(posX / m_numUnits, posY / m_numUnits));
	}
}

void SquadEntity::setAvgPosition(BWAPI::Position pos)
{
	m_avgPos = pos;

}

void SquadEntity::setNumUnits(int num)
{
	m_numUnits = m_squadUnits.size();
}

Position SquadEntity::calculateAvgPosition(Unitset units)
{
	int posX = 0;
	int posY = 0;
	for(Unitset::iterator it = units.begin(); it < units.end(); it++)
	{
		posX += it->getPosition().x;
		posY += it->getPosition().y;
	}

	if(m_numUnits > 0)
	{
		return Position(posX / m_numUnits, posY / m_numUnits);
	}

	return Position(0, 0);
}

void SquadEntity::assignCurrentEnemySquad()
{
	//TODO - Iterate over all existing enemy squads, and find the corresponding enemy squad
	m_enemySquad = SquadManager::getInstance()->getEnemySquads()->at(0);
}

int SquadEntity::getAvgDps()
{
	return m_avgDps;
}

int SquadEntity::getAvgHealth()
{
	return m_avgHealth;
}

int SquadEntity::getSqDistToClosestEnemyGroup()
{
	return m_sqDistToClosestEnemyGroup;
}

BWAPI::Position SquadEntity::getAvgPosition()
{
	return m_avgPos;
}

int SquadEntity::getNumUnits()
{
	return m_numUnits;
}

void SquadEntity::calculateNumUnits()
{
	setNumUnits(m_squadUnits.size());
}

void SquadEntity::removeDeadUnits()
{
	m_squadUnits.remove_if(&SquadEntity::checkNoHitPoints);
	calculateNumUnits();
}

bool SquadEntity::checkNoHitPoints(const BWAPI::Unit unit)
{
	return unit->getHitPoints() <= 0; 
}

std::vector<Position> SquadEntity::calculateSurroundPositions(bool forceCalculate)
{
	std::vector<Position> retVect;

	if(m_enemySquad->getNumUnits() <= 0)
	{
		m_posToSurround = Position(0, 0);
		return retVect;
	}

	Position oldSurroundPos = m_posToSurround;
	Position newSurroundPos = m_enemySquad->getAvgPosition();
	if(Common::computeSqDistBetweenPoints(oldSurroundPos, newSurroundPos) > 2500 || forceCalculate)
	{
		m_posToSurround = newSurroundPos;
	}
	else
	{
		return retVect;
	}

	
	float radius = sqrt(std::max(m_enemySquad->getDispersionSqDist(), 130.0f * 130.0f));
	int index = 0;
	double radsBetweenPos = 2 * 3.14159 / m_numUnits;

	for(Unitset::iterator u = m_squadUnits.begin(); u != m_squadUnits.end(); u++, index++)
	{
		Position posEnemy = m_enemySquad->getAvgPosition();
		Position pos;
		pos.x = static_cast<int>(radius * sin(radsBetweenPos * index));
		pos.y = static_cast<int>(radius * cos(radsBetweenPos * index));
		pos = pos + posEnemy;

		retVect.push_back(pos);
	}

	return retVect;

}


void SquadEntity::calculateUnitsDispersion()
{
	float maxSqDistance = 0.0f;

	for(Unitset::iterator u = m_squadUnits.begin(); u != m_squadUnits.end(); ++u)
	{
		Position pos1 = m_enemySquad->getAvgPosition();
		Position pos2 = u->getPosition();
		float sqDist = static_cast<float>(Common::computeSqDistBetweenPoints(pos1, pos2));
		if(sqDist > maxSqDistance)
		{
			maxSqDistance = sqDist;
		}
	}

	m_dispersionSqDist = maxSqDistance;
}

float SquadEntity::getDispersionSqDist() const
{
	return m_dispersionSqDist;
}


//Check if, given the last action that was issued, a new one can be processed
//This is in order not to send too many actions in a short interval of time.
//Sort of like let the actions already issued have a consequence before sending a new one.
bool SquadEntity::checkCanIssueNextAction()
{
	if(m_frameCount > 5)
	{
		m_canIssueNextAction = true;
		m_frameCount = 0;
	}
	else
	{
		m_canIssueNextAction = false;
	}

	return m_canIssueNextAction;

	////SquadEntity* squad = SquadManager::getInstance()->m_squad;
	//if(m_numUnits > 0)
	//{
	//	if(m_currentAction == ATTACK)
	//	{
	//		//Remove info from old FLEE or HOLD actions
	//		m_frameCount = 0;
	//		m_lastActionTilePos = Position(-1, -1);

	//		if(m_enemySquad->getNumUnits() <= 0)
	//		{
	//			//No enemy units on sight, can send new action
	//			m_currentAction = COUNT;
	//			m_canIssueNextAction = true;
	//		}
	//		else
	//		{
	//			//When some unit has started to attack, then we can send a new action
	//			bool isAnyUnitAttacking = false;
	//			for(Unitset::iterator u = m_squadUnits.begin(); u != m_squadUnits.end(); ++u)
	//			{
	//				if(u->isAttacking())
	//				{
	//					isAnyUnitAttacking = true;
	//					break;
	//				}
	//			}

	//			if(isAnyUnitAttacking)
	//			{
	//				m_currentAction = COUNT;
	//				m_canIssueNextAction = true;
	//			}
	//		}
	//	}
	//	else if(m_currentAction == ATTACK_SURROUND)
	//	{
	//		Position enemyPos = m_enemySquad->getAvgPosition();

	//		if(Common::computeSqDistBetweenPoints(m_posToSurround, enemyPos) > 50 * 50)
	//		{
	//			m_currentAction = COUNT;
	//			m_canIssueNextAction = true;
	//		}
	//		else
	//		{
	//			m_canIssueNextAction = false;
	//		}
	//	}
	//	else if(m_currentAction == HOLD)
	//	{
	//		//Remove info from old ATTACK action
	//		m_lastUnitAttacked = NULL;

	//		if(m_frameCount > 5)
	//		{
	//			m_currentAction = COUNT;
	//			m_canIssueNextAction = true;
	//			m_frameCount = 0;
	//		}
	//	}
	//}
	//else
	//{
	//	m_canIssueNextAction = true;
	//}
}

void SquadEntity::attackClosestEnemyUnit()
{
	if(m_enemySquad == NULL || m_enemySquad->getNumUnits() <= 0)
	{
		return;
	}

	Unitset enemyUnits = m_enemySquad->getSquadUnits();

	int index = 0;
	int unitIndexToAttack = 0;
	int minDist = std::numeric_limits<int>::max();
	for(Unitset::iterator it = enemyUnits.begin(); it != enemyUnits.end(); it++, index++)
	{
		Position enemyPos = it->getPosition();
		int dist = enemyPos.getApproxDistance(m_avgPos);
		if(dist < minDist)
		{
			unitIndexToAttack = index;
			minDist = dist;
		}
	}

	for(Unitset::iterator u = m_squadUnits.begin(); u != m_squadUnits.end(); ++u)
	{
		m_lastUnitAttacked = enemyUnits[unitIndexToAttack];


		//if(!performBasicChecks(u))
		//{
		//	break;
		//}

		////Don't send an attack command twice in a row over the same unit
		//bool attackFrame = u->isAttackFrame();
		//bool startingAttack = u->isStartingAttack();
		//bool attacking = u->isAttacking();

		////if(m_issuingNewAction && unitIndex != 0 || unitIndex == 0)
		//{
		//	if(m_lastUnitAttacked == NULL || m_lastUnitAttacked != enemyUnits[unitIndexToAttack])
		//	{
		//		if(!u->isAttackFrame() && !u->isStartingAttack() && !u->isAttacking())
		//		{
		//			//u->attack(enemyUnits[unitIndexToAttack]);

		//			m_lastUnitAttacked = enemyUnits[unitIndexToAttack];
		//		}
		//	}
		//}
	}

}

void SquadEntity::applyCurrentAction()
{
	switch(m_currentAction)
	{
		case ATTACK:
		{
			attackClosestEnemyUnit();
		}
		break;

		case HOLD:
		{
			holdPositions();
		}
		break;

		case ATTACK_SURROUND:
		{
			attackSurround();
		}
		break;

		case ATTACK_HALF_SURROUND:
		{
			attackHalfSurround();
		}
		break;

		default:
		{

		}
		break;
	}
}

bool SquadEntity::performBasicChecks(Unitset::iterator u)
{
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


void SquadEntity::holdPositions()
{
	int tileX = getAvgPosition().x / TILE_SIZE;
	int tileY = getAvgPosition().y / TILE_SIZE;

	for(Unitset::iterator u = m_squadUnits.begin(); u != m_squadUnits.end(); ++u)
	{
		if(!performBasicChecks(u))
		{
			break;
		}

		if(m_lastActionTilePos == Position(-1, -1) || m_lastActionTilePos != Position(tileX, tileY))
		{
			u->holdPosition();
			m_lastActionTilePos = Position(tileX, tileY);
		}
	}
}

void SquadEntity::attackSurround()
{
	std::vector<Position> surroundPositions;
	m_surroundPositions = calculateSurroundPositions(false);

	//if(surroundPositions.size() > 0)
	//{
	//	int index = 0;
	//	for(Unitset::iterator u = m_squadUnits.begin(); u != m_squadUnits.end(); ++u, ++index)
	//	{
	//		if(!performBasicChecks(u))
	//		{
	//			return;
	//		}

	//		u->move(surroundPositions.at(index));
	//		//u->attack(surroundPositions.at(index));
	//	}
	//}
}

void SquadEntity::setHealthInState(State& state)
{
	if(m_numUnits <= 0)
	{
		state.setAvgHealth(NA);
	}
	else if(m_avgHealth < m_enemySquad->getAvgHealth() * 0.8)
	{
		state.setAvgHealth(LOW);
	}
	else if(m_avgHealth >= m_enemySquad->getAvgHealth() * 0.8 && m_avgHealth < m_enemySquad->getAvgHealth() * 1.2)
	//else if(m_avgHealth - m_enemySquad->getAvgHealth() >= 0 && m_avgHealth - m_enemySquad->getAvgHealth() < 15 * m_enemySquad->getNumUnits()) //TODO - REVISE THIS
	{
		state.setAvgHealth(MID);
	}
	else
	{
		state.setAvgHealth(HIGH);
	}
}

void SquadEntity::setDpsInState(State& state)
{
	if(m_numUnits <= 0)
	{
		state.setAvgDps(NA);
	}
	//else if(m_avgDps < m_enemySquad->getAvgDps())
	else if(m_avgDps < m_enemySquad->getAvgDps() * 0.8)
	{
		state.setAvgDps(LOW);
	}
	else if(m_avgDps >= m_enemySquad->getAvgDps() * 0.8 && m_avgDps < m_enemySquad->getAvgDps() * 1.2)
	//else if(m_avgDps - m_enemySquad->getAvgDps() >= 0 && m_avgDps - m_enemySquad->getAvgDps() < 15 * m_enemySquad->getNumUnits()) //TODO - REVISE THIS
	{
		state.setAvgDps(MID);
	}
	else
	{
		state.setAvgDps(HIGH);
	}
}

void SquadEntity::setDistanceInState(State& state)
{
	if(m_sqDistToClosestEnemyGroup < 10000) //TODO - REVISE THIS
	{
		state.setSqDistToClosestEnemyGroup(LOW);
	}
	else if(m_sqDistToClosestEnemyGroup < 1000000)
	{
		state.setSqDistToClosestEnemyGroup(MID);
	}
	else
	{
		state.setSqDistToClosestEnemyGroup(HIGH);
	}
}

void SquadEntity::setHitPointsInState(State& state)
{
	state.m_hitPoints = m_avgHealth;
}

void SquadEntity::setEnemyHitPointsInState(State& state)
{
	state.m_enemyHitPoints = m_enemySquad->getAvgHealth();
}


vector<UnitEntity*> SquadEntity::getSquadUnitEntities()
{
	return UnitManager::getInstance()->getUnitEntitiesBySquad(this);
}

Action SquadEntity::getLastAction() const
{
	return m_lastAction;
}

void SquadEntity::setLastAction(Action action)
{
	m_lastAction = action;
}

Action SquadEntity::getCurrentAction() const
{
	return m_currentAction;
}

void SquadEntity::setCurrentAction(Action action)
{
	m_currentAction = action;
}

State SquadEntity::getLastState() const
{
	return m_lastState;
}

void SquadEntity::setLastState(State state)
{
	m_lastState = state;
}

bool SquadEntity::getCanIssueNextAction() const
{
	return m_canIssueNextAction;
}

void SquadEntity::setCanIssueNextAction(bool yes)
{
	m_canIssueNextAction = yes;
}

bool SquadEntity::getIsEnemySquad() const
{
	return m_isEnemySquad;
}

void SquadEntity::setIsEnemySquad(bool yes)
{
	m_isEnemySquad = yes;
}

BWAPI::Unit SquadEntity::getUnitToAttack(UnitEntity* unitEntity)
{
	assert(unitEntity->getCurrentSquadAction() == ATTACK);
	return m_lastUnitAttacked;
}

BWAPI::Position SquadEntity::calculateActionTilePosForSurround(UnitEntity* unitEntity)
{
	assert(unitEntity->getCurrentSquadAction() == ATTACK_SURROUND || unitEntity->getCurrentSquadAction() == ATTACK_HALF_SURROUND);
	
	if(m_surroundPositions.empty())
	{
		if(unitEntity->getCurrentSquadAction() == ATTACK_SURROUND)
		{
			calculateSurroundPositions(true);
		}
		else if(unitEntity->getCurrentSquadAction() == ATTACK_HALF_SURROUND)
		{
			calculateHalfSurroundPositions(true);
		}
	}

	Position unitPos = unitEntity->getUnit()->getPosition();
	int minDist = std::numeric_limits<int>::max();
	Position retPos = unitPos;
	vector<Position>::iterator removeIt;
	for(vector<Position>::iterator pos = m_surroundPositions.begin(); pos != m_surroundPositions.end(); ++pos)
	{
		int dist = 0;
		dist = Common::computeSqDistBetweenPoints(*pos, unitPos);
		if(dist < minDist)
		{
			dist = minDist;
			retPos = *pos;
			removeIt = pos;
		}
	}

	if(retPos != unitPos)
	{
		m_surroundPositions.erase(removeIt);
	}

	return retPos;
}

bool SquadEntity::canIssueNextAction()
{
	return m_canIssueNextAction;
}

bool SquadEntity::hasNewSurroundPositions() const
{
	return !m_surroundPositions.empty();
}

void SquadEntity::attackHalfSurround()
{
	std::vector<Position> surroundPositions;
	m_surroundPositions = calculateHalfSurroundPositions(false);
}

struct FloatPosition
{
	FloatPosition() {}
	FloatPosition(float x, float y): x(x), y(y) {}
	float x;
	float y;
};

std::vector<BWAPI::Position> SquadEntity::calculateHalfSurroundPositions(bool forceCalculate)
{
	std::vector<Position> retVect;

	if(m_enemySquad->getNumUnits() <= 0)
	{
		m_posToSurround = Position(0, 0);
		return retVect;
	}

	Position oldSurroundPos = m_posToSurround;
	Position newSurroundPos = m_enemySquad->getAvgPosition();
	if(Common::computeSqDistBetweenPoints(oldSurroundPos, newSurroundPos) > 2500 || forceCalculate)
	{
		m_posToSurround = newSurroundPos;
	}
	else
	{
		return retVect;
	}


	float radius = sqrt(std::max(m_enemySquad->getDispersionSqDist(), 130.0f * 130.0f));
	double radsBetweenPos = 2 * 3.14159 / m_numUnits / 2; // /2 because only half surround

	//Translate pos to surround to origin (0, 0) 
	Position avgPosTranslatedToOrigin = m_avgPos - m_posToSurround;
	//Uniform
	FloatPosition floatPos(static_cast<float>(avgPosTranslatedToOrigin.x), static_cast<float>(avgPosTranslatedToOrigin.y));
	float norm = sqrt(floatPos.x * floatPos.x+ floatPos.y * floatPos.y);
	
	FloatPosition normalizedAvgPosTranslatedToOrigin = FloatPosition(avgPosTranslatedToOrigin.x / norm, avgPosTranslatedToOrigin.y / norm);
	//Rotate
	FloatPosition rotatedPos;
	int index = -static_cast<int>(m_numUnits / 2) + 1;
	int rot_index = 0;
	for(Unitset::iterator u = m_squadUnits.begin(); u != m_squadUnits.end(); u++, index++)
	{
		rot_index = rot_index * (-1);

		rotatedPos.x = static_cast<float>(normalizedAvgPosTranslatedToOrigin.x * cos(radsBetweenPos * rot_index) - normalizedAvgPosTranslatedToOrigin.y * sin(radsBetweenPos * rot_index));
		rotatedPos.y = static_cast<float>(normalizedAvgPosTranslatedToOrigin.x * sin(radsBetweenPos * rot_index) + normalizedAvgPosTranslatedToOrigin.y * cos(radsBetweenPos * rot_index));

		if(rot_index >= 0)
		{
			rot_index++;
			rot_index -= 2 * rot_index;
		}

		//Multiply by surround radius
		rotatedPos.x *= radius;
		rotatedPos.y *= radius;

		//Translate back to correct position
		rotatedPos.x += m_posToSurround.x;
		rotatedPos.y += m_posToSurround.y;

		Position finalPos(static_cast<int>(rotatedPos.x), static_cast<int>(rotatedPos.y));
		retVect.push_back(finalPos);


	}

	return retVect;

}
