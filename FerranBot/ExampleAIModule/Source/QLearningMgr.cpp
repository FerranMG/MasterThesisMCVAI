
#include "QLearningMgr.h"
#include <fstream>
#include <iostream>
#include <sys\stat.h>
#include <limits>

using namespace BWAPI;
using namespace std;


const float GAMMA = 0.9f;

QLearningMgr* QLearningMgr::instance = nullptr;
const int QLearningMgr::MAX_STATES_VISITED_BEFORE_TOTAL_EXPLOIT = 1000; //TODO - CHANGE THIS
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

USApair::USApair(UnitState stateNew, Action actionNew)
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
}


QLearningMgr::~QLearningMgr()
{
	delete q_map;
	delete q_count;
	delete q_mapUnit;
	delete q_countUnit;

	q_map = nullptr;
	q_count = nullptr;
	q_mapUnit = nullptr;
	q_countUnit = nullptr;

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
	static bool m_forceExploration = true;
	if(m_forceExploration)
	{
		m_exploreExploitCoef = -1.0f;
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

	if( lastState.m_avgDpsXHealthGroup == a && lastState.m_avgHealthGroup == b &&lastState.m_distToClosestEnemyGroup == c)
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
		 float reward = m_forceReward;
		 m_forceReward = 0;
		 return reward;
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
		if(stateLast.m_avgDpsXHealthGroup < stateNew.m_avgDpsXHealthGroup) //Enemy has less DPS than before
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
		if(stateLast.m_avgDpsXHealthGroup > stateNew.m_avgDpsXHealthGroup) //We has less DPS than before
		{
			reward -= 20.0f;
		}
		else
		{
			reward -= 10.0f;
		}
	}
	else if(stateLast.m_avgHealthGroup			== stateNew.m_avgHealthGroup			&& stateLast.m_avgHealthGroup == LOW
		 && stateLast.m_avgDpsXHealthGroup		== stateNew.m_avgDpsXHealthGroup		&& stateLast.m_avgDpsXHealthGroup == LOW )
	{
		//Low health and low dps => try to run away!
		float rew = RunAway(stateLast, stateNew);
		reward -= rew;
	}
	else if(stateLast.m_hitPoints <= stateNew.m_hitPoints) //we are not losing life
	{
		if(stateLast.m_enemyHitPoints >= stateNew.m_enemyHitPoints) //enemy is losing life
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
	//TODO
	

	float exploreExploitRand = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 1;

	float exploreExploitCoef = getExploreExploitCoefficient();
	if(exploreExploitCoef > exploreExploitRand) //Exploit
	{
		float maxValue = std::numeric_limits<float>::lowest();//-1000000.0f;
		
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
		//float minValue = 1000000.0f;
		float qValue = 0.0f;
		bool stateFound = false;
		int numTimesStateVisited = 100000;


			//stateFound = true;

			//float currentQValue = it->second;
			//if(currentQValue < minValue)
			//{
			//	minValue = currentQValue;
			//	outNewAction = (Action)act;
			//}

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
			//return minValue;
		}
		else
		{
			outNewAction = (Action)0;
			return 0.0f;
		}
	}
}



float QLearningMgr::getOptimalFutureValueForUnit(UnitState state, Action& outNewAction)
{
	//TODO
	float exploreExploitRand = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX) - 1;

	float exploreExploitCoef = getExploreExploitCoefficient();
	if(exploreExploitCoef > exploreExploitRand) //Exploit
	{
		float maxValue = std::numeric_limits<float>::lowest();//-1000000.0f;

		bool stateFound = false;

		for(int act = 0; act < COUNT; act++)
		{
			USApair usapair = USApair(state, (Action)act);

			map<USApair, float, unitCompare>::iterator it = q_mapUnit->find(usapair);
			if(it != q_mapUnit->end())
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
			USApair usapair = USApair(state, (Action)act);

			map<USApair, float, unitCompare>::iterator it = q_mapUnit->find(usapair);
			if(it != q_mapUnit->end())
			{
				map<USApair, int, unitCompare>::iterator it_count = q_countUnit->find(usapair);
				if(it_count != q_countUnit->end())
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
			//return minValue;
		}
		else
		{
			outNewAction = (Action)0;
			return 0.0f;
		}
	}
}


