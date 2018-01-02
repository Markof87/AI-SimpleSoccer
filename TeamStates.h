#ifndef TEAMSTATES_H
#define TEAMSTATES_H
//------------------------------------------------------------------------
//
//  Name: TeamStates.h
//
//  Desc: State prototypes for football team states.
//
//-----------------------------------------------------------------------
#include "FSM/State.h"
#include "Messaging/Telegram.h"
#include <string>

class SoccerTeam;

//-----------------------------------------------------------------------
class PrepareForKickOff : public State<SoccerTeam> {

private:
	PrepareForKickOff() {}

public:
	//Singleton
	static PrepareForKickOff* Instance();
	void Enter(SoccerTeam* keeper);
	void Execute(SoccerTeam* keeper);
	void Exit(SoccerTeam* keeper);
	bool OnMessage(SoccerTeam*, const Telegram&) { return false; }

};

//-----------------------------------------------------------------------
class Defending : public State<SoccerTeam> {

private:
	Defending() {}

public:
	//Singleton
	static Defending* Instance();
	void Enter(SoccerTeam* keeper);
	void Execute(SoccerTeam* keeper);
	void Exit(SoccerTeam* keeper);
	bool OnMessage(SoccerTeam*, const Telegram&) { return false; }

};

//-----------------------------------------------------------------------
class Attacking : public State<SoccerTeam> {

private:
	Attacking(){}

public:
	//Singleton
	static Attacking* Instance();
	void Enter(SoccerTeam* keeper);
	void Execute(SoccerTeam* keeper);
	void Exit(SoccerTeam* keeper);
	bool OnMessage(SoccerTeam*, const Telegram&) { return false; }

};

#endif // !TEAMSTATES_H
