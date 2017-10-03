#include "2D/geometry.h"
#include "Debug/DebugConsole.h"
#include "Messaging/MessageDispatcher.h"
#include "Messaging/Telegram.h"
#include "time/Regulator.h"
#include "FieldPlayer.h"
#include "FieldPlayerStates.h"
#include "Goal.h"
#include "ParamLoader.h"
#include "SoccerBall.h"
#include "SoccerMessages.h"
#include "SoccerPitch.h"
#include "SoccerTeam.h"
#include "SteeringBehaviors.h"

#define PLAYER_STATE_INFO_ON

//GLOBAL STATE
GlobalPlayerState* GlobalPlayerState::Instance() {
	static GlobalPlayerState instance;
	return &instance;
}

void GlobalPlayerState::Execute(FieldPlayer* player) {

	//If a player is in possession and close to the ball reduce his max speed
	if ((player->BallWithinReceivingRange())) player->SetMaxSpeed(Prm.PlayerMaxSpeedWithBall);
	else player->SetMaxSpeed(Prm.PlayerMaxSpeedWithoutBall);

}

bool GlobalPlayerState::OnMessage(FieldPlayer* player, const Telegram& telegram) {

	switch (telegram.Msg) {

	case Msg_ReceiveBall:
	{
		//Set the target.
		player->Steering()->SetTarget(*(static_cast<Vector2D*>(telegram.ExtraInfo)));

		//Change state.
		player->GetFSM()->ChangeState(ReceiveBall::Instance());

		return true;
	}
	break;

	case Msg_SupportAttacker:
	{

		//If already supporting just return.
		if (player->GetFSM()->isInState(*SupportAttacker::Instance())) return true;

		//Set the target to be the best supporting position.
		player->Steering()->SetTarget(player->Team()->GetSupportSpot());

		//Change state.
		player->GetFSM()->ChangeState(SupportAttacker::Instance());

		return true;

	}
	break;

	case Msg_Wait:
	{

		//Change state.
		player->GetFSM()->ChangeState(Wait::Instance());

		return true;

	}
	break;

	case Msg_GoHome:
	{

		player->SetDefaultHomeRegion();

		//Change state.
		player->GetFSM()->ChangeState(ReturnToHomeRegion::Instance());

		return true;

	}
	break;

	case Msg_PassToMe:
	{

		//Get the position of the player requesting the pass.
		FieldPlayer* receiver = static_cast<FieldPlayer*>(telegram.ExtraInfo);

		#ifdef PLAYER_STATE_INFO_ON
			debug_con << "Player " << player->ID() << " received request from " << receiver->ID() << " to make pass" << "";
		#endif // PLAYER_STATE_INFO_ON

		//If the ball is not within kicking range or their is already a receiving player,
		//this player cannot pass the ball to the player making the request.
		if (player->Team()->Receiver() != NULL || !player->BallWithinKickingRange()) {

			#ifdef PLAYER_STATE_INFO_ON
				debug_con << "Player " << player->ID() << " cannot make requested pass <cannot kick ball>" << "";
			#endif // PLAYER_STATE_INFO_ON

			return true;

		}

		//Make the pass.
		player->Ball()->Kick(receiver->Pos() - player->Ball()->Pos(), Prm.MaxPassingForce);

		#ifdef PLAYER_STATE_INFO_ON
			debug_con << "Player " << player->ID() << " passed ball to requesting player" << "";
		#endif // PLAYER_STATE_INFO_ON

		//Let the receiver know a pass is coming
		Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY, player->ID(), receiver->ID(), Msg_ReceiveBall, &receiver->Pos());

		//Change state.
		player->GetFSM()->ChangeState(Wait::Instance());

		player->FindSupport();

		return true;

	}
	break;

	}

	return false;

}

//CHASE BALL
ChaseBall* ChaseBall::Instance() {
	
	static ChaseBall instance;
	return &instance;

}

void ChaseBall::Enter(FieldPlayer* player) {

	player->Steering()->SeekOn();

	#ifdef PLAYER_STATE_INFO_ON
		debug_con << "Player " << player->ID() << " enters chase state" << "";
	#endif // PLAYER_STATE_INFO_ON

}

void ChaseBall::Execute(FieldPlayer* player) {

	//If the ball is within kicking range the player changes state to KickBall.
	if (player->BallWithinKickingRange()) player->GetFSM()->ChangeState(KickBall::Instance());
	return;

	//If the player is the closest player to the ball then he should keep chasing it.
	if (player->IsClosestTeamMemberToBall()) player->Steering()->SetTarget(player->Ball()->Pos());
	return;

	//If the player is not closest to the ball anymore, he should return back to his home region and wait for another opportunity.
	player->GetFSM()->ChangeState(ReturnToHomeRegion::Instance());

}

void ChaseBall::Exit(FieldPlayer* player) {
	player->Steering()->SeekOff();
}

//SUPPORT ATTACKING PLAYER
SupportAttacker* SupportAttacker::Instance() {

	static SupportAttacker instance;
	return &instance;

}

void SupportAttacker::Enter(FieldPlayer* player) {

	player->Steering()->ArriveOn();
	player->Steering()->SetTarget(player->Team()->GetSupportSpot());

	#ifdef PLAYER_STATE_INFO_ON
		debug_con << "Player " << player->ID() << " enters support state" << "";
	#endif // PLAYER_STATE_INFO_ON

}

