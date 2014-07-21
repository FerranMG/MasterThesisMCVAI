#pragma once
#include <vector>
#include <BWAPI.h>

typedef enum x{ATTACK, HOLD, FLEE} Action;

struct State
{
	int groundEnemiesN;
	int groundEnemiesE;
	int groundEnemiesS;
	int groundEnemiesW;
};

typedef struct SApair
{
	State state;
	Action action;
} SApair;

class QLearningMgr
{
public:
	static QLearningMgr* getInstance();
	~QLearningMgr();

	void hola();

private:
	static QLearningMgr* instance;
	QLearningMgr();

};


//class QLearningMgr
//{
//public:
//	static QLearningMgr* getInstance();
//	~QLearningMgr();
//
//	//void updateQ(SApair sapair, int reward);
//
//	//State getCurrentState(BWAPI::Unitset::iterator u);
//	void updateTest();
//
//private:
//	static QLearningMgr* instance;
//	QLearningMgr();
//	//std::vector <SApair> saVector;
//
//
//};