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

				unit->setLastUnitAction(unit->getCurrentUnitAction());
				unit->setUnitAction(unitAction);
				//unit->setLastSquadAction(unit->getCurrentSquadAction());
				unit->setLastSquadAction(unit->getLastSquadAction());

				unit->setCanIssueNextAction(false);

				unit->setLastUnitState(stateNew);

				unit->applyCurrentUnitAction();
			}

			unit->checkCanIssueNextAction();
		}



		//Position posLeftTop = (*unit)->getPosition();
		//posLeftTop.x -= 20;
		//posLeftTop.y -= 20;
		//Position posRightBottom = (*unit)->getPosition();

		//posRightBottom.x += 20;
		//posRightBottom.y += 20;
		//Broodwar->drawBoxScreen(posLeftTop, posRightBottom, Colors::Blue);
		//Broodwar->drawBoxScreen(posLeftTop, posRightBottom, Colors::Red, true);

		//Broodwar->drawTextScreen(Position(100, 100), "hoasdfoahdfopihs");
		//Broodwar->drawLineScreen(Position(100, 100), Position(500, 500), Colors::Blue);
		//
		//m_unit->pos
		//Position tilePosLeftTop = m_unit->getTilePosition();
		//Position tilePosRightBototm = m_unit->getTilePosition();
		//tilePosLeftTop.x -= 20;
		//tilePosLeftTop.y -= 20;
		//tilePosRightBototm.x += 20;
		//tilePosRightBototm.y += 20;

		//Broodwar->drawBoxScreen(tilePosLeftTop, tilePosRightBototm, Colors::Red, true);

	
	}
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
