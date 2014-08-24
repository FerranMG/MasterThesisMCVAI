
#include "QLearningMgr.h"
#include <fstream>
#include <iostream>
#include <sys\stat.h>
#include <limits>

using namespace BWAPI;
using namespace std;


const float GAMMA = 0.9f;

QLearningMgr* QLearningMgr::instance = nullptr;
const int QLearningMgr::MAX_STATES_VISITED_BEFORE_TOTAL_EXPLOIT = 2000;
const int QLearningMgr::MAX_NUM_GAMES_PLAYED_BEFORE_TOTAL_EXPLOIT = 1000;
float QLearningMgr::m_exploreExploitCoef = -1.0f;
int QLearningMgr::m_totalNumStatesVisited = 0;


SApair::SApair(State stateNew, Action actionNew)
{
	state = stateNew;
	action = actionNew;
}

SApair::SApair()
{
}

USApair::USApair(UnitState stateNew, UnitAction actionNew)
{
	state = stateNew;
	action = actionNew;
}

USApair::USApair()
{
}


QLearningMgr::QLearningMgr()
{
	q_map = new std::map<SApair, float, compare>;
	q_count = new std::map<SApair, int, compare>;
	q_mapUnit = new std::map<USApair, float, unitCompare>;
	q_countUnit = new std::map<USApair, int, unitCompare>;
	m_forceReward = 0.0f;
	m_forceUnitReward = 0.0f;

	m_numGamesPlayed = 0;
	m_numGamesWon = 0;

	m_gamesWon = new std::vector<int>;
}


QLearningMgr::~QLearningMgr()
{
	delete q_map;
	delete q_count;
	delete q_mapUnit;
	delete q_countUnit;
	delete  m_gamesWon;

	q_map = nullptr;
	q_count = nullptr;
	q_mapUnit = nullptr;
	q_countUnit = nullptr;
	m_gamesWon = nullptr;

	instance = nullptr;
}

QLearningMgr* QLearningMgr::getInstance()
{
	if(instance == nullptr)
	{
		instance = new QLearningMgr();
	}

	return instance;
}


Action QLearningMgr::updateSquadQ(State lastState, Action lastAction, State stateNew)
{
	//DEBUG
	static bool m_forceExploration = false;
	static bool m_forceExploitation = false;
	if(m_forceExploration)
	{
		m_exploreExploitCoef = -1.0f;
	}
	else if(m_forceExploitation)
	{
		m_exploreExploitCoef = 1.0f;
	}
	else
	//_DEBUG
	{
		updateExploreExploitCoef();
	}
	m_totalNumStatesVisited++;
	//DEBUG
	static Group a = LOW;
	static Group b = LOW;
	static Group c = LOW;

	if( lastState.m_avgDpsGroup == a && lastState.m_avgHealthGroup == b &&lastState.m_distToClosestEnemyGroup == c)
	{
		int a = 0;
	}

	//_DEBUG
	SApair sapair = SApair(lastState, lastAction);
	Action outNewAction = ATTACK;

	if(q_map->find(sapair) != q_map->end())
	{
		float optimalFutureValue = getOptimalFutureValue(stateNew, outNewAction);
		float currentQValue = q_map->at(sapair);

		float alpha = getAlpha(sapair);
		float reward = getReward(sapair, stateNew);
		currentQValue = currentQValue + alpha * (reward + GAMMA * optimalFutureValue - currentQValue); 

		std::pair<std::map<SApair, float>::iterator,bool> ret;
		std::map<SApair, int, compare>::iterator it = q_count->find(sapair);
		q_map->at(sapair) = currentQValue;
		q_count->at(sapair) = it->second + 1;
	}
	else
	{
		q_map->insert(std::pair<SApair, float>(sapair, 0.0f));
		q_count->insert(std::pair<SApair, int>(sapair, 0));
		outNewAction = (Action)0;
	}
	
	return outNewAction;
}


float QLearningMgr::getAlpha(SApair sapair)
{
	return 0.1f; //In stochastic environments, a constant learning rate is usually used
}


float QLearningMgr::getReward(SApair sapair, State stateNew)
 {
	 if(m_forceReward != 0)
	 {
		 return m_forceReward;
	 }

	float reward = 0.0f;

	//penalize changing action
	if(sapair.state.m_action != stateNew.m_action)
	{
		reward -= 2.0f;
	}

	State stateLast = sapair.state;
	if(stateLast.m_avgHealthGroup < stateNew.m_avgHealthGroup) //Enemy has less life than before
	{
		if(stateLast.m_avgDpsGroup < stateNew.m_avgDpsGroup) //Enemy has less DPS than before
		{
			reward += 20.0f;
		}
		else
		{
			reward += 10.0f;
		}
	}
	else if(stateLast.m_avgHealthGroup > stateNew.m_avgHealthGroup) //We have less life than before
	{
		if(stateLast.m_avgDpsGroup > stateNew.m_avgDpsGroup) //We has less DPS than before
		{
			reward -= 20.0f;
		}
		else
		{
			reward -= 10.0f;
		}
	}
	else if(stateLast.m_avgHealthGroup			== stateNew.m_avgHealthGroup			&& stateLast.m_avgHealthGroup == LOW
		 && stateLast.m_avgDpsGroup		== stateNew.m_avgDpsGroup		&& stateLast.m_avgDpsGroup == LOW )
	{
		//Low health and low dps => try to run away!
		float rew = RunAway(stateLast, stateNew);
		reward -= rew;
	}
	else if(stateLast.m_hitPoints <= stateNew.m_hitPoints) //we are not losing life
	{
		if(stateLast.m_enemyHitPoints > stateNew.m_enemyHitPoints) //enemy is losing life
		{
			reward += 5.0f;
		}
		else
		{
			//Nothing
		}
	}
	else if(stateLast.m_hitPoints > stateNew.m_hitPoints) //we are losing life
	{
		if(stateLast.m_enemyHitPoints >= stateNew.m_enemyHitPoints) //enemy is losing life
		{
			if(stateNew.m_avgHealthGroup >= MID) //we can afford to lose life
			{
				reward += 3.0f;
			}
			else
			{
				reward -= 3.0f;
			}
		}
		else
		{
			//Nothing
		}
	}
	else
	{
		//Nothing
	}

	return reward;
}


