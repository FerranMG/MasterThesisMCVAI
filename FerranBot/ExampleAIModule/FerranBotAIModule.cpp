#include "FerranBotAIModule.h"
#include <iostream>
#include <fstream>
#include "SquadManager.h"
#include "QLearningMgr.h"
#include "UnitManager.h"


using namespace BWAPI;
using namespace Filter;
 

void FerranBotAIModule::onStart()
{
#if defined(DEBUG)
	int bwapi_rev = BWAPI_getRevision();
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif
	//assert(_CrtCheckMemory());

	// Hello World!
	Broodwar->sendText("Hello world FerranBot!");
	
	// Print the map name.
	// BWAPI returns std::string when retrieving a string, don't forget to add .c_str() when printing!
	Broodwar << "The map is " << Broodwar->mapName() << "!" << std::endl;

	// Enable the UserInput flag, which allows us to control the bot and type messages.
	Broodwar->enableFlag(Flag::UserInput);

	// Uncomment the following line and the bot will know about everything through the fog of war (cheat).
	//Broodwar->enableFlag(Flag::CompleteMapInformation);

	// Set the command optimization level so that common commands can be grouped
	// and reduce the bot's APM (Actions Per Minute).
	Broodwar->setCommandOptimizationLevel(2);

	// Check if this is a replay
	if ( Broodwar->isReplay() )
	{

		// Announce the players in the replay
		Broodwar << "The following players are in this replay:" << std::endl;
		
		// Iterate all the players in the game using a std:: iterator
		Playerset players = Broodwar->getPlayers();
		for(auto p = players.begin(); p != players.end(); ++p)
		{
			// Only print the player if they are not an observer
			if ( !p->isObserver() )
				Broodwar << p->getName() << ", playing as " << p->getRace() << std::endl;
		}

	}
	else // if this is not a replay
	{
		// Retrieve you and your enemy's races. enemy() will just return the first enemy.
		// If you wish to deal with multiple enemies then you must use enemies().
		if ( Broodwar->enemy() ) // First make sure there is an enemy
			Broodwar << "The matchup is " << Broodwar->self()->getRace() << " vs " << Broodwar->enemy()->getRace() << std::endl;
	}
	//assert(_CrtCheckMemory());

	//m_unitManager = UnitManager::getInstance();
	//assert(_CrtCheckMemory());
	UnitManager::getInstance();

	//m_squadManager = SquadManager::getInstance();
	////assert(_CrtCheckMemory());
	SquadManager::getInstance()->addSquad(new SquadEntity());
	////assert(_CrtCheckMemory());
	SquadManager::getInstance()->getSquads()->at(0)->addUnits(Broodwar->self()->getUnits());
	////assert(_CrtCheckMemory());
	SquadManager::getInstance()->addEnemySquad(new SquadEntity());

	//m_qLearningMgr = QLearningMgr::getInstance();
	////assert(_CrtCheckMemory());
	QLearningMgr::getInstance()->readFromStream();
}

void FerranBotAIModule::onEnd(bool isWinner)
{
	//Update the q_map one last time, because tracking a win or a loss is, you know, important. :D
	if(isWinner)
	{
		QLearningMgr::getInstance()->forceReward(100.0f);
	}
	else
	{
		QLearningMgr::getInstance()->forceReward(-100.0f);
	}

	SquadEntity* currentSquad = SquadManager::getInstance()->getSquads()->at(0);

	QLearningMgr::getInstance()->updateSquadQ(currentSquad->getLastState(), currentSquad->getLastAction(), currentSquad->getCurrentState());
	
	SquadManager::getInstance()->removeAllSquads();

	delete SquadManager::getInstance()->m_squads;
	delete SquadManager::getInstance()->m_enemySquads;
	SquadManager::getInstance()->m_squads = NULL;
	SquadManager::getInstance()->m_enemySquads = NULL;
	delete SquadManager::getInstance();

	UnitManager::getInstance()->removeAllUnitEntities();
	delete UnitManager::getInstance();

	////Save q_map and q_count info in a file
	QLearningMgr::getInstance()->writeToStream();

	delete QLearningMgr::getInstance();


	Broodwar->restartGame();

	//int leaks = _CrtCheckMemory();
	//_CrtDumpMemoryLeaks();
}

