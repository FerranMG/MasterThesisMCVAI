#include "QLearningMgr.h"

QLearningMgr* QLearningMgr::instance = nullptr;

QLearningMgr::QLearningMgr()
{
}

QLearningMgr::~QLearningMgr()
{
}

QLearningMgr* QLearningMgr::getInstance()
{
	if(instance == nullptr)
	{
		instance = new QLearningMgr();
	}

	return instance;
}

void QLearningMgr::hola()
{
	int a = 0;
}



//#include "QLearningMgr.h"
//
//using namespace BWAPI;

//QLearningMgr* QLearningMgr::instance = nullptr;
//
//QLearningMgr::QLearningMgr()
//{
//}
//
//
//QLearningMgr::~QLearningMgr()
//{
//}
//
//QLearningMgr* QLearningMgr::getInstance()
//{
//	if(instance == nullptr)
//	{
//		instance = new QLearningMgr();
//	}
//
//	return instance;
//}
//
//
////void QLearningMgr::updateQ(SApair sapair, int reward)
////{
////
////}
////
////
////
////State QLearningMgr::getCurrentState(Unitset::iterator u)
////{
////	State state = State.groundEnemiesE;
////	return state;
////}
//
//
//void QLearningMgr::updateTest()
//{
//
//}