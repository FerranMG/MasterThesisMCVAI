#include <BWAPI.h>
#include "Defines.h"
#include "Common.h"

class SquadEntity;

class UnitEntity
{
public:
	UnitEntity(BWAPI::Unitset::iterator u);
	~UnitEntity();

	void update();
	void assignToSquad(SquadEntity* squadEntity);
	SquadEntity* getSquad() const;

	void setSquadAction(Action action);
	void setUnitAction(UnitAction action);

	BWAPI::Unitset::iterator getUnitIterator() const;
	BWAPI::Unit getUnit() const {return m_unit;}
	void   applyCurrentUnitAction();

	bool   getIsEnemyUnit() const;
	void   setIsEnemyUnit(bool yes);

	UnitState		getCurrentUnitState() const;
	UnitAction		getCurrentUnitAction() const;
	Action			getCurrentSquadAction() const;
	UnitState		getLastUnitState() const;
	UnitAction		getLastUnitAction() const;
	Action			getLastSquadAction() const;

	void setLastUnitAction(UnitAction action);
	void setLastSquadAction(Action action);

	bool canIssueNextAction() const;
	bool checkCanIssueNextAction();
	bool mustUpdateCurrentAction();
	void setCanIssueNextAction(bool yes);
	void setLastUnitState(UnitState stateNew);

	void setCurrentUnitToAttack(BWAPI::Unit enemyUnit);
	void resetCurrentActionTilePos();

private:
	void attackClosestEnemyUnit();
	void holdPosition();
	bool performBasicChecks() const;

	void checkDirectEnemy();
	void setHealthInUnitState(UnitState& state) const;
	void setDpsInUnitState(UnitState& state) const;
	void setDistanceInUnitState(UnitState& state) const;
	void setHitPointsInUnitState(UnitState& state) const;
	void setEnemyHitPointsInUnitState(UnitState& state) const;
	void setLastUnitActionInUnitState(UnitState& state) const;
	void setLastSquadActionInUnitState(UnitState& state) const;
	void applySquadActionToUnit();

	BWAPI::Unitset::iterator m_unitIterator;
	BWAPI::Unit m_unit;
	BWAPI::Unit m_directEnemy;

	SquadEntity* m_squad;
	Action m_squadAction;
	UnitAction m_unitAction;

	BWAPI::Position m_currentActionTilePos;
	BWAPI::Unit		m_currentUnitAttacked;

	BWAPI::Position m_lastActionTilePos;
	BWAPI::Unit		m_lastUnitAttacked;

	bool m_isEnemyUnit;

	bool m_canIssueNextAction;

	int m_frameCount;

	UnitState	m_lastUnitState;
	UnitAction	m_lastUnitAction;
	Action		m_lastSquadAction;

	bool m_hasUnitStartedAttack;
};