void FerranBotAIModule::onFrame()
{
	// Called once every game frame
	//assert(_CrtCheckMemory());
	// Display the game frame rate as text in the upper left area of the screen
	Broodwar->drawTextScreen(200, 0,"FPS: %d", Broodwar->getFPS() );
	Broodwar->drawTextScreen(200, 20, "Average FPS: %f", Broodwar->getAverageFPS() );

	// Return if the game is a replay or is paused
	if ( Broodwar->isReplay() || Broodwar->isPaused() || !Broodwar->self() )
		return;

	// Prevent spamming by only running our onFrame once every number of latency frames.
	// Latency frames are the number of frames before commands are processed.
	if ( Broodwar->getFrameCount() % Broodwar->getLatencyFrames() != 0)
	{
		return;
	}

	//m_frameCount++;
	////assert(_CrtCheckMemory());
	SquadManager::getInstance()->update();
	////assert(_CrtCheckMemory());
	UnitManager::getInstance()->update();
	////assert(_CrtCheckMemory());
	
	//for(size_t squadIdx = 0; squadIdx < SquadManager::getInstance()->getSquads()->size(); squadIdx++)
	////FOR EACH SQUAD
	////for(std::vector<SquadEntity*>::iterator squad = SquadManager::getInstance()->getSquads()->begin(); squad != SquadManager::getInstance()->getSquads()->end(); ++squad)
	//{
	//	SquadEntity* squad = SquadManager::getInstance()->getSquads()->at(squadIdx);

	//	State stateNew = squad->getCurrentState();
	//	if(squad->getNumUnits() > 0)
	//	{
	//		Action action = QLearningMgr::getInstance()->updateQ(squad->m_lastState, squad->m_lastAction, stateNew);

	//		Broodwar->sendText("Last State H %d DPS %d dist %d Action %d", squad->m_lastState.m_avgHealthGroup, squad->m_lastState.m_avgDpsXHealthGroup, squad->m_lastState.m_distToClosestEnemyGroup, squad->m_lastAction);
	//		assert(action >= ATTACK && action <= COUNT);

	//		squad->m_currentAction = action;
	//		squad->m_canIssueNextAction = false;

	//		squad->m_lastAction = action;
	//		squad->m_lastState = stateNew;


	//		squad->applyCurrentAction();

	//	}
	//}
}

void FerranBotAIModule::onSendText(std::string text)
{

	// Send the text to the game if it is not being processed.
	Broodwar->sendText("%s", text.c_str());


	// Make sure to use %s and pass the text as a parameter,
	// otherwise you may run into problems when you use the %(percent) character!

}

void FerranBotAIModule::onReceiveText(BWAPI::Player player, std::string text)
{
	// Parse the received text
	Broodwar << player->getName() << " said \"" << text << "\"" << std::endl;
}

void FerranBotAIModule::onPlayerLeft(BWAPI::Player player)
{
	// Interact verbally with the other players in the game by
	// announcing that the other player has left.
	Broodwar->sendText("Goodbye %s!", player->getName().c_str());
}

void FerranBotAIModule::onNukeDetect(BWAPI::Position target)
{

	// Check if the target is a valid position
	if ( target )
	{
		// if so, print the location of the nuclear strike target
		Broodwar << "Nuclear Launch Detected at " << target << std::endl;
	}
	else 
	{
		// Otherwise, ask other players where the nuke is!
		Broodwar->sendText("Where's the nuke?");
	}

	// You can also retrieve all the nuclear missile targets using Broodwar->getNukeDots()!
}

void FerranBotAIModule::onUnitDiscover(BWAPI::Unit unit)
{
}

void FerranBotAIModule::onUnitEvade(BWAPI::Unit unit)
{
}

void FerranBotAIModule::onUnitShow(BWAPI::Unit unit)
{
}

void FerranBotAIModule::onUnitHide(BWAPI::Unit unit)
{
}

