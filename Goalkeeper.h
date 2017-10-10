#ifndef GOALKEEPER_H
#define GOALKEEPER_H
//------------------------------------------------------------------------
//
//  Name: Goalkeeper.h
//
//  Desc: class to implement a goaleeper agent.
//
//------------------------------------------------------------------------
#include "2D/Vector2D.h"
#include "FSM/StateMachine.h"
#include "PlayerBase.h"

class PlayerBase;

class GoalKeeper : public PlayerBase {

private:

	//An instance of the state machine class.
	StateMachine<GoalKeeper>* m_pStateMachine;

	//This vector is updated to point towards the ball and is used when rendering the goalkeeper to ensure he always appears to be watching the ball.
	Vector2D m_vLookAt;

public:
	GoalKeeper(SoccerTeam* home_team, int home_region, State<GoalKeeper>* start_state, Vector2D heading, Vector2D velocity, double mass, double max_force, double max_speed, double max_turn_rate, double scale);
	~GoalKeeper() { delete m_pStateMachine; }

	//These must be implemented
	void Update();
	void Render();
	bool HandleMessage(const Telegram& msg);

	//Returns true if the ball comes close enough for the keeper to consider intercepting.
	bool BallWithinRangeForIntercept()const;

	//Returns true if the keeper has ventured too far away from the goalmouth.
	bool TooFarFromGoalMouth()const;

	//This method is called by the Intercept state to determine the spot along the goalmouth which will act as one of the interpose targets (the other is the ball).
	//The specific point at the goal line that the keeper is trying to cover is flexible and can move depending on where the ball is on the field.
	//To achieve this we just scale the ball's y value by the ratio of the goal width to playingField width.
	Vector2D GetRearInterposeTarget()const;

	StateMachine<GoalKeeper>* GetFSM()const { return m_pStateMachine; }

	Vector2D LookAt()const { return m_vLookAt; }
	void SetLookAt(Vector2D v) { m_vLookAt = v; }

};

#endif