float QLearningMgr::getOptimalFutureValue(State state, Action& outNewAction)
{
	float exploreExploitRand = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 1;

	float exploreExploitCoef = getExploreExploitCoefficient();
	if(exploreExploitCoef > exploreExploitRand) //Exploit
	{
		float maxValue = -1000000.0f;
		
		bool stateFound = false;

		for(int act = 0; act < COUNT; act++)
		{
			SApair sapair = SApair(state, (Action)act);

			map<SApair, float, compare>::iterator it = q_map->find(sapair);
			if(it != q_map->end())
			{
				stateFound = true;

				float currentQValue = it->second;
				if(currentQValue > maxValue)
				{
					maxValue = currentQValue;
					outNewAction = (Action)act;
				}
			}
		}

		if(stateFound)
		{
			return maxValue;
		}
		else
		{
			outNewAction = (Action)0;
			return 0.0f;
		}
	}
	else //Explore
	{
		float qValue = 0.0f;
		bool stateFound = false;
		int numTimesStateVisited = 100000;

		for(int act = 0; act < COUNT; act++)
		{
			SApair sapair = SApair(state, (Action)act);

			map<SApair, float, compare>::iterator it = q_map->find(sapair);
			if(it != q_map->end())
			{
				map<SApair, int, compare>::iterator it_count = q_count->find(sapair);
				if(it_count != q_count->end())
				{
					if(it_count->second < numTimesStateVisited)
					{
						stateFound = true;
						numTimesStateVisited = it_count->second;
						outNewAction = (Action)(it_count->first.action);
						qValue = it->second;
					}
				}
				else
				{
					assert(false);
				}
			}
			else
			{
				outNewAction =(Action)act;
				return 0.0f;
			}
		}

		if(stateFound)
		{
			return qValue;
		}
		else
		{
			outNewAction = (Action)0;
			return 0.0f;
		}
	}
}



float QLearningMgr::getOptimalFutureValueForUnit(UnitState state, UnitAction& outNewAction)
{
	float exploreExploitRand = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 1;

	float exploreExploitCoef = getExploreExploitCoefficient();
	if(exploreExploitCoef > exploreExploitRand) //Exploit
	{
		float maxValue = -1000000.0f;

		bool stateFound = false;

		for(int act = SQUAD_ACTION; act < UNIT_COUNT; act++)
		{
			USApair usapair = USApair(state, (UnitAction)act);

			map<USApair, float, unitCompare>::iterator it = q_mapUnit->find(usapair);
			if(it != q_mapUnit->end())
			{
				stateFound = true;

				float currentQValue = it->second;
				if(currentQValue > maxValue)
				{
					maxValue = currentQValue;
					outNewAction = (UnitAction)act;
				}
			}
		}

		if(stateFound)
		{
			return maxValue;
		}
		else
		{
			outNewAction = (UnitAction)0;
			return 0.0f;
		}
	}
	else //Explore
	{
		float qValue = 0.0f;
		bool stateFound = false;
		int numTimesStateVisited = 100000;

		for(int act = SQUAD_ACTION; act < UNIT_COUNT; act++)
		{
			USApair usapair = USApair(state, (UnitAction)act);

			map<USApair, float, unitCompare>::iterator it = q_mapUnit->find(usapair);
			map<USApair, float, unitCompare>::iterator end = q_mapUnit->end();
			
			//If this new state has already been found before, choose the least used action
			if(it != q_mapUnit->end())
			{
				map<USApair, int, unitCompare>::iterator it_count = q_countUnit->find(usapair);
				if(it_count != q_countUnit->end())
				{
					if(it_count->second < numTimesStateVisited)
					{
						stateFound = true;
						numTimesStateVisited = it_count->second;
						outNewAction = (UnitAction)(it_count->first.action);
						qValue = it->second;
					}
				}
				else
				{
					assert(false);
				}
			}
			else
			{
				outNewAction =(UnitAction)act;
				return 0.0f;
			}
		}

		if(stateFound)
		{
			return qValue;
		}
		else
		{
			outNewAction = (UnitAction)0;
			return 0.0f;
		}
	}
}


Action QLearningMgr::getAction(State state)
{
	if(m_totalNumStatesVisited < 200)
	{
		return (Action)0;
	}
	else if(m_totalNumStatesVisited < 400)
	{
		return (Action)2;
	}
	else
	{
		int random = std::rand() % 2;
		if(random == 0)
		{
			return (Action)0;
		}
		else
		{
			return (Action)2;
		}
	}
}


void QLearningMgr::writeToStream()
{
	writeQMapHumanReadable();

	writeQMapBinary();

	writeQCountBinary();

	writeQVars();
}


