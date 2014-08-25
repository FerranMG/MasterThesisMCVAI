#include "UnitManager.h"
#include "UnitEntity.h"
#include "SquadEntity.h"
#include "QLearningMgr.h"
#include "SquadManager.h"

using namespace BWAPI;
using namespace std;


UnitManager* UnitManager::instance = nullptr;
int UnitManager::s_enemyUnitsAlive = 0;
int UnitManager::s_friendlyUnitsAlive = 0;
int UnitManager::s_killedEnemyUnitsOld = 0;
int UnitManager::s_killedEnemyUnits = 0;
int UnitManager::s_killedEnemyUnitsByFriendly = 0;


UnitManager::UnitManager()
{
}


UnitManager::~UnitManager()
{
	instance = nullptr;
}


UnitManager* UnitManager::getInstance()
{
	if(instance == nullptr)
	{
		instance = new UnitManager();
	}

	return instance;
}


UnitEntity* UnitManager::createUnit(Unitset::iterator u)
{
 	UnitEntity* unit = new UnitEntity(u);
	m_allUnitEntities.push_back(unit);

	return unit;
}


vector<UnitEntity*> UnitManager::getUnitEntitiesBySquad(SquadEntity* squad)
{
	vector<UnitEntity*> foundUnits;

	for(vector<UnitEntity*>::iterator unit = m_allUnitEntities.begin(); unit != m_allUnitEntities.end(); unit++)
	{
		if((*unit)->getSquad() == squad)
		{
			foundUnits.push_back(*unit);
		}
	}

	return foundUnits;
}


void UnitManager::update()
{
	calculateNumEnemyUnitsKilledByFriendly();

	removeDeadUnits();

	for(vector<UnitEntity*>::iterator unit = m_allUnitEntities.begin(); unit != m_allUnitEntities.end(); unit++)
	{
		(*unit)->update();
	}



	for(vector<UnitEntity*>::iterator unitEntity = m_allUnitEntities.begin(); unitEntity != m_allUnitEntities.end(); unitEntity++)
	{
		UnitEntity* unit = (*unitEntity);
		if(!unit->getIsEnemyUnit())
		{
			UnitState stateNew = unit->getCurrentUnitState();

			if(unit->canIssueNextAction())
			{
				UnitAction unitAction = QLearningMgr::getInstance()->updateUnitQ(unit->getLastUnitState(), unit->getLastUnitAction(), unit->getLastSquadAction(), stateNew);

				Broodwar->sendText("GameNum %d GamesWon %d SquadAction %s UnitAction %s", QLearningMgr::getInstance()->m_numGamesPlayed, QLearningMgr::getInstance()->m_numGamesWon, QLearningMgr::getInstance()->TranslateActionToWord(unit->getCurrentSquadAction()).c_str(), QLearningMgr::getInstance()->TranslateActionToWord(unitAction).c_str());

				//if(unitAction != unit->getLastUnitAction())
				//{
				//	unit->resetCurrentActionTilePos();
				//}

				//unit->setUnitAction(unitAction); //Current action will be used in applyCurrentUnitAction

				////We store the current values as the last values.
				//unit->setLastUnitAction(unitAction);
				//unit->setLastUnitState(stateNew);
				//unit->setLastSquadAction(unit->getCurrentSquadAction());

				//unit->setCanIssueNextAction(false);
				//

				unit->setLastUnitAction(unitAction);
				//unit->setLastUnitAction(unit->getCurrentUnitAction());
				unit->setUnitAction(unitAction);
				unit->setLastSquadAction(unit->getLastSquadAction());

				unit->setCanIssueNextAction(false);

				unit->setLastUnitState(stateNew);

				if(unit->getCurrentUnitAction() != unit->getLastUnitAction())
				{
					unit->resetCurrentActionTilePos();
				}

				unit->applyCurrentUnitAction();
			}
			else if(unit->mustUpdateCurrentAction())
			{
				unit->applyCurrentUnitAction();
			}

			unit->checkCanIssueNextAction();
		}
	}

	QLearningMgr::getInstance()->resetForcedUnitReward();
}


bool checkNoHitPoints(const UnitEntity* unitEntity)
{
	if(unitEntity->getUnit()->getHitPoints() <= 0 )
	{
		if(!unitEntity->getIsEnemyUnit())
		{
			QLearningMgr::getInstance()->forceUnitReward(-1000);
			QLearningMgr::getInstance()->updateUnitQ(unitEntity->getLastUnitState(), unitEntity->getLastUnitAction(), unitEntity->getLastSquadAction(), unitEntity->getCurrentUnitState());

			int testKillCount = unitEntity->getUnit()->getKillCount();
			int a = 0;
		}
		else
		{
			//SquadManager::getInstance->getEnemySquads->at(0)->m_absoluteNumberOfUnitsEverInSquad--;
		}

		return true;
	}

	return false; 
}

