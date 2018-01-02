#pragma warning (disable:4786)
#ifndef PLAYERBASE_H
#define PLAYERBASE_H
//------------------------------------------------------------------------
//
//  Name: PlayerBase.h
//
//  Desc: Definition of a soccer player base class. The player inherits from the autolist
//        class so that any player created will be automatically added to a list that is easily
//        accessible by any other game objects.
//
//------------------------------------------------------------------------
#include <vector>
#include <string>
#include <cassert>
#include "2D/Vector2D.h"
#include "misc/autolist.h"
#include "Game/MovingEntity.h"

class SoccerTeam;
class SoccerPitch;
class SoccerBall;
class SteeringBehaviors;
class Region;

class PlayerBase : public MovingEntity,
	public AutoList<PlayerBase>
{

public:
	enum player_role{goal_keeper, attacker, defender};

protected:
	//This player's role in the team
	player_role m_PlayerRole;

	//A pointer to this player's team
	SoccerTeam* m_pTeam;

	//The steering behaviors
	SteeringBehaviors* m_pSteering;

	//The region that this player is assigned to.
	int m_iHomeRegion;

	//The region this player moves to before kickoff.
	int m_iDefaultRegion;

	//The distance to the ball (in squared-space). This value is queried a lot
	//so it's calculated once each time-step and stored here.
	double m_dDistSqToBall;

	//The vertex buffer.
	std::vector<Vector2D> m_vecPlayerVB;

	//The buffer for the transformed vertices
	std::vector<Vector2D> m_vecPlayerVBTrans;

public:
	PlayerBase(SoccerTeam* home_team, int home_region, Vector2D heading, Vector2D velocity, double mass, double max_force, double max_speed, double max_turn_rate, double scale, player_role role);
	virtual ~PlayerBase();

	//Returns true if there is an opponent within this player's comfort zone
	bool IsThreatened()const;

	//Rotates the player to face the ball or the player's current target
	void TrackBall();
	void TrackTarget();

	//This messages the player that is closest to the supporting spot to change state to support the attacking player.
	void FindSupport()const;

	//Returns true if the ball can be grabbed by the goalkeeper.
	bool BallWithinKeeperRange()const;

	//Returns true if the ball is withing kicking range.
	bool BallWithinKickingRange()const;

	//Returns true if the ball comes within range of a receiver.
	bool BallWithinReceivingRange()const;

	//Returns true if the player is located within the boundaries of his home region.
	bool InHomeRegion()const;

	//Returns true if this player is ahead of the attacker.
	bool IsAheadOfAttacker()const;

	//Returns true if a player is located at the designated support spot.
	bool AtSupportSpot()const {};

	//Returns true if the player is located at his steering target.
	bool AtTarget()const;

	//Returns true if the player is the closest player in his team to the ball.
	bool IsClosestTeamMemberToBall()const;

	//Returns true if the point specified by 'position' is located in front of the player.
	bool PositionInFrontOfPlayer(Vector2D position)const;

	//Returns true if the player is the closest player on the pitch to the ball.
	bool IsClosestPlayerOnPitchToBall()const;

	//Returns true if the player is the controlling player.
	bool IsControllingPlayer()const;

	//Returns true if the player is located in the designated 'hot region', the area close to the opponent's goal
	bool InHotRegion()const;

	player_role Role()const { return m_PlayerRole; }
	double DistSqToBall()const { return m_dDistSqToBall; }
	void SetDistSqToBall(double val) { m_dDistSqToBall = val; }

	//Calculate distance to opponent's/home goal. Used frequently by the passing methods.
	double DistToOppGoal()const;
	double DistToHomeGoal()const;

	void SetDefaultHomeRegion() { m_iHomeRegion = m_iDefaultRegion; }

	SoccerBall* const Ball()const;
	SoccerPitch* const Pitch()const;
	SteeringBehaviors* const Steering()const { return m_pSteering; }
	const Region* const HomeRegion()const;
	void SetHomeRegion(int NewRegion) { m_iHomeRegion = NewRegion; }
	SoccerTeam* const Team()const { return m_pTeam; }

};

#endif // !PLAYERBASE_H
