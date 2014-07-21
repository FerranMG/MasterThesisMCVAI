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



	for(vector<UnitEntity*>::iterator unit = m_allUnitEntities.begin(); unit != m_allUnitEntities.end(); unit++)
	{
		if(!(*unit)->getIsEnemyUnit() && (*unit)->canIssueNextAction())
		{
			UnitState stateNew = (*unit)->getCurrentUnitState();

			if((*unit)->canIssueNextAction())
			{
				QLearningMgr::getInstance()->updateUnitQ((*unit)->getLastUnitState(), (*unit)->getLastUnitAction(), (*unit)->getLastSquadAction(), stateNew);

				(*unit)->setCanIssueNextAction(false);
				(*unit)->setLastUnitAction((*unit)->getCurrentUnitAction());
				(*unit)->setLastSquadAction((*unit)->getCurrentSquadAction());

				(*unit)->applyCurrentUnitAction();
			}
		}

		(*unit)->checkCanIssueNextAction();


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

//struct Eraser
//{
//	Eraser(UnitEntity* unitEntity) : unitEntity(unitEntity) {}
//	UnitEntity* unitEntity;
//	bool operator()(UnitEntity* unitEntity) const
//	{
//		return unitEntity->getUnit()->getHitPoints() <= 0;
//	}
//};

bool checkNoHitPoints(const UnitEntity* unitEntity)
{
	return unitEntity->getUnit()->getHitPoints() <= 0; 
}

void UnitManager::removeDeadUnits()
{
	//vector<UnitEntity*> entitiesToRemove;
	//for(vector<UnitEntity*>::iterator unit = m_allUnitEntities.begin(); unit != m_allUnitEntities.end(); unit++)
	//{
	//	//if((*unit)->getUnitIterator()->getHitPoints() <= 0)
	//	if((*unit)->getUnit()->getHitPoints() <= 0)
	//	{
	//		entitiesToRemove.push_back((*unit));
	//	}
	//}

	m_allUnitEntities.erase(remove_if(m_allUnitEntities.begin(), m_allUnitEntities.end(), checkNoHitPoints), m_allUnitEntities.end());
	//m_allUnitEntities.erase(remove(m_allUnitEntities.begin(), m_allUnitEntities.end(), ))

	//for(vector<UnitEntity*>::iterator deadUnit = entitiesToRemove.begin(); deadUnit != entitiesToRemove.end(); deadUnit++)
	//{
	//	m_allUnitEntities.erase(deadUnit);
	//}
}

void UnitManager::removeAllUnitEntities()
{
	while(m_allUnitEntities.size() > 0)
	{
		m_allUnitEntities.pop_back();
	}
}