void QLearningMgr::readFromStream()
{
	struct stat results;
	stat("FerranBotData\\qmap.bin", &results);

	//std::map<SApair, float, compare>* readMap = new std::map<SApair, float, compare>;
	ifstream ifs_map("FerranBotData\\qmap.bin", ios::binary);
	if(!ifs_map.fail())
	{
		ifs_map.seekg(0, ios::beg);
		while(ifs_map.eof() == false)
		{
			SApair sapair;
			float qvalue;
			ifs_map.read((char*)&sapair, sizeof(SApair));
			//ifs_map.seekg(sizeof(SApair), ios::cur);
			ifs_map.read((char*)&qvalue, sizeof(float));
			//ifs_map.seekg(sizeof(float), ios::cur);

			q_map->insert(std::pair<SApair, float>(sapair, qvalue));

		}
	}

	ifs_map.close();




	stat("FerranBotData\\qmapUnit.bin", &results);

	//std::map<SApair, float, compare>* readMap = new std::map<SApair, float, compare>;
	ifstream ifs_mapUnit("FerranBotData\\qmapUnit.bin", ios::binary);
	if(!ifs_mapUnit.fail())
	{
		ifs_mapUnit.seekg(0, ios::beg);
		while(ifs_mapUnit.eof() == false)
		{
			USApair usapair;
			float qvalue;
			ifs_mapUnit.read((char*)&usapair, sizeof(USApair));
			//ifs_map.seekg(sizeof(SApair), ios::cur);
			ifs_mapUnit.read((char*)&qvalue, sizeof(float));
			//ifs_map.seekg(sizeof(float), ios::cur);

			q_mapUnit->insert(std::pair<USApair, float>(usapair, qvalue));

		}
	}

	ifs_mapUnit.close();




	stat("FerranBotData\\qcount.bin", &results);

	//std::map<SApair, int, compare>* readCount = new std::map<SApair, int, compare>;
	ifstream ifs_count("FerranBotData\\qcount.bin", ios::binary);

	if(!ifs_count.fail())
	{
		ifs_count.seekg(0, ios::beg);
		while(ifs_count.eof() == false)
		{
			SApair sapair;
			int count;

			ifs_count.read((char*)&sapair, sizeof(SApair));
			ifs_count.read((char*)&count, sizeof(int));

			q_count->insert(std::pair<SApair, int>(sapair, count));
		}
	}

	ifs_count.close();



	stat("FerranBotData\\qcountUnit.bin", &results);

	//std::map<SApair, int, compare>* readCount = new std::map<SApair, int, compare>;
	ifstream ifs_countUnit("FerranBotData\\qcountUnit.bin", ios::binary);

	if(!ifs_countUnit.fail())
	{
		ifs_countUnit.seekg(0, ios::beg);
		while(ifs_countUnit.eof() == false)
		{
			USApair usapair;
			int count;

			ifs_countUnit.read((char*)&usapair, sizeof(USApair));
			ifs_countUnit.read((char*)&count, sizeof(int));

			q_countUnit->insert(std::pair<USApair, int>(usapair, count));
		}
	}

	ifs_countUnit.close();




	stat("FerranBotData\\qvars.bin", &results);

	ifstream ifs_vars("FerranBotData\\qvars.bin", ios::binary);

	if(!ifs_vars.fail())
	{
		ifs_count.seekg(0, ios::beg);
		{
			ifs_vars.read((char*)&m_exploreExploitCoef, sizeof(float));
			ifs_vars.read((char*)&m_totalNumStatesVisited, sizeof(int));

			ifs_vars.read((char*)&m_numGamesPlayed, sizeof(int));
			ifs_vars.read((char*)&m_numGamesWon, sizeof(int));

			for(int i = 0; i < m_numGamesPlayed; ++i)
			{
				if(ifs_vars.eof())
				{
					break;
				}

				int gameWon;

				ifs_vars.read((char*)&gameWon, sizeof(int));
				m_gamesWon->push_back(gameWon);
			}
		}
	}

	ifs_vars.close();
}


float QLearningMgr::getExploreExploitCoefficient()
{
	return m_exploreExploitCoef;
}

void QLearningMgr::updateExploreExploitCoef()
{
	float totalExplore = -1.0f;
	float totalExploit = 1.0f;
	//if(getTotalNumStatesVisited() < MAX_STATES_VISITED_BEFORE_TOTAL_EXPLOIT)
	//{
	//	m_exploreExploitCoef = (totalExplore * (MAX_STATES_VISITED_BEFORE_TOTAL_EXPLOIT - getTotalNumStatesVisited()) + totalExploit * getTotalNumStatesVisited()) / MAX_STATES_VISITED_BEFORE_TOTAL_EXPLOIT;
	//}
	//else
	//{
	//	m_exploreExploitCoef = totalExploit;
	//}


	if(m_numGamesPlayed < MAX_NUM_GAMES_PLAYED_BEFORE_TOTAL_EXPLOIT)
	{
		m_exploreExploitCoef = (totalExplore * (MAX_NUM_GAMES_PLAYED_BEFORE_TOTAL_EXPLOIT - m_numGamesPlayed) + totalExploit * m_numGamesPlayed) / MAX_NUM_GAMES_PLAYED_BEFORE_TOTAL_EXPLOIT;
	}
	else
	{
		m_exploreExploitCoef = totalExploit;
	}
}

int QLearningMgr::getTotalNumStatesVisited()
{
	return m_totalNumStatesVisited;
}

