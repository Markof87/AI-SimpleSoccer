#include "2D/geometry.h"
#include "Debug/DebugConsole.h"
#include "Messaging/MessageDispatcher.h"
#include "Messaging/Telegram.h"
#include "FieldPlayer.h"
#include "Goal.h"
#include "Goalkeeper.h"
#include "GoalkeeperStates.h"
#include "ParamLoader.h"
#include "PlayerBase.h"
#include "SoccerMessages.h"
#include "SoccerPitch.h"
#include "SoccerTeam.h"
#include "SteeringBehaviors.h"

//Uncomment to send state info to debug window
//#define GOALY_STATE_INFO_ON

//-------------------------------------GoalKeeperState------------------------------------
//-----------------------------------------------------------------------------------------
GoalKeeperState* GoalKeeperState::Instance() {

	static GoalKeeperState instance;
	return &instance;

}

bool GoalKeeperState::OnMessage(GoalKeeper* keeper, const Telegram& telegram) {

	switch (telegram.Msg) {

	case Msg_GoHome:
	{

		keeper->SetDefaultHomeRegion();
		keeper->GetFSM()->ChangeState(ReturnHome::Instance());

	}
	break;

	case Msg_ReceiveBall:
	{
		keeper->GetFSM()->ChangeState(InterceptBall::Instance());
	}
	break;

	}

	return false;

}

//-----------------------------------------TendGoal----------------------------------------
//
// This is the main state for the goalkeeper. When in this state he will move left to right across
// the goalmouth using the 'interpose' steering behavior to put himself between the ball and the back of the net.
// If the ball comes within the 'goalkeeper range' he moves out of the goalmouth to attempt to intecept it.
//-----------------------------------------------------------------------------------------
TendGoal* TendGoal::Instance() {

	static TendGoal instance;
	return &instance;

}

void TendGoal::Enter(GoalKeeper* keeper) {

	//Turn interpose on
	keeper->Steering()->InterposeOn(Prm.GoalKeeperTendingDistance);

	//Interpose will position the agent between the ball position and a target position situated along the goal mouth.
	//This call sets the target.
	keeper->Steering()->SetTarget(keeper->GetRearInterposeTarget());

}

void TendGoal::Execute(GoalKeeper* keeper) {

	//The rear interpose target will change as the ball's position changes so it must be updated each update-step.
	keeper->Steering()->SetTarget(keeper->GetRearInterposeTarget());

	//If the ball comes in range the keeper traps it and then changes state to put the ball back in play.
	if (keeper->BallWithinKeeperRange()) {

		keeper->Ball()->Trap();
		keeper->Pitch()->SetGoalKeeperHasBall(true);
		keeper->GetFSM()->ChangeState(PutBallBackInPlay::Instance());
		return;

	}

	//If the ball is within a predefined distance, the keeper moves out from position to try and intercept it.
	if (keeper->BallWithinRangeForIntercept() && !keeper->Team()->InControl()) keeper->GetFSM()->ChangeState(InterceptBall::Instance());

	//If the keeper has ventured too far away from the goal-line and there is no threat from the opponents he should move back towards it.
	if (keeper->TooFarFromGoalMouth() && keeper->Team()->InControl()) {

		keeper->GetFSM()->ChangeState(ReturnHome::Instance());
		return;

	}

}

void TendGoal::Exit(GoalKeeper* keeper) {
	keeper->Steering()->InterposeOff();
}

//---------------------------------------ReturnHome---------------------------------------
//
// In this state the goalkeeper simply returns back to the center of the goal region before changing state back to TendGoal
//-----------------------------------------------------------------------------------------
ReturnHome* ReturnHome::Instance() {

	static ReturnHome instance;
	return &instance;

}

void ReturnHome::Enter(GoalKeeper* keeper) {
	keeper->Steering()->ArriveOn();
}

void ReturnHome::Execute(GoalKeeper* keeper) {

	keeper->Steering()->SetTarget(keeper->HomeRegion()->Center());

	//If close enough to home or the opponents get control over the ball, change state to tend goal.
	if (keeper->InHomeRegion() || !keeper->Team()->InControl()) keeper->GetFSM()->ChangeState(TendGoal::Instance());

}

void ReturnHome::Exit(GoalKeeper* keeper) {
	keeper->Steering()->ArriveOff();
}

//--------------------------------------InterceptBall--------------------------------------
//
// In this state the GP will attempt to intercept the ball using the pursuit steering behavior,
// but he only does so long as he remains within his home region.
//-----------------------------------------------------------------------------------------
InterceptBall* InterceptBall::Instance() {

	static InterceptBall instance;
	return &instance;

}

void InterceptBall::Enter(GoalKeeper* keeper) {

	keeper->Steering()->PursuitOn();
	#ifdef GOALY_STATE_INFO_ON
		debug_con << "Goaly" << keeper->ID() << " enters InterceptBall" << "";
	#endif // GOALY_STATE_INFO_ON

}

void InterceptBall::Execute(GoalKeeper* keeper) {

	//If the goalkeeper moves to far away from the goal he should return to his home region
	//UNLESS he is the closest player to the ball, in which case, he should keep trying to intercept it.
	if (keeper->TooFarFromGoalMouth() && !keeper->IsClosestPlayerOnPitchToBall()) {

		keeper->GetFSM()->ChangeState(ReturnHome::Instance());
		return;

	}

	//If the ball becomes in range of the goalkeeper's hands he traps the ball and puts it back in play.
	if (keeper->BallWithinKeeperRange()) {

		keeper->Ball()->Trap();
		keeper->Pitch()->SetGoalKeeperHasBall(true);
		keeper->GetFSM()->ChangeState(PutBallBackInPlay::Instance());
		return;

	}

}

void InterceptBall::Exit(GoalKeeper* keeper) {
	keeper->Steering()->PursuitOff();
}

//------------------------------------PutBallBackInPlay------------------------------------
//
//-----------------------------------------------------------------------------------------
PutBallBackInPlay* PutBallBackInPlay::Instance() {

	static PutBallBackInPlay instance;
	return &instance;

}

void PutBallBackInPlay::Enter(GoalKeeper* keeper) {

	//Let the team know that the keeper is in control.
	keeper->Team()->SetControllingPlayer(keeper);

	//Send all the players home
	keeper->Team()->Opponents()->ReturnAllFieldPlayersToHome();
	keeper->Team()->ReturnAllFieldPlayersToHome();

}

void PutBallBackInPlay::Execute(GoalKeeper* keeper) {

	PlayerBase* receiver = NULL;
	Vector2D BallTarget;

	//Test if there are players further forward on the field we might be able to pass to. If so, make a pass.
	if (keeper->Team()->FindPass(keeper, receiver, BallTarget, Prm.MaxPassingForce, Prm.GoalKeeperMinPassDist)) {

		//Make the pass.
		keeper->Ball()->Kick(Vec2DNormalize(BallTarget - keeper->Ball()->Pos()), Prm.MaxPassingForce);

		//Goalkeeper no longer has ball.
		keeper->Pitch()->SetGoalKeeperHasBall(false);

		//Let the receiving player know the ball's comin' at him.
		Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY, keeper->ID(), receiver->ID(), Msg_ReceiveBall, &BallTarget);

		//Go back to tending the goal
		keeper->GetFSM()->ChangeState(TendGoal::Instance());

		return;

	}

	keeper->SetVelocity(Vector2D());

}