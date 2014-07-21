#pragma once
#include <BWAPI.h>
#include "Common.h"

#include "Defines.h"
#if defined(DEBUG)
	#define _CRTDBG_MAP_ALLOC
	#include <stdlib.h>
	#include <crtdbg.h>
#endif

class UnitManager;
class SquadManager;
class QLearningMgr;

// Remember not to use "Broodwar" in any global class constructor!

class FerranBotAIModule : public virtual BWAPI::AIModule
{
public:
	virtual ~FerranBotAIModule();


  // Virtual functions for callbacks, leave these as they are.
  virtual void onStart();
  virtual void onEnd(bool isWinner);
  virtual void onFrame();
  virtual void onSendText(std::string text);
  virtual void onReceiveText(BWAPI::Player player, std::string text);
  virtual void onPlayerLeft(BWAPI::Player player);
  virtual void onNukeDetect(BWAPI::Position target);
  virtual void onUnitDiscover(BWAPI::Unit unit);
  virtual void onUnitEvade(BWAPI::Unit unit);
  virtual void onUnitShow(BWAPI::Unit unit);
  virtual void onUnitHide(BWAPI::Unit unit);
  virtual void onUnitCreate(BWAPI::Unit unit);
  virtual void onUnitDestroy(BWAPI::Unit unit);
  virtual void onUnitMorph(BWAPI::Unit unit);
  virtual void onUnitRenegade(BWAPI::Unit unit);
  virtual void onSaveGame(std::string gameName);
  virtual void onUnitComplete(BWAPI::Unit unit);
  // Everything below this line is safe to modify.


  void attackClosestEnemyUnit(BWAPI::Unitset::iterator u, int index);
  void checkCanIssueNextAction();
  //static BWAPI::Position m_lastActionTilePos;
  //static BWAPI::Unit m_lastUnitAttacked;
  //static Action m_currentAction;

  //Count frames in which the game would update. There is already if ( Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
  //in the onFrame method, but this is still more restrictive. 
  //m_canIssueNextAction is used so that no two actions are processed too quickly. If not, sometimes the actions were useless (ie, issuing too many attack commands)


  


  bool m_issuingNewAction; //CHAPUZA TEMPORAL, fins que mogui attackClosestEnemyUnit a SquadEntity

  //UnitManager*  m_unitManager;
  //SquadManager* m_squadManager;
  //QLearningMgr* m_qLearningMgr;
  //
};