float QLearningMgr::RunAway(const State& stateLast, const State& stateNew)
{
	//Low health and low dps => try to run away!

	if(stateLast.m_distToClosestEnemyGroup < stateNew.m_distToClosestEnemyGroup) 
	{
		if(stateNew.m_distToClosestEnemyGroup == MID)
		{
			return 7.5f;
		}
		else if(stateNew.m_distToClosestEnemyGroup == HIGH)
		{
			return 10.0f;
		}
		return 0.0f;
	}
	else if(stateLast.m_distToClosestEnemyGroup == stateNew.m_distToClosestEnemyGroup)
	{
		if(stateNew.m_distToClosestEnemyGroup == LOW)
		{
			return -2.5f;
		}
		else if(stateNew.m_distToClosestEnemyGroup == MID)
		{
			return -0.5f;
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		if(stateNew.m_distToClosestEnemyGroup == LOW)
		{
			return -7.5f;
		}
		else if(stateNew.m_distToClosestEnemyGroup == MID)
		{
			return -2.5f;
		}
		else
		{
			return 0.0f;
		}
	}
}

float QLearningMgr::RunAwayForUnit(const UnitState& stateLast, const UnitState& stateNew)
{
	//Low health and low dps => try to run away!

	if(stateLast.m_distToClosestEnemyGroup < stateNew.m_distToClosestEnemyGroup) 
	{
		if(stateNew.m_distToClosestEnemyGroup == MID)
		{
			return 7.5f;
		}
		else if(stateNew.m_distToClosestEnemyGroup == HIGH)
		{
			return 10.0f;
		}
		return 0.0f;
	}
	else if(stateLast.m_distToClosestEnemyGroup == stateNew.m_distToClosestEnemyGroup)
	{
		if(stateNew.m_distToClosestEnemyGroup == LOW)
		{
			return 12.5f;
			//return -2.5f;
		}
		else if(stateNew.m_distToClosestEnemyGroup == MID)
		{
			return 10.5f;
			//return -0.5f;
		}
		else
		{
			return 0.0f;
		}
	}
	else
	{
		if(stateNew.m_distToClosestEnemyGroup == LOW)
		{
			return 17.5f;
			//return -7.5f;
		}
		else if(stateNew.m_distToClosestEnemyGroup == MID)
		{
			return 12.5f;
			//return -2.5f;
		}
		else
		{
			return 0.0f;
		}
	}
}


void QLearningMgr::writeQMapHumanReadable()
{
	ofstream ofs_human_readable_qmap("FerranBotData\\qmap_human_readable.txt");
	std::map<SApair, float, compare>::iterator iter = q_map->begin();
	std::map<SApair, int, compare>::iterator itercount = q_count->begin();

	if(ofs_human_readable_qmap.is_open())
	{
		Group lastDist = NA;
		while(iter != q_map->end())
		{
			ofs_human_readable_qmap << "State avgDps = " << TranslateGroupToWord(iter->first.state.m_avgDpsGroup) 
				<< " avgHealth = " << TranslateGroupToWord(iter->first.state.m_avgHealthGroup) 
				<< " dist = " << TranslateGroupToWord(iter->first.state.m_distToClosestEnemyGroup)
				<< " lastAction = " << TranslateActionToWord(iter->first.state.m_action)
				<< " action = " << TranslateActionToWord(iter->first.action);
			ofs_human_readable_qmap << " q_value = " << iter->second;
			ofs_human_readable_qmap << " visited = " << itercount->second << endl;

			if(iter->first.state.m_distToClosestEnemyGroup != lastDist)
			{
				ofs_human_readable_qmap << "\n";
				lastDist = iter->first.state.m_distToClosestEnemyGroup;
			}
			 
			iter++;
			itercount++;
		}


		ofs_human_readable_qmap << "\n\n\nNum games played = " << m_numGamesPlayed;
		ofs_human_readable_qmap << "\nNum games won = " << m_numGamesWon;

		std::vector<int>::iterator gamesWonIt = m_gamesWon->begin();
		int index = 0;
		while(gamesWonIt != m_gamesWon->end())
		{
			ofs_human_readable_qmap << "\ngame number " << index << " has been " << m_gamesWon->at(index);
			index++;
			gamesWonIt++;
		}
	}

	ofs_human_readable_qmap.close();

	//___________________________________________________

	if(q_mapUnit->empty())
	{
		return;
	}
	ofstream ofs_human_readable_qmapUnit("FerranBotData\\qmapUnit_human_readable.txt");
	std::map<USApair, float, unitCompare>::iterator iterUnitMap = q_mapUnit->begin();
	std::map<USApair, int, unitCompare>::iterator iterUnitCount = q_countUnit->begin();
	//std::map<SApair, float, compare>::iterator iter = q_mapUnit->begin();
	//std::map<SApair, int, compare>::iterator itercount = q_countUnit->begin();

	if(ofs_human_readable_qmapUnit.is_open())
	{
		Group lastDist = NA;
		while(iterUnitMap != q_mapUnit->end())
		{
			ofs_human_readable_qmapUnit << "State avgDps = " << TranslateGroupToWord(iterUnitMap->first.state.m_avgDpsGroup) 
				<< " avgHealth = " << TranslateGroupToWord(iterUnitMap->first.state.m_avgHealthGroup) 
				<< " dist = " << TranslateGroupToWord(iterUnitMap->first.state.m_distToClosestEnemyGroup) 
				<< " numUnitsInRadius = " << TranslateGroupToWord(iterUnitMap->first.state.m_numEnemyUnitsInRadius)
				<< " lastSquadAction = " << TranslateActionToWord(iterUnitMap->first.state.m_lastSquadAction)
				<< " lastUnitAction = " << TranslateActionToWord(iterUnitMap->first.state.m_lastUnitAction)

				<< " action = " << TranslateActionToWord(iterUnitMap->first.action);
			ofs_human_readable_qmapUnit << " q_value = " << iterUnitMap->second;
			ofs_human_readable_qmapUnit << " visited = " << iterUnitCount->second << endl;

			if(iterUnitMap->first.state.m_distToClosestEnemyGroup != lastDist)
			{
				ofs_human_readable_qmap << "\n";
				lastDist = iterUnitMap->first.state.m_distToClosestEnemyGroup;
			}

			iterUnitMap++;
			iterUnitCount++;
		}
	}

	ofs_human_readable_qmapUnit.close();
}

void QLearningMgr::writeQMapBinary()
{
	ofstream ofs_map("FerranBotData\\qmap.bin", ios::binary);
	if(ofs_map.is_open())
	{
		std::map<SApair, float, compare>::iterator it = q_map->begin();


		while(it != q_map->end())
		{
			ofs_map.write((char*)&it->first, sizeof(SApair));
			ofs_map.write((char*)&it->second, sizeof(float));

			it++;
		}
	}
	ofs_map.close();

	//////////////////////////////////////////////////////////////////////////

	if(q_mapUnit->empty())
	{
		return;
	}

	ofstream ofs_mapUnit("FerranBotData\\qmapUnit.bin", ios::binary);
	if(ofs_mapUnit.is_open())
	{
		std::map<USApair, float, unitCompare>::iterator it = q_mapUnit->begin();
		//std::map<SApair, float, compare>::iterator it = q_map->begin();


		while(it != q_mapUnit->end())
		{
			ofs_mapUnit.write((char*)&it->first, sizeof(USApair));
			ofs_mapUnit.write((char*)&it->second, sizeof(float));

			it++;
		}
	}
	ofs_mapUnit.close();
}

void QLearningMgr::writeQCountBinary()
{
	ofstream ofs_count("FerranBotData\\qcount.bin", ios::binary);
	if(ofs_count.is_open())
	{
		std::map<SApair, int, compare>::iterator itcount = q_count->begin();
		while(itcount != q_count->end())
		{
			ofs_count.write((char*)&itcount->first, sizeof(SApair));
			ofs_count.write((char*)&itcount->second, sizeof(int));
			itcount++;
		}
	}

	ofs_count.close();

	//////////////////////////////////////////////////////////////////////////

	if(q_countUnit->empty())
	{
		return;
	}

	ofstream ofs_countUnit("FerranBotData\\qcountUnit.bin", ios::binary);
	if(ofs_countUnit.is_open())
	{
		std::map<USApair, int, unitCompare>::iterator itcountUnit = q_countUnit->begin();
		//std::map<SApair, int, compare>::iterator itcount = q_count->begin();
		while(itcountUnit != q_countUnit->end())
		{
			ofs_countUnit.write((char*)&itcountUnit->first, sizeof(USApair));
			ofs_countUnit.write((char*)&itcountUnit->second, sizeof(int));
			itcountUnit++;
		}
	}

	ofs_countUnit.close();
}


void QLearningMgr::writeQVars()
{
	ofstream ofs_vars("FerranBotData\\qvars.bin", ios::binary);
	if(ofs_vars.is_open())
	{
		ofs_vars.write((char*)&m_exploreExploitCoef, sizeof(float));
		ofs_vars.write((char*)&m_totalNumStatesVisited, sizeof(int));

		ofs_vars.write((char*)&m_numGamesPlayed, sizeof(int));
		ofs_vars.write((char*)&m_numGamesWon, sizeof(int));

		std::vector<int>::iterator gamesWonIt = m_gamesWon->begin();
		int index = 0;
		while(gamesWonIt != m_gamesWon->end())
		{
			ofs_vars.write((char*)&m_gamesWon->at(index), sizeof(int));
			//ofs_vars.write((char*)&gamesWonIt, sizeof(int));
			gamesWonIt++;
			index++;
		}
	}
	ofs_vars.close();
}


UnitAction QLearningMgr::updateUnitQ(UnitState lastState, UnitAction lastUnitAction, Action lastSquadAction, UnitState stateNew)
{
	//DEBUG
	static bool m_forceExploration = false;
	static bool m_forceExploitation = false;
	if(m_forceExploration)
	{
		m_exploreExploitCoef = -1.0f;
	}
	else if(m_forceExploitation)
	{
		m_exploreExploitCoef = 1.0f;
	}
	//else
	//	//_DEBUG
	//{
	//	updateExploreExploitCoef();
	//}
	//m_totalNumStatesVisited++;
	//DEBUG
	static Group a = MID;
	static Group b = HIGH;
	static Group c = LOW;

	if( lastState.m_avgDpsGroup == a && lastState.m_avgHealthGroup == b &&lastState.m_distToClosestEnemyGroup == c)
	{
		int a = 0;
	}

	//_DEBUG
	USApair usapair = USApair(lastState, lastUnitAction);
	UnitAction outNewAction = UNIT_ATTACK;

	if(q_mapUnit->find(usapair) != q_mapUnit->end())
	{
		float optimalFutureValue = getOptimalFutureValueForUnit(stateNew, outNewAction);
		float currentQValue = q_mapUnit->at(usapair);

		float alpha = getAlphaForUnit(usapair);
		float reward = getRewardForUnit(usapair, stateNew);
		currentQValue = currentQValue + alpha * (reward + GAMMA * optimalFutureValue - currentQValue); 

		std::pair<std::map<USApair, float, unitCompare>::iterator,bool> ret;
		std::map<USApair, int, unitCompare>::iterator it = q_countUnit->find(usapair);
		q_mapUnit->at(usapair) = currentQValue;
		q_countUnit->at(usapair) = it->second + 1;
	}
	else
	{
		q_mapUnit->insert(std::pair<USApair, float>(usapair, 0.0f));
		q_countUnit->insert(std::pair<USApair, int>(usapair, 0));
		outNewAction = SQUAD_ACTION;
	}

	return outNewAction;
}

float QLearningMgr::getAlphaForUnit(USApair usapair)
{
	return 0.1f; //In stochastic environments, a constant learning rate is usually used
}



float QLearningMgr::getRewardForUnit(USApair usapair, UnitState stateNew)
{
	if(m_forceUnitReward != 0)
	{
		return m_forceUnitReward;
	}

	UnitAction lastAction = usapair.action;
	switch(lastAction)
	{
		case SQUAD_ACTION:
		{
			return getRewardForUnitSquadAction(usapair, stateNew);
		}
		break;

		case UNIT_ATTACK:
		{
			return getRewardForUnitAttack(usapair, stateNew);
		}
		break;

		case UNIT_HOLD:
		{
			return getRewardForUnitHold(usapair, stateNew);
		}
		break;

		case UNIT_FLEE:
		{
			return getRewardForUnitFlee(usapair, stateNew);
		}
		break;

		default:
		{
			return 0.0f;
		}
	}
}

//float QLearningMgr::getRewardForUnit(USApair usapair, UnitState stateNew)
//{
//	if(m_forceUnitReward != 0)
//	{
//		float reward = m_forceUnitReward;
//		m_forceUnitReward = 0;
//		return reward;
//	}
//
//	float reward = 0.0f;
//
//	//penalize changing action
//	if(usapair.state.m_lastUnitAction != stateNew.m_lastUnitAction || stateNew.m_lastUnitAction != SQUAD_ACTION)
//	{
//		reward -= 2.0f;
//	}
//
//	UnitState stateLast = usapair.state;
//	if(stateLast.m_avgHealthGroup < stateNew.m_avgHealthGroup) //Enemy has less life than before
//	{
//		if(stateLast.m_avgDpsXHealthGroup < stateNew.m_avgDpsXHealthGroup) //Enemy has less DPS than before
//		{
//			reward += 20.0f;
//		}
//		else
//		{
//			reward += 10.0f;
//		}
//	}
//	else if(stateLast.m_avgHealthGroup > stateNew.m_avgHealthGroup) //We have less life than before
//	{
//		if(stateLast.m_avgDpsXHealthGroup > stateNew.m_avgDpsXHealthGroup) //We has less DPS than before
//		{
//			reward -= 20.0f;
//		}
//		else
//		{
//			reward -= 10.0f;
//		}
//	}
//	else if(stateLast.m_avgHealthGroup			== stateNew.m_avgHealthGroup			&& stateLast.m_avgHealthGroup == LOW
//		&& stateLast.m_avgDpsXHealthGroup		== stateNew.m_avgDpsXHealthGroup		&& stateLast.m_avgDpsXHealthGroup == LOW )
//	{
//		//Low health and low dps => try to run away!
//		float rew = RunAwayForUnit(stateLast, stateNew);
//		reward += rew;
//	}
//	else if(stateLast.m_hitPoints <= stateNew.m_hitPoints) //we are not losing life
//	{
//		if(stateLast.m_enemyHitPoints >= stateNew.m_enemyHitPoints) //enemy is losing life
//		{
//			reward += 5.0f;
//		}
//		else
//		{
//			//Nothing
//		}
//	}
//	else if(stateLast.m_hitPoints > stateNew.m_hitPoints) //we are losing life
//	{
//		if(stateLast.m_enemyHitPoints >= stateNew.m_enemyHitPoints) //enemy is losing life
//		{
//			if(stateNew.m_avgHealthGroup >= MID) //we can afford to lose life
//			{
//				reward += 3.0f;
//			}
//			else
//			{
//				reward -= 3.0f;
//			}
//		}
//		else
//		{
//			//Nothing
//		}
//	}
//	else
//	{
//		//Nothing
//	}
//
//	return reward;
//}

string QLearningMgr::TranslateGroupToWord(Group group)
{
	if(group == LOW)
	{
		return "LOW ";
	}
	else if(group == MID)
	{
		return "MID ";
	}
	else if(group == HIGH)
	{
		return "HIGH";
	}
	else
	{
		return "NA  ";
	}
}

string QLearningMgr::TranslateActionToWord(int action)
{
	if(action == ATTACK)
	{
		return "ATTACK         ";
	}
	else if(action == HOLD)
	{
		return "HOLD           ";
	}
	else if(action == ATTACK_SURROUND)
	{
		return "ATTACK_SURROUND";
	}
	else if(action == ATTACK_HALF_SURROUND)
	{
		return "ATTACK_HALF    ";
	}
	else if(action == COUNT)
	{
		return "COUNT          ";
	}
	else if(action == SQUAD_ACTION)
	{
		return "SQUAD_ACTION   ";
	}
	else if(action == UNIT_ATTACK)
	{
		return "UNIT_ATTACK    ";
	}
	else if(action == UNIT_HOLD)
	{
		return "UNIT_HOLD      ";
	}
	else if(action == UNIT_FLEE)
	{
		return "UNIT_FLEE      ";
	}
	else
	{
		return "UNIT_COUNT     ";
	}
}


float QLearningMgr::getRewardForUnitSquadAction(USApair usapair, UnitState stateNew) const
{
	float reward = 0.0f;

	if(stateNew.m_avgHealthGroup == LOW)
	{
		reward += -10.0f; //If health is low, don't follow squad orders
	}
	else if(stateNew.m_distToClosestEnemyGroup == LOW)
	{
		reward += -3.0f; //If there's an enemy very close, disregard squad orders
	}
	else
	{
		reward += 20.0f; //Else, always follow squad orders
	}


	if(stateNew.m_numEnemyUnitsInRadius == HIGH)
	{
		reward -= 2.0f;
	}
	else if(stateNew.m_numEnemyUnitsInRadius == MID)
	{
		reward -= 1.0f;
	}
	else if(stateNew.m_numEnemyUnitsInRadius == LOW)
	{
		reward += 1.0f;
	}
	else if(stateNew.m_numEnemyUnitsInRadius == NA)
	{
		reward += 1.0f;
	}


	return reward;
}

float QLearningMgr::getRewardForUnitAttack(USApair usapair, UnitState stateNew) const
{
	UnitState lastState = usapair.state;
	float reward = 0.0f;

	if(lastState.m_lastUnitAction != UNIT_ATTACK) //Penalize if we've changed action
	{
		reward += -2.0f;
	}

	//_____________________

	if(stateNew.m_avgDpsGroup == HIGH)
	{
		if(stateNew.m_avgHealthGroup == HIGH)
		{
			if(stateNew.m_distToClosestEnemyGroup == HIGH)
			{
				//0
			}
			else if(stateNew.m_distToClosestEnemyGroup == MID)
			{
				reward += 2.0f;
			}
			else if(stateNew.m_distToClosestEnemyGroup == LOW)
			{
				reward += 5.0f; //A close enemy and high probabilities to win
			}
		}
		else if(stateNew.m_avgHealthGroup == MID)
		{
			//Probabilities to win are more or less high
			if(stateNew.m_distToClosestEnemyGroup == HIGH)
			{
				reward += -2.0f; //Enforce following squad action
			}
			else if(stateNew.m_distToClosestEnemyGroup == MID)
			{
				reward += -1.0f;
			}
			else if(stateNew.m_distToClosestEnemyGroup == LOW)
			{
				//0
			}
		}
		else if(stateNew.m_avgHealthGroup == LOW)
		{
			//Probabilities to win are low
			if(stateNew.m_distToClosestEnemyGroup == HIGH)
			{
				reward += -3.0f; //Enforce following squad action
			}
			else if(stateNew.m_distToClosestEnemyGroup == MID)
			{
				reward += -4.0f;
			}
			else if(stateNew.m_distToClosestEnemyGroup == LOW)
			{
				reward += -5.0f;
			}
		}
	}
	else if(stateNew.m_avgDpsGroup == MID)
	{
		if(stateNew.m_avgHealthGroup == HIGH)
		{
			if(stateNew.m_distToClosestEnemyGroup == HIGH)
			{
				reward += -2.0f;
			}
			else if(stateNew.m_distToClosestEnemyGroup == MID)
			{
				reward += -1.0f;
			}
			else if(stateNew.m_distToClosestEnemyGroup == LOW)
			{
				reward += 1.0f;
			}
		}
		else if(stateNew.m_avgHealthGroup == MID)
		{
			if(stateNew.m_distToClosestEnemyGroup == HIGH)
			{
				reward += -4.0f; //Enforce following squad action
			}
			else if(stateNew.m_distToClosestEnemyGroup == MID)
			{
				reward += -2.0f;
			}
			else if(stateNew.m_distToClosestEnemyGroup == LOW)
			{
				//0
			}
		}
		else if(stateNew.m_avgHealthGroup == LOW)
		{
			if(stateNew.m_distToClosestEnemyGroup == HIGH)
			{
				reward += -6.0f; //Enforce following squad action
			}
			else if(stateNew.m_distToClosestEnemyGroup == MID)
			{
				reward += -5.0f;
			}
			else if(stateNew.m_distToClosestEnemyGroup == LOW)
			{
				reward += -4.0f;
			}
		}
	}
	else if(stateNew.m_avgDpsGroup == LOW)
	{
		if(stateNew.m_avgHealthGroup == HIGH)
		{
			if(stateNew.m_distToClosestEnemyGroup == HIGH)
			{
				reward += -4.0f;
			}
			else if(stateNew.m_distToClosestEnemyGroup == MID)
			{
				reward += -3.0f;
			}
			else if(stateNew.m_distToClosestEnemyGroup == LOW)
			{
				reward += -2.0f;
			}
		}
		else if(stateNew.m_avgHealthGroup == MID)
		{
			if(stateNew.m_distToClosestEnemyGroup == HIGH)
			{
				reward += -6.0f; //Enforce following squad action
			}
			else if(stateNew.m_distToClosestEnemyGroup == MID)
			{
				reward += -4.0f;
			}
			else if(stateNew.m_distToClosestEnemyGroup == LOW)
			{
				reward += -2.0f;
			}
		}
		else if(stateNew.m_avgHealthGroup == LOW)
		{
			if(stateNew.m_distToClosestEnemyGroup == HIGH)
			{
				reward += -8.0f; //Enforce following squad action
			}
			else if(stateNew.m_distToClosestEnemyGroup == MID)
			{
				reward += -7.0f;
			}
			else if(stateNew.m_distToClosestEnemyGroup == LOW)
			{
				reward += -4.0f;
			}
		}
	}
	
	if(stateNew.m_numEnemyUnitsInRadius == HIGH)
	{
		reward -= 2.0f;
	}
	else if(stateNew.m_numEnemyUnitsInRadius == MID)
	{
		reward -= 1.0f;
	}
	else if(stateNew.m_numEnemyUnitsInRadius == LOW)
	{
		reward += 1.0f;
	}
	else if(stateNew.m_numEnemyUnitsInRadius == NA)
	{
		reward += 1.0f;
	}

	return reward;
}

float QLearningMgr::getRewardForUnitHold(USApair usapair, UnitState stateNew) const
{
	UnitState lastState = usapair.state;
	float reward = 0.0f;

	if(lastState.m_lastUnitAction != UNIT_HOLD) //Penalize if we've changed action
	{
		reward += -2.0f;
	}

	//_____________________

	if(lastState.m_avgHealthGroup < stateNew.m_avgHealthGroup) //Life balance is worse
	{
		reward += -10.0f;
	}
	else if(lastState.m_avgHealthGroup > stateNew.m_avgHealthGroup) //Life balance is better
	{
		reward += 10.0f;
	}

	//_____________________
	
	if(lastState.m_avgDpsGroup < stateNew.m_avgDpsGroup) //AvgDps balance is worse
	{
		reward += -10.0f;
	}
	else if(lastState.m_avgDpsGroup > stateNew.m_avgDpsGroup) //AvgDps balance is better
	{
		reward += 10.0f;
	}

	//_____________________

	if(lastState.m_distToClosestEnemyGroup > stateNew.m_distToClosestEnemyGroup) //Units are getting closer
	{
		if(stateNew.m_distToClosestEnemyGroup == MID)
		{
			if(stateNew.m_avgDpsGroup == HIGH) //If we are more much more powerful than the enemy
			{
				reward += 2.0f;
			}
			else if(stateNew.m_avgDpsGroup == MID) //If we are a bit more powerful than the enemy
			{
				//0
			}
			else if(stateNew.m_avgDpsGroup == LOW) //If we are less powerful than the enemy
			{
				reward += -1.0f;
			}
		}
		else if(stateNew.m_distToClosestEnemyGroup == LOW)
		{
			if(stateNew.m_avgDpsGroup == HIGH) //If we are more much more powerful than the enemy
			{
				reward += 1.0f;
			}
			else if(stateNew.m_avgDpsGroup == MID) //If we are a bit more powerful than the enemy
			{
				reward += -1.0f;
			}
			else if(stateNew.m_avgDpsGroup == LOW) //If we are less powerful than the enemy
			{
				reward += -10.0f;
			}
		}
	}

	if(stateNew.m_numEnemyUnitsInRadius == HIGH)
	{
		reward -= 2.0f;
	}
	else if(stateNew.m_numEnemyUnitsInRadius == MID)
	{
		reward -= 1.0f;
	}
	else if(stateNew.m_numEnemyUnitsInRadius == LOW)
	{
		reward += 1.0f;
	}
	else if(stateNew.m_numEnemyUnitsInRadius == NA)
	{
		reward -= 2.0f;
	}

	return reward;
}

float QLearningMgr::getRewardForUnitFlee(USApair usapair, UnitState stateNew) const
{
	UnitState lastState = usapair.state;
	float reward = 0.0f;

	if(lastState.m_lastUnitAction != UNIT_FLEE) //Penalize if we've changed action
	{
		reward += -2.0f;
	}

	//_____________________

	if(stateNew.m_avgHealthGroup == LOW) //Life balance is bad
	{
		if(lastState.m_avgHealthGroup == stateNew.m_avgHealthGroup 
		|| lastState.m_avgHealthGroup > stateNew.m_avgHealthGroup) //Life balance is the same or worse
		{
			if(stateNew.m_avgDpsGroup == HIGH)
			{
				reward += -5.0f; //Don't flee, we can win this
			}
			else if(stateNew.m_avgDpsGroup == MID)
			{
				reward += 5.0f; //Things are not looking good. Go away!
			}
			else if(stateNew.m_avgDpsGroup == LOW)
			{
				reward += 10.0f; //Run for your lives!
			}
		}
	}
	else if(stateNew.m_avgHealthGroup == MID) 
	{
		if(stateNew.m_avgDpsGroup == MID
		|| stateNew.m_avgDpsGroup == HIGH)
		{
			reward += -2.0f; //Don't flee
		}
		else if(stateNew.m_avgDpsGroup == LOW)
		{
			reward += 5.0f;
		}
	}
	else if(stateNew.m_avgHealthGroup == HIGH) 
	{
		if(stateNew.m_avgDpsGroup == LOW)
		{
			reward += 2.0f;
		}
		else
		{
			reward += -7.0f; //Fleeing is for cowards!
		}
	}

	if(reward > 0 && lastState.m_distToClosestEnemyGroup < stateNew.m_distToClosestEnemyGroup)
	{
		//If fleeing is a good option (ie, reward > 0) and we are effectively getting away, increase reward
		reward += 1.0f;
	}
	else if(reward > 0 && lastState.m_distToClosestEnemyGroup > stateNew.m_distToClosestEnemyGroup)
	{
		reward += -1.0f;
	}
	else if(reward < 0 && lastState.m_distToClosestEnemyGroup < stateNew.m_distToClosestEnemyGroup)
	{
		//If fleeing is NOT a good option and we are getting away, decrease reward!
		reward += -1.0f;
	}


	if(stateNew.m_numEnemyUnitsInRadius == HIGH)
	{
		reward += 2.0f;
	}
	else if(stateNew.m_numEnemyUnitsInRadius == MID)
	{
		reward += 1.0f;
	}
	else if(stateNew.m_numEnemyUnitsInRadius == LOW)
	{
		//0
	}
	else if(stateNew.m_numEnemyUnitsInRadius == NA)
	{
		reward -= 3.0f;
	}

	return reward;
}

void QLearningMgr::resetForcedReward()
{
	m_forceReward = 0;
}

void QLearningMgr::resetForcedUnitReward()
{
	m_forceUnitReward = 0;
}