void UnitManager::removeDeadUnits()
{
	//int countEnemyUnitsOld = getNumEnemyUnits();
	//int countFriendlyUnitsOld = getNumFriendlyUnits();
	//s_killedEnemyUnits = getNumEnemyUnitsKilled();

	//int countOfEnemyUnitsDeadThisFrame = s_killedEnemyUnits - s_killedEnemyUnitsOld;
	//s_killedEnemyUnitsOld = s_killedEnemyUnits;
	//
	m_allUnitEntities.erase(remove_if(m_allUnitEntities.begin(), m_allUnitEntities.end(), checkNoHitPoints), m_allUnitEntities.end());
	
	//int countEnemyUnits = getNumEnemyUnits();
	//int countFriendlyUnits = getNumFriendlyUnits();

	//int visibleAndAliveEnemyUnits = SquadManager::getInstance->getEnemySquads->at(0)->m_absoluteNumberOfUnitsEverInSquad;
	//if


	//int deadOrDisappearedEnemyUnits = countEnemyUnitsOld - countEnemyUnits;
	//int disappearedUnits = deadOrDisappearedEnemyUnits - s_killedEnemyUnits;

	//s_friendlyUnitsAlive = countFriendlyUnits;

	//SquadManager::getInstance->getEnemySquads->at(0)->m_absoluteNumberOfUnitsEverInSquad = ;

	//s_enemyUnitsAlive = SquadManager::getInstance->getEnemySquads->at(0)->m_absoluteNumberOfUnitsEverInSquad - 

	//if(countOfEnemyUnitsDeadThisFrame > 0)
	//{
	//	s_enemyUnitsAlive = countEnemyUnits + (deadOrDisappearedEnemyUnits - countOfEnemyUnitsDeadThisFrame);
	//}
	//else
	//{
	//	s_enemyUnitsAlive = countEnemyUnits;
	//}
	//assert(s_friendlyUnitsAlive >= 0);
	//assert(s_enemyUnitsAlive >= 0);


	//SquadManager::getInstance()->m_enemySquads->at(0)->m_absoluteNumberOfUnitsEverInSquad = s_enemyUnitsAlive;

}

void UnitManager::removeAllUnitEntities()
{
	while(m_allUnitEntities.size() > 0)
	{
		m_allUnitEntities.pop_back();
	}
}

vector<UnitEntity*> UnitManager::getAllUnitEntities() const
{
	return m_allUnitEntities;
}

int UnitManager::getNumEnemyUnits()
{
	int numEnemyUnits = 0;
	for(vector<UnitEntity*>::iterator unit = m_allUnitEntities.begin(); unit != m_allUnitEntities.end(); unit++)
	{
		if((*unit)->getIsEnemyUnit())
		{
			++numEnemyUnits;
		}
	}

	return numEnemyUnits;
}

int UnitManager::getNumFriendlyUnits()
{
	int numFriendlyUnits = 0;
	for(vector<UnitEntity*>::iterator unit = m_allUnitEntities.begin(); unit != m_allUnitEntities.end(); unit++)
	{
		if((*unit)->getIsEnemyUnit() == false)
		{
			++numFriendlyUnits;
		}
	}

	return numFriendlyUnits;
}

int UnitManager::getNumEnemyUnitsKilled()
{
	int numEnemyUnitsKilled = 0;
	for(Unitset::iterator unit = SquadManager::getInstance()->m_differentFriendlyUnits.begin(); unit != SquadManager::getInstance()->m_differentFriendlyUnits.end(); ++unit)
	{
		numEnemyUnitsKilled += unit->getKillCount();
	}

	//for(vector<UnitEntity*>::iterator unit = m_allUnitEntities.begin(); unit != m_allUnitEntities.end(); unit++)
	//{
	//	if((*unit)->getIsEnemyUnit() == false)
	//	{
	//		int killCount = (*unit)->getUnit()->getKillCount();
	//		numEnemyUnitsKilled += killCount;
	//	}
	//}

	return numEnemyUnitsKilled;
}

void UnitManager::calculateNumEnemyUnitsKilledByFriendly()
{
	int numEnemyUnitsKilled = 0;
	for(Unitset::iterator unit = SquadManager::getInstance()->m_differentFriendlyUnits.begin(); unit != SquadManager::getInstance()->m_differentFriendlyUnits.end(); ++unit)
	{
		numEnemyUnitsKilled += unit->getKillCount();
	}

	if(numEnemyUnitsKilled > s_killedEnemyUnitsByFriendly)
	{
		s_killedEnemyUnitsByFriendly = numEnemyUnitsKilled;
	}
}
