#ifndef FIELDPLAYERSTATES_H
#define FIELDPLAYERSTATES_H
//------------------------------------------------------------------------
//
//  Name: FieldPlayerStates.h
//
//  Desc: States for the field players of Simple Soccer.
//
//------------------------------------------------------------------------
#include <string>

#include "FSM/State.h"
#include "Messaging/Telegram.h"
#include "constants.h"

class FieldPlayer;
class SoccerPitch;

//------------------------------------------------------------------------
class GlobalPlayerState : public State<FieldPlayer> {

private:
	GlobalPlayerState(){}

public:
	//Singleton
	static GlobalPlayerState* Instance();

	void Enter(FieldPlayer* player){}
	void Execute(FieldPlayer* player) {}
	void Exit(FieldPlayer* player) {}

	bool OnMessage(FieldPlayer*, const Telegram&);

};

//------------------------------------------------------------------------
class ChaseBall : public State<FieldPlayer> {

private:
	ChaseBall() {}

public:
	//Singleton
	static ChaseBall* Instance();

	void Enter(FieldPlayer* player) {}
	void Execute(FieldPlayer* player) {}
	void Exit(FieldPlayer* player) {}

	bool OnMessage(FieldPlayer*, const Telegram&) { return false; }

};

//------------------------------------------------------------------------
class Wait : public State<FieldPlayer> {

private:
	Wait() {}

public:
	//Singleton
	static Wait* Instance();

	void Enter(FieldPlayer* player) {}
	void Execute(FieldPlayer* player) {}
	void Exit(FieldPlayer* player) {}

	bool OnMessage(FieldPlayer*, const Telegram&) { return false; }

};

//------------------------------------------------------------------------
class ReceiveBall : public State<FieldPlayer> {

private:
	ReceiveBall() {}

public:
	//Singleton
	static ReceiveBall* Instance();

	void Enter(FieldPlayer* player) {}
	void Execute(FieldPlayer* player) {}
	void Exit(FieldPlayer* player) {}

	bool OnMessage(FieldPlayer*, const Telegram&) { return false; }

};

//------------------------------------------------------------------------
class KickBall : public State<FieldPlayer> {

private:
	KickBall() {}

public:
	//Singleton
	static KickBall* Instance();

	void Enter(FieldPlayer* player) {}
	void Execute(FieldPlayer* player) {}
	void Exit(FieldPlayer* player) {}

	bool OnMessage(FieldPlayer*, const Telegram&) { return false; }

};

//------------------------------------------------------------------------
class Dribble : public State<FieldPlayer> {

private:
	Dribble() {}

public:
	//Singleton
	static Dribble* Instance();

	void Enter(FieldPlayer* player) {}
	void Execute(FieldPlayer* player) {}
	void Exit(FieldPlayer* player) {}

	bool OnMessage(FieldPlayer*, const Telegram&) { return false; }

};

//------------------------------------------------------------------------
class SupportAttacker : public State<FieldPlayer> {

private:
	SupportAttacker() {}

public:
	//Singleton
	static SupportAttacker* Instance();

	void Enter(FieldPlayer* player) {}
	void Execute(FieldPlayer* player) {}
	void Exit(FieldPlayer* player) {}

	bool OnMessage(FieldPlayer*, const Telegram&) { return false; }

};

//------------------------------------------------------------------------
class ReturnToHomeRegion : public State<FieldPlayer> {

private:
	ReturnToHomeRegion() {}

public:
	//Singleton
	static ReturnToHomeRegion* Instance();

	void Enter(FieldPlayer* player) {}
	void Execute(FieldPlayer* player) {}
	void Exit(FieldPlayer* player) {}

	bool OnMessage(FieldPlayer*, const Telegram&) { return false; }

};

#endif // !FIELDPLAYERSTATES_H