Action QLearningMgr::getAction(State state)
{
	//for(int act = 0; act < COUNT; act++)
	//{
	//	SApair sapair;
	//	sapair.action = (Action)act;
	//	sapair.state = state;

	//	map<SApair, int, compare>::iterator it = q_count->find(sapair);

	//	if(it != q_count->end())
	//	{
	//		//Explore each state action a total of 5 times
	//		//TODO - find a better way to do this
	//		if(it->second < 5)
	//		{
	//			it->second++;
	//			return (Action)act;
	//		}
	//	}
	//	else
	//	{
	//		return (Action)act;
	//	}
	//}
	//return (Action)0;

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
	stat("qmap.bin", &results);

	//std::map<SApair, float, compare>* readMap = new std::map<SApair, float, compare>;
	ifstream ifs_map("qmap.bin", ios::binary);
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



	stat("qcount.bin", &results);

	//std::map<SApair, int, compare>* readCount = new std::map<SApair, int, compare>;
	ifstream ifs_count("qcount.bin", ios::binary);

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


	stat("qvars.bin", &results);

	ifstream ifs_vars("qvars.bin", ios::binary);

	if(!ifs_vars.fail())
	{
		ifs_count.seekg(0, ios::beg);
		//if(ifs_count.eof() == false)
		{
			ifs_vars.read((char*)&m_exploreExploitCoef, sizeof(float));
			ifs_vars.read((char*)&m_totalNumStatesVisited, sizeof(int));
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
	if(getTotalNumStatesVisited() < MAX_STATES_VISITED_BEFORE_TOTAL_EXPLOIT)
	{
		m_exploreExploitCoef = (totalExplore * (MAX_STATES_VISITED_BEFORE_TOTAL_EXPLOIT - getTotalNumStatesVisited()) + totalExploit * getTotalNumStatesVisited()) / MAX_STATES_VISITED_BEFORE_TOTAL_EXPLOIT;
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


void QLearningMgr::writeQMapHumanReadable()
{
	ofstream ofs_human_readable_qmap("qmap_human_readable.txt");
	std::map<SApair, float, compare>::iterator iter = q_map->begin();
	std::map<SApair, int, compare>::iterator itercount = q_count->begin();

	if(ofs_human_readable_qmap.is_open())
	{
		while(iter != q_map->end())
		{
			ofs_human_readable_qmap << "State dpsXHealth = " << iter->first.state.m_avgDpsXHealthGroup << " avgHealth =" 
				<< iter->first.state.m_avgHealthGroup << " dist = " << iter->first.state.m_distToClosestEnemyGroup << " action = " << iter->first.action;
			ofs_human_readable_qmap << " q_value = " << iter->second;
			ofs_human_readable_qmap << " visited = " << itercount->second << endl;

			iter++;
			itercount++;
		}
	}

	ofs_human_readable_qmap.close();

	//___________________________________________________

	if(q_mapUnit->empty())
	{
		return;
	}
	ofstream ofs_human_readable_qmapUnit("qmapUnit_human_readable.txt");
	std::map<USApair, float, unitCompare>::iterator iterUnitMap = q_mapUnit->begin();
	std::map<USApair, int, unitCompare>::iterator iterUnitCount = q_countUnit->begin();
	//std::map<SApair, float, compare>::iterator iter = q_mapUnit->begin();
	//std::map<SApair, int, compare>::iterator itercount = q_countUnit->begin();

	if(ofs_human_readable_qmapUnit.is_open())
	{
		while(iterUnitMap != q_mapUnit->end())
		{
			ofs_human_readable_qmapUnit << "State dpsXHealth = " << iterUnitMap->first.state.m_avgDpsXHealthGroup << " avgHealth =" 
				<< iterUnitMap->first.state.m_avgHealthGroup << " dist = " << iterUnitMap->first.state.m_distToClosestEnemyGroup << " action = " << iterUnitMap->first.action;
			ofs_human_readable_qmapUnit << " q_value = " << iterUnitMap->second;
			ofs_human_readable_qmapUnit << " visited = " << iterUnitCount->second << endl;

			iterUnitMap++;
			iterUnitCount++;
		}
	}

	ofs_human_readable_qmapUnit.close();
}

void QLearningMgr::writeQMapBinary()
{
	ofstream ofs_map("qmap.bin", ios::binary);
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

	ofstream ofs_mapUnit("qmapUnit.bin", ios::binary);
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
	ofstream ofs_count("qcount.bin", ios::binary);
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

	ofstream ofs_countUnit("qcountUnit.bin", ios::binary);
	if(ofs_countUnit.is_open())
	{
		std::map<USApair, int, unitCompare>::iterator itcountUnit = q_countUnit->begin();
		//std::map<SApair, int, compare>::iterator itcount = q_count->begin();
		while(itcountUnit != q_countUnit->end())
		{
			ofs_count.write((char*)&itcountUnit->first, sizeof(USApair));
			ofs_count.write((char*)&itcountUnit->second, sizeof(int));
			itcountUnit++;
		}
	}

	ofs_countUnit.close();
}


void QLearningMgr::writeQVars()
{
	ofstream ofs_vars("qvars.bin", ios::binary);
	if(ofs_vars.is_open())
	{
		ofs_vars.write((char*)&m_exploreExploitCoef, sizeof(float));
		ofs_vars.write((char*)&m_totalNumStatesVisited, sizeof(int));
	}
	ofs_vars.close();
}


Action QLearningMgr::updateUnitQ(UnitState lastState, Action lastUnitAction, Action lastSquadAction, UnitState stateNew)
{
	//DEBUG
	static bool m_forceExploration = true;
	if(m_forceExploration)
	{
		m_exploreExploitCoef = -1.0f;
	}
	//else
	//	//_DEBUG
	//{
	//	updateExploreExploitCoef();
	//}
	//m_totalNumStatesVisited++;
	//DEBUG
	static Group a = LOW;
	static Group b = LOW;
	static Group c = LOW;

	if( lastState.m_avgDpsXHealthGroup == a && lastState.m_avgHealthGroup == b &&lastState.m_distToClosestEnemyGroup == c)
	{
		int a = 0;
	}

	//_DEBUG
	USApair usapair = USApair(lastState, lastUnitAction);
	Action outNewAction = ATTACK;

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
		outNewAction = (Action)0;
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
		float reward = m_forceUnitReward;
		m_forceUnitReward = 0;
		return reward;
	}

	float reward = 0.0f;

	//penalize changing action
	if(usapair.state.m_unitAction != stateNew.m_unitAction || stateNew.m_unitAction != stateNew.m_squadAction)
	{
		reward -= 2.0f;
	}

	UnitState stateLast = usapair.state;
	if(stateLast.m_avgHealthGroup < stateNew.m_avgHealthGroup) //Enemy has less life than before
	{
		if(stateLast.m_avgDpsXHealthGroup < stateNew.m_avgDpsXHealthGroup) //Enemy has less DPS than before
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
		if(stateLast.m_avgDpsXHealthGroup > stateNew.m_avgDpsXHealthGroup) //We has less DPS than before
		{
			reward -= 20.0f;
		}
		else
		{
			reward -= 10.0f;
		}
	}
	else if(stateLast.m_avgHealthGroup			== stateNew.m_avgHealthGroup			&& stateLast.m_avgHealthGroup == LOW
		&& stateLast.m_avgDpsXHealthGroup		== stateNew.m_avgDpsXHealthGroup		&& stateLast.m_avgDpsXHealthGroup == LOW )
	{
		//Low health and low dps => try to run away!
		float rew = RunAwayForUnit(stateLast, stateNew);
		reward -= rew;
	}
	else if(stateLast.m_hitPoints <= stateNew.m_hitPoints) //we are not losing life
	{
		if(stateLast.m_enemyHitPoints >= stateNew.m_enemyHitPoints) //enemy is losing life
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
