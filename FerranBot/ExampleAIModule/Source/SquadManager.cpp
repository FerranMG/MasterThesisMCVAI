#include "SquadManager.h"
#include "QLearningMgr.h"

using namespace BWAPI;


SquadManager* SquadManager::instance = NULL;

SquadManager::SquadManager()
{
	m_squads = new std::vector<SquadEntity*>;
	m_enemySquads = new std::vector<SquadEntity*>;
}

SquadManager::~SquadManager()
{
	instance = NULL;
}

SquadManager* SquadManager::getInstance()
{
	if(instance == NULL)
	{
		instance = new SquadManager();
	}  
	return instance;
}


void SquadManager::update()
{
	//TODO Manage squad creation, merge, split, etc.


	for(size_t squadIdx = 0; squadIdx < m_squads->size(); ++squadIdx)
	{
		m_squads->at(squadIdx)->update();
		m_squads->at(squadIdx)->calculateUnitsDispersion();
	}



	//Add newly discovered enemy units to the current squad.
	Player enemy = Broodwar->enemy();
	if(!enemy)
	{
		return;
	}

	Unitset enemyUnits = Broodwar->enemy()->getUnits();
	for(Unitset::iterator it = enemyUnits.begin(); it != enemyUnits.end(); it++)
	{
		//TODO - Add units to corresponding enemy squad
		Unitset currentSquadUnits = m_enemySquads->at(0)->getSquadUnits();
		if(currentSquadUnits.find(*it) == currentSquadUnits.end())
		{
			m_enemySquads->at(0)->addUnit(it);
			m_enemySquads->at(0)->m_absoluteNumberOfUnitsEverInSquad++;
		}
	}


	for(size_t enemySquadIdx = 0; enemySquadIdx < m_enemySquads->size(); ++enemySquadIdx)
	{
		if(m_enemySquads->at(enemySquadIdx)->getNumUnits() > 0)
		{
			m_enemySquads->at(enemySquadIdx)->update();
			m_enemySquads->at(enemySquadIdx)->calculateUnitsDispersion();
		}
	}




		//FOR EACH SQUAD
	for(std::vector<SquadEntity*>::iterator squadEntity = SquadManager::getInstance()->getSquads()->begin(); squadEntity != SquadManager::getInstance()->getSquads()->end(); ++squadEntity)
	{
		SquadEntity* squad = (*squadEntity);
		State stateNew = squad->getCurrentState();
		if(squad->getNumUnits() > 0)
		{
			if(squad->canIssueNextAction())
			{
				Action action = QLearningMgr::getInstance()->updateSquadQ(squad->getLastState(), squad->getLastAction(), stateNew);

				//Broodwar->sendText("Last State H %d DPS %d dist %d Action %d", squad->getLastState().m_avgHealthGroup, squad->getLastState().m_avgDpsGroup, squad->getLastState().m_distToClosestEnemyGroup, squad->getLastAction());
				assert(action >= ATTACK && action <= COUNT);

				squad->setLastAction(squad->getCurrentAction());

				squad->setCurrentAction(action);

				squad->setCanIssueNextAction(false);

				squad->setLastState(stateNew);

				squad->applyCurrentAction();
			}

			squad->checkCanIssueNextAction();
		}
	}

	QLearningMgr::getInstance()->resetForcedReward();
}


void SquadManager::addSquad(SquadEntity* squadEntity)
{
	m_squads->push_back(squadEntity);
	squadEntity->setIsEnemySquad(false);
}


void SquadManager::addEnemySquad(SquadEntity* squadEntity)
{
	m_enemySquads->push_back(squadEntity);
	squadEntity->setIsEnemySquad(true);
}


std::vector<SquadEntity*>* SquadManager::getSquads() const
{
	return m_squads;
}


std::vector<SquadEntity*>* SquadManager::getEnemySquads() const
{
	return m_enemySquads;
}

void SquadManager::removeAllSquads()
{
	while(m_squads->size() > 0)
	{
		m_squads->pop_back();
	}

	while(m_enemySquads->size() > 0)
	{
		m_enemySquads->pop_back();
	}


}
