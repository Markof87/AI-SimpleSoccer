#ifndef PARAMLOADER
#define PARAMLOADER
#pragma warning(disable:4800)
//------------------------------------------------------------------------
//
//  Name: ParamLoader.h
//
//  Desc: Singleton class to handle the loading of default parameter values
//	      from an initialization file "params.ini"
//
//------------------------------------------------------------------------
#include <fstream>
#include <string>
#include <cassert>

#include "constants.h"
#include "misc/iniFileLoaderBase.h"

#define Prm (*ParamLoader::Instance())

class ParamLoader : public iniFileLoaderBase {

public:
	static ParamLoader* Instance();

	double GoalWidth;
	
	int NumSupportSpotsX;
	int NumSupportSpotsY;

	//These values tweak the various rules used to calculate the support spots
	double Spot_PassSafeScore;
	double Spot_CanScoreFromPositionScore;
	double Spot_DistFromControllingPlayerScore;
	double Spot_ClosenessToSupportPlayerScore;
	double Spot_AheadOfAttackerScore;

	double SupportSpotUpdateFreq;

	double ChancePlayerAttemptPotShot;
	double ChanceOfUsingArriveTypeReceiveBehavior;

	double BallSize;
	double BallMass;
	double Friction;

	double KeeperInBallRange;
	double KeeperInBallRangeSq;

	double PlayerInTargetRange;
	double PlayerInTargetRangeSq;

	double PlayerMass;

	//Max steering force
	double PlayerMaxForce;

	double PlayerMaxSpeedWithBall;
	double PlayerMaxSpeedWithoutBall;
	double PlayerMaxTurnRate;
	double PlayerScale;
	double PlayerComfortZone;
	double PlayerComfortZoneSq;
	double PlayerKickingDistance;
	double PlayerKickingDistanceSq;
	double PlayerKickingFrequency;

	//In the range zero to 1.0 adjust the amount of noise added to a kick, the lower value the worse the players get
	double PlayerKickingAccuracy;

	double MaxDribbleForce;
	double MaxShootingForce;
	double MaxPassingForce;

	//Number of times the SoccerTeam::CanShoot method attempts to find a valid shot.
	int NumAttempsToFindValidStrike;

	//The distance away from the center of its home region a player must be to be considered at home.
	double WithinRangeOfHome;

	//How close a player must get to a sweet spot before he can change state.
	double WithinRangeOfSupportSpot;
	double WithinRangeOfSupportSpotSq;

	//The minimum distance a receiving player must be from the passing player
	double MinPassDist;
	double GoalKeeperMinPassDist;

	//This is the distance the keeper puts between the back of the net and the ball when using
	//the interpose steering behavior.
	double GoalKeeperTendingDistance;

	//When the ball becomes within this distance of the goalkeeper he changes state to intercept the ball.
	double GoalKeeperInterceptRange;
	double GoalKeeperInterceptRangeSq;

	//How close the ball must be to a receiver before he starts chasing it.
	double BallWithinReceivingRange;
	double BallWithinReceivingRangeSq;

	//These values control what debug info you can see.
	bool bStates;
	bool bIDs;
	bool bSupportSpots;
	bool bRegions;
	bool bShowControllingTeam;
	bool bViewTargets;
	bool bHighlightIfThreatened;
	
	int FrameRate;

	double SeparationCoefficient;

	//How close a neighbour must be before an agent perceives it.
	double ViewDistance;

	//Zero this to turn the constraint off.
	bool bNonPenetrationConstraint;


private:
	ParamLoader() :iniFileLoaderBase("Params.ini") {

		GoalWidth = GetNextParameterDouble();

		NumSupportSpotsX = GetNextParameterInt();
		NumSupportSpotsY = GetNextParameterInt();

		Spot_PassSafeScore = GetNextParameterDouble();
		Spot_CanScoreFromPositionScore = GetNextParameterDouble();
		Spot_DistFromControllingPlayerScore = GetNextParameterDouble();
		Spot_ClosenessToSupportPlayerScore = GetNextParameterDouble();
		Spot_AheadOfAttackerScore = GetNextParameterDouble();

		SupportSpotUpdateFreq = GetNextParameterDouble();

		ChancePlayerAttemptPotShot = GetNextParameterDouble();
		ChanceOfUsingArriveTypeReceiveBehavior = GetNextParameterDouble();

		BallSize = GetNextParameterDouble();
		BallMass = GetNextParameterDouble();
		Friction = GetNextParameterDouble();

		KeeperInBallRange = GetNextParameterDouble();
		KeeperInBallRangeSq = KeeperInBallRange * KeeperInBallRange;

		PlayerInTargetRange = GetNextParameterDouble();
		PlayerInTargetRangeSq = PlayerInTargetRange * PlayerInTargetRange;

		PlayerMass = GetNextParameterDouble();

		PlayerMaxForce = GetNextParameterDouble();

		PlayerMaxSpeedWithBall = GetNextParameterDouble();
		PlayerMaxSpeedWithoutBall = GetNextParameterDouble();
		PlayerMaxTurnRate = GetNextParameterDouble();
		PlayerScale = GetNextParameterDouble();
		PlayerComfortZone = GetNextParameterDouble();
		PlayerComfortZoneSq = PlayerComfortZone * PlayerComfortZone;
		PlayerKickingDistance += BallSize;
		PlayerKickingDistanceSq = PlayerKickingDistance * PlayerKickingDistance;
		PlayerKickingFrequency = GetNextParameterDouble();
		PlayerKickingAccuracy = GetNextParameterDouble();

		MaxDribbleForce = GetNextParameterDouble();
		MaxShootingForce = GetNextParameterDouble();
		MaxPassingForce = GetNextParameterDouble();

		NumAttempsToFindValidStrike = GetNextParameterInt();

		WithinRangeOfHome = GetNextParameterDouble();

		WithinRangeOfSupportSpot = GetNextParameterDouble();
		WithinRangeOfSupportSpotSq = WithinRangeOfSupportSpot * WithinRangeOfSupportSpot;

		MinPassDist = GetNextParameterDouble();
		GoalKeeperMinPassDist = GetNextParameterDouble();

		GoalKeeperTendingDistance = GetNextParameterDouble();

		GoalKeeperInterceptRange = GetNextParameterDouble();
		GoalKeeperInterceptRangeSq = GoalKeeperInterceptRange * GoalKeeperInterceptRange;

		BallWithinReceivingRange = GetNextParameterDouble();
		BallWithinReceivingRangeSq = BallWithinReceivingRange * BallWithinReceivingRange;

		bStates = GetNextParameterBool();
		bIDs = GetNextParameterBool();
		bSupportSpots = GetNextParameterBool();
		bRegions = GetNextParameterBool();
		bShowControllingTeam = GetNextParameterBool();
		bViewTargets = GetNextParameterBool();
		bHighlightIfThreatened = GetNextParameterBool();

		FrameRate = GetNextParameterInt();

		SeparationCoefficient = GetNextParameterDouble();

		ViewDistance = GetNextParameterDouble();

		bNonPenetrationConstraint = GetNextParameterBool();

	}

};

#endif // !PARAMLOADER

