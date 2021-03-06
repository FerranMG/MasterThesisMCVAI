#pragma once
#include <vector>
#include <map>
#include <BWAPI.h>
#include "Common.h"
#include "Defines.h"

#pragma optimize( "", off )

typedef struct SApair
{
	SApair();
	SApair(State state, Action action);

	State state;
	Action action;

	//bool operator<( const SApair& n) const
	//{
	//	if (this->state.m_avgHealthGroup < n.state.m_avgHealthGroup)
	//		return true;
	//	if (this->state.m_avgDpsGroup < n.state.m_avgDpsGroup)
	//		return true;
	//	if (this->state.m_distToClosestEnemyGroup < n.state.m_distToClosestEnemyGroup)
	//		return true;
	//	if (this->action < n.action)
	//		return true;
	//	return false;
	//}


} SApair;


typedef struct USApair
{
	USApair();
	USApair(UnitState state, UnitAction action);

	UnitState state;
	UnitAction action;

	//bool operator<( const USApair& n) const
	//{
	//	if (this->state.m_avgHealthGroup < n.state.m_avgHealthGroup)
	//		return true;
	//	if (this->state.m_avgDpsGroup < n.state.m_avgDpsGroup)
	//		return true;
	//	if (this->state.m_distToClosestEnemyGroup < n.state.m_distToClosestEnemyGroup)
	//		return true;
	//	if (this->state.m_numEnemyUnitsInRadius < n.state.m_numEnemyUnitsInRadius)
	//		return true;
	//	if (this->action < n.action)
	//		return true;
	//	return false;
	//}


} USApair;


class compare
{
public:
	bool operator()(const SApair sapair1,const SApair sapair2) const
	{
		if(sapair1.state.m_avgHealthGroup < sapair2.state.m_avgHealthGroup)
		{
			return true;
		}
		else if(sapair1.state.m_avgHealthGroup == sapair2.state.m_avgHealthGroup
			    && sapair1.state.m_avgDpsGroup < sapair2.state.m_avgDpsGroup)
		{
			return true;
		}
		else if(sapair1.state.m_avgHealthGroup == sapair2.state.m_avgHealthGroup
				&& sapair1.state.m_avgDpsGroup == sapair2.state.m_avgDpsGroup
				&& sapair1.state.m_distToClosestEnemyGroup < sapair2.state.m_distToClosestEnemyGroup)
		{
			return true;
		}
		else if(sapair1.state.m_avgHealthGroup == sapair2.state.m_avgHealthGroup
			&& sapair1.state.m_avgDpsGroup == sapair2.state.m_avgDpsGroup
			&& sapair1.state.m_distToClosestEnemyGroup == sapair2.state.m_distToClosestEnemyGroup
			&& sapair1.state.m_action < sapair2.state.m_action)
		{
			return true;
		}
		else if(sapair1.state.m_avgHealthGroup == sapair2.state.m_avgHealthGroup
			&& sapair1.state.m_avgDpsGroup == sapair2.state.m_avgDpsGroup
			&& sapair1.state.m_distToClosestEnemyGroup == sapair2.state.m_distToClosestEnemyGroup
			&& sapair1.state.m_action == sapair2.state.m_action
			&& sapair1.action < sapair2.action)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};



class unitCompare
{
public:
	bool operator()(const USApair sapair1,const USApair sapair2) const
	{
		if(sapair1.state.m_avgHealthGroup < sapair2.state.m_avgHealthGroup)
		{
			return true;
		}
		else if(sapair1.state.m_avgHealthGroup == sapair2.state.m_avgHealthGroup
			&& sapair1.state.m_avgDpsGroup < sapair2.state.m_avgDpsGroup)
		{
			return true;
		}
		else if(sapair1.state.m_avgHealthGroup == sapair2.state.m_avgHealthGroup
			&& sapair1.state.m_avgDpsGroup == sapair2.state.m_avgDpsGroup
			&& sapair1.state.m_distToClosestEnemyGroup < sapair2.state.m_distToClosestEnemyGroup)
		{
			return true;
		}
		else if(sapair1.state.m_avgHealthGroup == sapair2.state.m_avgHealthGroup
			&& sapair1.state.m_avgDpsGroup == sapair2.state.m_avgDpsGroup
			&& sapair1.state.m_distToClosestEnemyGroup == sapair2.state.m_distToClosestEnemyGroup
			&& sapair1.state.m_numEnemyUnitsInRadius < sapair2.state.m_numEnemyUnitsInRadius)
		{
			return true;
		}
		else if(sapair1.state.m_avgHealthGroup == sapair2.state.m_avgHealthGroup
			&& sapair1.state.m_avgDpsGroup == sapair2.state.m_avgDpsGroup
			&& sapair1.state.m_distToClosestEnemyGroup == sapair2.state.m_distToClosestEnemyGroup
			&& sapair1.state.m_numEnemyUnitsInRadius == sapair2.state.m_numEnemyUnitsInRadius
			&& sapair1.state.m_lastSquadAction < sapair2.state.m_lastSquadAction)
		{
			return true;
		}
		else if(sapair1.state.m_avgHealthGroup == sapair2.state.m_avgHealthGroup
			&& sapair1.state.m_avgDpsGroup == sapair2.state.m_avgDpsGroup
			&& sapair1.state.m_distToClosestEnemyGroup == sapair2.state.m_distToClosestEnemyGroup
			&& sapair1.state.m_numEnemyUnitsInRadius == sapair2.state.m_numEnemyUnitsInRadius
			&& sapair1.state.m_lastSquadAction == sapair2.state.m_lastSquadAction
			&& sapair1.state.m_lastUnitAction < sapair2.state.m_lastUnitAction)
		{
			return true;
		}
		else if(sapair1.state.m_avgHealthGroup == sapair2.state.m_avgHealthGroup
			&& sapair1.state.m_avgDpsGroup == sapair2.state.m_avgDpsGroup
			&& sapair1.state.m_distToClosestEnemyGroup == sapair2.state.m_distToClosestEnemyGroup
			&& sapair1.state.m_numEnemyUnitsInRadius == sapair2.state.m_numEnemyUnitsInRadius
			&& sapair1.state.m_lastSquadAction == sapair2.state.m_lastSquadAction
			&& sapair1.state.m_lastUnitAction == sapair2.state.m_lastUnitAction
			&& sapair1.action < sapair2.action)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
};


class QLearningMgr
{
public:
	static QLearningMgr* getInstance();
	~QLearningMgr();

