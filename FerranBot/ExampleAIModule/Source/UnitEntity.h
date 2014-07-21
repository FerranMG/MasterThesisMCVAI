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
	void setUnitAction(Action action);

	BWAPI::Unitset::iterator getUnitIterator() const;
	BWAPI::Unit getUnit() const {return m_unit;}
	Action calculateUnitAction();
	void   applyCurrentUnitAction();

	bool   getIsEnemyUnit() const;
	void   setIsEnemyUnit(bool yes);

	UnitState		getCurrentUnitState() const;
	Action			getCurrentUnitAction() const;
	Action			getCurrentSquadAction() const;
	UnitState		getLastUnitState() const;
	Action			getLastUnitAction() const;
	Action			getLastSquadAction() const;

	void setLastUnitAction(Action action);
	void setLastSquadAction(Action action);

	bool canIssueNextAction() const;
	bool checkCanIssueNextAction();
	void setCanIssueNextAction(bool yes);

private:
	void attackClosestEnemyUnit();
	void holdPosition();
	void attackSurround();
	bool performBasicChecks() const;

	void checkDirectEnemy();
	void setHealthInUnitState(UnitState& state) const;
	void setDpsXHealthInUnitState(UnitState& state) const;
	void setDistanceInUnitState(UnitState& state) const;
	void setHitPointsInUnitState(UnitState& state) const;
	void setEnemyHitPointsInUnitState(UnitState& state) const;
	void setLastUnitActionInUnitState(UnitState& state) const;
	void setLastSquadActionInUnitState(UnitState& state) const;
	BWAPI::Unitset::iterator m_unitIterator;
	BWAPI::Unit m_unit;
	BWAPI::Unit m_directEnemy;

	SquadEntity* m_squad;
	Action m_squadAction;
	Action m_unitAction;
	BWAPI::Position m_lastActionTilePos;
	BWAPI::Unit		m_lastUnitAttacked;

	bool m_isEnemyUnit;

	bool m_canIssueNextAction;

	int m_frameCount;

	UnitState	m_lastUnitState;
	Action		m_lastUnitAction;
	Action		m_lastSquadAction;



	//TODO - SEE WHAT OTHER INFO I NEED TO GET FROM THE u IN THE CONSTRUCTOR

};