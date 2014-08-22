#include "UnitManager.h"
#include "UnitEntity.h"
#include "SquadEntity.h"
#include "QLearningMgr.h"

using namespace BWAPI;
using namespace std;


UnitManager* UnitManager::instance = nullptr;


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

				Broodwar->sendText("SquadAction %s ", QLearningMgr::getInstance()->TranslateActionToWord(unit->getCurrentSquadAction()).c_str(), " UnitAction %s " , QLearningMgr::getInstance()->TranslateActionToWord(unitAction).c_str());


				unit->setLastUnitAction(unit->getCurrentUnitAction());
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
	return unitEntity->getUnit()->getHitPoints() <= 0; 
}

void UnitManager::removeDeadUnits()
{
	m_allUnitEntities.erase(remove_if(m_allUnitEntities.begin(), m_allUnitEntities.end(), checkNoHitPoints), m_allUnitEntities.end());
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