void FerranBotAIModule::onUnitCreate(BWAPI::Unit unit)
{
	if ( Broodwar->isReplay() )
	{
		// if we are in a replay, then we will print out the build order of the structures
		if ( unit->getType().isBuilding() && !unit->getPlayer()->isNeutral() )
		{
			int seconds = Broodwar->getFrameCount()/24;
			int minutes = seconds/60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s creates a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
}

void FerranBotAIModule::onUnitDestroy(BWAPI::Unit unit)
{
}

void FerranBotAIModule::onUnitMorph(BWAPI::Unit unit)
{
	if ( Broodwar->isReplay() )
	{
		// if we are in a replay, then we will print out the build order of the structures
		if ( unit->getType().isBuilding() && !unit->getPlayer()->isNeutral() )
		{
			int seconds = Broodwar->getFrameCount()/24;
			int minutes = seconds/60;
			seconds %= 60;
			Broodwar->sendText("%.2d:%.2d: %s morphs a %s", minutes, seconds, unit->getPlayer()->getName().c_str(), unit->getType().c_str());
		}
	}
}

void FerranBotAIModule::onUnitRenegade(BWAPI::Unit unit)
{
}

void FerranBotAIModule::onSaveGame(std::string gameName)
{
	Broodwar << "The game was saved to \"" << gameName << "\"" << std::endl;
}

void FerranBotAIModule::onUnitComplete(BWAPI::Unit unit)
{
	//Broodwar->sendText("unit completed! id = %d positionX = %d, positionY = %d", unit->getID(), unit->getPosition().x, unit->getPosition().y);
	//std::ofstream fileFerran;
	//fileFerran.open("bwapi-data\\logs\\fileFerran.txt");
	//fileFerran << "unit id " << unit->getID();
	//fileFerran.close();
}


void FerranBotAIModule::attackClosestEnemyUnit(Unitset::iterator u, int unitIndex)
{
	//if(SquadManager::getInstance() != NULL)
	//{
	//	if(SquadManager::getInstance()->m_enemySquad != NULL)
	//	{
	//		//TRACE("test");
	//		std::cerr << "num enemy units " << SquadManager::getInstance()->m_enemySquad->getNumUnits();
	//	}
	//	else
	//	{
	//		std::cerr << "m_enemySquad NULL!!!";
	//	}

	//}
	//else
	//{
	//	std::cerr << "Squad Manager NULL!!!";
	//}

	//Unitset enemyUnits = SquadManager::getInstance()->m_enemySquad->getSquadUnits();

	//
	//int index = 0;
	//int unitIndexToAttack = 0;
	//int minDist = 999999;
	//Position unitPos = u->getPosition();
	//for(Unitset::iterator it = enemyUnits.begin(); it != enemyUnits.end(); it++, index++)
	//{
	//	Position enemyPos = it->getPosition();
	//	int dist = enemyPos.getApproxDistance(unitPos);
	//	if(dist < minDist)
	//	{
	//		unitIndexToAttack = index;
	//		minDist = dist;
	//	}
	//}

	////Don't send an attack command twice in a row over the same unit
	//bool attackFrame = u->isAttackFrame();
	//bool startingAttack = u->isStartingAttack();
	//bool attacking = u->isAttacking();



	//if(m_issuingNewAction && unitIndex != 0 || unitIndex == 0)
	//{
	//	if(squad->m_lastUnitAttacked == NULL || squad->m_lastUnitAttacked != enemyUnits[unitIndexToAttack])
	//	{
	//		if(!u->isAttackFrame() && !u->isStartingAttack() && !u->isAttacking())
	//		{
	//			u->attack(enemyUnits[unitIndexToAttack]);
	//			squad->m_lastUnitAttacked = enemyUnits[unitIndexToAttack];

	//			if(unitIndex == 0)
	//			{
	//				m_issuingNewAction = true; //UNCOMMENT THIS TO MAKE THE GAME CRASH WHEN RESTARTING
	//			}
	//		}
	//	}
	//}

	////SquadManager::getInstance()->m_squad->getNumUnits();
	////if(unitIndex == SquadManager::getInstance()->m_squad->getNumUnits())
	////{
	////	m_issuingNewAction = false;
	////}

	//


	////u->attack(enemyUnits[unitIndexToAttack], true);
}



////Check if, given the last action that was issued, a new one can be processed
////This is in order not to send too many actions in a short interval of time.
////Sort of like let the actions already issued have a consequence before sending a new one.
//void FerranBotAIModule::checkCanIssueNextAction()
//{
//	//TODO - Iterate over all the squads
//	SquadEntity* squad = SquadManager::getInstance()->m_squad;
//	if(squad->getNumUnits() > 0)
//	{
//		Unitset squadUnits = squad->getSquadUnits();
//
//		if(m_currentAction == ATTACK)
//		{
//			//Remove info from old FLEE or HOLD actions
//			m_frameCount = 0;
//			m_lastActionTilePos = Position(-1, -1);
//
//			if(SquadManager::getInstance()->m_enemySquad->getNumUnits() <= 0)
//			{
//				//No enemy units on sight, can send new action
//				m_currentAction = COUNT;
//				m_canIssueNextAction = true;
//			}
//			else
//			{
//				//When some unit has started to attack, then we can send a new action
//				bool isAnyUnitAttacking = false;
//				for ( Unitset::iterator u = squadUnits.begin(); u != squadUnits.end(); ++u )
//				{
//					if(u->isAttacking())
//					{
//						isAnyUnitAttacking = true;
//						break;
//					}
//				}
//
//				if(isAnyUnitAttacking)
//				{
//					m_currentAction = COUNT;
//					m_canIssueNextAction = true;
//				}
//			}
//		}
//		else if(m_currentAction == ATTACK_SURROUND)
//		{
//			Position enemyPos = squad->getEnemySquad()->getAvgPosition();
//			Position posToSurround = squad->getPosToSurround();
//
//			if(Common::computeSqDistBetweenPoints(posToSurround, enemyPos) > 50 * 50)
//			{
//				m_currentAction = COUNT;
//				m_canIssueNextAction = true;
//			}
//			else
//			{
//				m_canIssueNextAction = false;
//			}
//		}
//		else if(m_currentAction == HOLD || m_currentAction == FLEE)
//		{
//			//Remove info from old ATTACK action
//			m_lastUnitAttacked = NULL;
//
//			if(m_frameCount > 5)
//			{
//				m_currentAction = COUNT;
//				m_canIssueNextAction = true;
//				m_frameCount = 0;
//			}
//		}
//	}
//}

