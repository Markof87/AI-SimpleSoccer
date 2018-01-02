#ifndef GOALKEEPERSTATES_H
#define GOALKEEPERSTATES_H
//------------------------------------------------------------------------
//
//  Name: GoalkeeperStates.h
//
//  Desc: Declarations of all the states used by a Simple Soccer goalkeeper.
//
//-----------------------------------------------------------------------
#include "FSM/State.h"
#include "Messaging/Telegram.h"
#include "constants.h"
#include <string>

class GoalKeeper;
class SoccerPitch;

class GoalKeeperState : public State<GoalKeeper> {

private:
	GoalKeeperState(){}

public:
	//Singleton.
	static GoalKeeperState* Instance();
	void Enter(GoalKeeper* keeper) {};
	void Execute(GoalKeeper* keeper) {};
	void Exit(GoalKeeper* keeper) {};
	bool OnMessage(GoalKeeper*, const Telegram&);

};

//-----------------------------------------------------------------------------------
class TendGoal : public State<GoalKeeper> {

private:
	TendGoal(){}

public:
	//Singleton.
	static TendGoal* Instance();
	void Enter(GoalKeeper* keeper);
	void Execute(GoalKeeper* keeper);
	void Exit(GoalKeeper* keeper);
	bool OnMessage(GoalKeeper*, const Telegram&) { return false; }
};

//-----------------------------------------------------------------------------------
class InterceptBall : public State<GoalKeeper> {

private:
	InterceptBall() {}

public:
	//Singleton.
	static InterceptBall* Instance();
	void Enter(GoalKeeper* keeper);
	void Execute(GoalKeeper* keeper);
	void Exit(GoalKeeper* keeper);
	bool OnMessage(GoalKeeper*, const Telegram&) { return false; }
};

//-----------------------------------------------------------------------------------
class ReturnHome : public State<GoalKeeper> {

private:
	ReturnHome() {}

public:
	//Singleton.
	static ReturnHome* Instance();
	void Enter(GoalKeeper* keeper);
	void Execute(GoalKeeper* keeper);
	void Exit(GoalKeeper* keeper);
	bool OnMessage(GoalKeeper*, const Telegram&) { return false; }
};

//-----------------------------------------------------------------------------------
class PutBallBackInPlay : public State<GoalKeeper> {

private:
	PutBallBackInPlay() {}

public:
	//Singleton.
	static PutBallBackInPlay* Instance();
	void Enter(GoalKeeper* keeper);
	void Execute(GoalKeeper* keeper);
	void Exit(GoalKeeper* keeper) {};
	bool OnMessage(GoalKeeper*, const Telegram&) { return false; }
};

#endif // !GOALKEEPERSTATES_H

