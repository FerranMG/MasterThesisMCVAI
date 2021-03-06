#pragma once
#include <BWAPI.h>
#include <vector>
#include "Common.h"
#include "Defines.h"

class UnitEntity;

class SquadEntity
{
public:
	SquadEntity();
	~SquadEntity();

	void addUnit(BWAPI::Unitset::iterator u);
	void addUnits(const BWAPI::Unitset &units);
	BWAPI::Unitset getSquadUnits() const;
	std::vector<UnitEntity*> getSquadUnitEntities();

	void update();

	State getCurrentState();

	void calculateAvgHealth();
	void calculateAvgDps();
	void calculateSqDistToClosestEnemyGroup();
	void calculateAvgPosition();
	BWAPI::Position calculateAvgPosition(BWAPI::Unitset units);
	void assignCurrentEnemySquad();
	void calculateNumUnits();
	void removeDeadUnits();
	static bool checkNoHitPoints(const BWAPI::Unit unit);

	void setAvgDps(int num);
	void setAvgHealth(int health);
	void setSqDistToClosestEnemyGroup(int dist);
	void setAvgPosition(BWAPI::Position pos);
	void setNumUnits(int num);

	void			setPosToSurround(BWAPI::Position pos) {m_posToSurround = pos;}
	BWAPI::Position getPosToSurround() const {return m_posToSurround;}

	SquadEntity* getEnemySquad() const {return m_enemySquad;}

	int								getAvgDps();
	int								getAvgHealth();
	int								getSqDistToClosestEnemyGroup();
	BWAPI::Position					getAvgPosition();
	int								getNumUnits();
	float							getDispersionSqDist() const;
	std::vector<BWAPI::Position>	calculateSurroundPositions(bool forceCalculate);
	void							calculateUnitsDispersion();
	bool							checkCanIssueNextAction();

	void applyCurrentAction();
	void attackClosestEnemyUnit();
	bool performBasicChecks(BWAPI::Unitset::iterator u);
	void holdPositions();
	void attackSurround();
	void attackHalfSurround();

	void setHealthInState(State& state);
	void setDpsInState(State& state);
	void setDistanceInState(State& state);
	void setHitPointsInState(State& state);
	void setEnemyHitPointsInState(State& state);
	void setLastActionInState(State& state);

	Action getLastAction() const;
	void   setLastAction(Action action);

	Action getCurrentAction() const;
	void   setCurrentAction(Action action);

	State  getLastState() const;
	void   setLastState(State state);

	bool   getCanIssueNextAction() const;
	void   setCanIssueNextAction(bool yes);

	bool   getIsEnemySquad() const;
	void   setIsEnemySquad(bool yes);

	bool getUnitToAttack(UnitEntity* unitEntity, BWAPI::Unit& oUnitToAttack);
	BWAPI::Position calculateActionTilePosForSurround(UnitEntity* unitEntity);
	bool hasNewSurroundPositions() const;
	bool canIssueNextAction();
	std::vector<BWAPI::Position> calculateHalfSurroundPositions(bool forceCalculate);
	int getNumUnitsKilled();


	BWAPI::Position m_lastActionTilePos;
	BWAPI::Unit		m_lastUnitAttacked;

	int m_absoluteNumberOfUnitsEverInSquad;

private:
	BWAPI::Unitset	m_squadUnits;

	int				m_avgDps;
	int				m_avgHealth;
	int				m_sqDistToClosestEnemyGroup;
	BWAPI::Position m_avgPos;
	int				m_numUnits;
	SquadEntity*	m_enemySquad;
	bool			m_canIssueNextAction;

	BWAPI::Position m_posToSurround;
	float			m_dispersionSqDist;

	Action			m_lastAction;
	Action			m_currentAction;
	State			m_lastState;

	int				m_frameCount;

	bool			m_isEnemySquad;
	std::vector<BWAPI::Position> m_surroundPositions;

};