void SupportAttacker::Execute(FieldPlayer* player) {

	//If his team loses control go back home.
	if (!player->Team()->InControl()) player->GetFSM()->ChangeState(ReturnToHomeRegion::Instance());
	return;

	//If the best supporting spot changes, change the steering target.
	if (player->Team()->GetSupportSpot() != player->Steering()->Target()) {

		player->Steering()->SetTarget(player->Team()->GetSupportSpot());
		player->Steering()->ArriveOn();

	}

	//If this player has a shot at the goal AND the attacker can pass the ball to him the attacker should pass the ball to his player.
	if (player->Team()->CanShoot(player->Pos(), Prm.MaxShootingForce)) player->Team()->RequestPass(player);

	//If this player is located at the support spot and his team still have possession, he should reamin still and turn to face the ball.
	if (player->AtTarget()) {

		player->Steering()->ArriveOff();

		//The player should keep his eyes on the ball.
		player->TrackBall();
		
		player->SetVelocity(Vector2D(0, 0));

		//If not threatened by another player request a pass.
		if (!player->IsThreatened()) player->Team()->RequestPass(player);

	}

}

void SupportAttacker::Exit(FieldPlayer* player) {

	//Set supporting player to null so that the team knows it has to determine a new one.
	player->Team()->SetSupportingPlayer(NULL);
	player->Steering()->ArriveOff();

}

//RETURN TO HOME REGION
ReturnToHomeRegion* ReturnToHomeRegion::Instance() {

	static ReturnToHomeRegion instance;
	return &instance;

}

void ReturnToHomeRegion::Enter(FieldPlayer* player) {

	player->Steering()->ArriveOn();
	if (!player->HomeRegion()->Inside(player->Steering()->Target(), Region::halfsize)) player->Steering()->SetTarget(player->HomeRegion()->Center());

	#ifdef PLAYER_STATE_INFO_ON
		debug_con << "Player " << player->ID() << " enters ReturnToHome state" << "";
	#endif // PLAYER_STATE_INFO_ON

}

void ReturnToHomeRegion::Execute(FieldPlayer* player) {

	if (player->Pitch()->GameOn()) {

		//If the ball is nearer this player than any other team member AND there is not an assigned receiver
		//AND the goalkeeper does not gave the ball, go chase it.
		if (player->IsClosestTeamMemberToBall() && (player->Team()->Receiver() == NULL) && !player->Pitch()->GoalKeeperHasBall()) {

			player->GetFSM()->ChangeState(ChaseBall::Instance());
			return;

		}

	}

	//If game is on and close enough to home, change state to wait and set the player target to his current position.
	if (player->Pitch()->GameOn() && player->HomeRegion()->Inside(player->Pos(), Region::halfsize)) {

		player->Steering()->SetTarget(player->Pos());
		player->GetFSM()->ChangeState(Wait::Instance());

	}

	//If game is not on the player must return much closer to the center of his home region.
	else if (!player->Pitch()->GameOn() && player->AtTarget()) player->GetFSM()->ChangeState(Wait::Instance());

}

void ReturnToHomeRegion::Exit(FieldPlayer* player) {
	player->Steering()->ArriveOff();
}

//WAIT
Wait* Wait::Instance() {

	static Wait instance;
	return &instance;

}

void Wait::Enter(FieldPlayer* player) {

	#ifdef PLAYER_STATE_INFO_ON
		debug_con << "Player " << player->ID() << " enters wait state" << "";
	#endif // PLAYER_STATE_INFO_ON

	//If the game is not on make sure the target is the center of the player's home region.
	//This is ensure all the players are in the correct positions ready for kick off.
	if (!player->Pitch()->GameOn()) player->Steering()->SetTarget(player->HomeRegion()->Center());

}

void Wait::Execute(FieldPlayer* player) {

	//If the player has been jostled out of position, get back in the position.
	if (!player->AtTarget()) {
		player->Steering()->ArriveOn();
		return;
	}

	else {

		player->Steering()->ArriveOff();
		player->SetVelocity(Vector2D(0, 0));

		//The player should keep his eyes on the ball.
		player->TrackBall();

	}

	//If this player's team is controlling AND this player is not the attacker 
	//AND is further up the field than the attacker he should request a pass.
	if (player->Team()->InControl() && (!player->IsControllingPlayer()) && player->IsAheadOfAttacker()) {

		player->Team()->RequestPass(player);
		return;

	}

	if (player->Pitch()->GameOn()) {

		//If the ball is nearer this player than any other team member AND there is not an assigned receiver
		//AND neither goalkeeper has the ball, go chase it.
		if (player->IsClosestTeamMemberToBall() && player->Team()->Receiver() == NULL && !player->Pitch()->GoalKeeperHasBall()) {

			player->GetFSM()->ChangeState(ChaseBall::Instance());
			return;

		}

	}

}

void Wait::Exit(FieldPlayer* player){}

//KICK BALL

KickBall* KickBall::Instance() {

	static KickBall instance;
	return &instance;

}

void KickBall::Enter(FieldPlayer* player) {

	//Let the team know this player is controlling.
	player->Team()->SetControllingPlayer(player);

	//The player can only make so many kick attempts per second.
	if(!player->IsReadyForNextKick()) player->GetFSM()->ChangeState(ChaseBall::Instance());

	#ifdef PLAYER_STATE_INFO_ON
		debug_con << "Player " << player->ID() << " enters kick state" << "";
	#endif // PLAYER_STATE_INFO_ON

}