	Action updateSquadQ(State lastState, Action lastAction, State stateNew);
	UnitAction updateUnitQ(UnitState lastState, UnitAction lastUnitAction, Action lastSquadAction, UnitState stateNew);
	float getAlpha(SApair sapair);
	float getReward(SApair sapair, State stateNew);
	float getOptimalFutureValue(State state, Action& outNewAction);
	Action getAction(State state);
	float getExploreExploitCoefficient();

	void writeToStream();
	void readFromStream();

	std::map<SApair, float, compare>* q_map;
	std::map<SApair, int, compare>*	 q_count;

	std::map<USApair, float, unitCompare>*	q_mapUnit;
	std::map<USApair, int, unitCompare>*		q_countUnit;
	//std::map<USApair, float, compare>*	 q_mapUnit;
	//std::map<USApair, int, compare>*	 q_countUnit;

	void forceReward(float reward) {m_forceReward = reward;}
	void forceUnitReward(float reward) {m_forceUnitReward = reward;}
	int getTotalNumStatesVisited();
	void resetForcedReward();
	void resetForcedUnitReward();

	std::string TranslateGroupToWord(Group group);
	std::string TranslateActionToWord(int action);

	static const int MAX_STATES_VISITED_BEFORE_TOTAL_EXPLOIT;
	static const int MAX_NUM_GAMES_PLAYED_BEFORE_TOTAL_EXPLOIT;
	int m_numGamesPlayed;
	int m_numGamesWon;
	std::vector<int>* m_gamesWon;

private:
	static QLearningMgr* instance;
	QLearningMgr();
	void updateExploreExploitCoef();
	float RunAway(const State& stateLast, const State& stateNew);
	static float m_exploreExploitCoef;
	static int m_totalNumStatesVisited;
	float m_forceReward;
	float m_forceUnitReward;

	void writeQMapHumanReadable();
	void writeQMapBinary();
	void writeQCountBinary();
	void writeQVars();
	float getOptimalFutureValueForUnit(UnitState state, UnitAction& outNewAction);
	float getAlphaForUnit(USApair usapair);
	float getRewardForUnit(USApair usapair, UnitState stateNew);
	float RunAwayForUnit(const UnitState& stateLast, const UnitState& stateNew);


	float getRewardForUnitSquadAction(USApair usapair, UnitState stateNew) const;
	float getRewardForUnitAttack(USApair usapair, UnitState stateNew) const;
	float getRewardForUnitHold(USApair usapair, UnitState stateNew) const;
	float getRewardForUnitFlee(USApair usapair, UnitState stateNew) const;
};

#pragma optimize( "", on )