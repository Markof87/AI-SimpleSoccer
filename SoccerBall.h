#ifndef SOCCERBALL_H
#define SOCCERBALL_H
#pragma warning (disable:4786)
//------------------------------------------------------------------------
//
//  Name: SoccerBall.h
//
//  Desc: Class to implement a soccer ball. This class inherits from MovingEntity
//        and provides further functionality for collision testing and position prediction.
//
//------------------------------------------------------------------------
#include <vector>

#include "Game/MovingEntity.h"
#include "constants.h"

class Wall2D;
class PlayerBase;

class SoccerBall : public MovingEntity {

private:
	//Keeps a record of the ball's position at the last update.
	Vector2D m_vOldPos;

	//A pointer to the player (or goalkeeper) who possessed the ball.
	PlayerBase* m_pOwner;

	//A local reference to the Walls that make up the pitch boundary (used in the collision detection).
	const std::vector<Wall2D>& m_PitchBoundary;

public:
	//Tests to see if the ball has collided with a ball and reflects the ball's
	//velocity accordingly.
	void TestCollisionWithWalls(const std::vector<Wall2D>& walls);

	SoccerBall(Vector2D pos, double BallSize, double mass, std::vector<Wall2D>& PitchBoundary) :
		MovingEntity(pos, BallSize, Vector2D(0, 0), -1.0, Vector2D(0, 1), mass, Vector2D(1.0, 1.0), 0, 0), m_PitchBoundary(PitchBoundary){}

	//Implement base class Update
	void Update();

	//Implement base class Render
	void Render();

	Vector2D AddNoiseToKick(Vector2D BallPos, Vector2D BallTarget);

	//A soccer ball doesn't need to handle messages
	bool HandleMessage(const Telegram& msg) { return false; }

	//This method applies a directional force to the ball (kikcs it!)
	void Kick(Vector2D direction, double force);

	//Given a kicking force and a distance to traverse defined by start and finish points, 
	//this method calculates how long it will take the ball to cover the distance.
	double TimeToCoverDistance(Vector2D from, Vector2D to, double force)const;

	//This method calculates where the ball will in 'time' seconds
	Vector2D FuturePosition(double time)const;

	//This is used by players and goalkeepers to 'trap' a ball -- to stop it dead.
	//That player is then assumed to be in possession of the ball and m_pOwner is adjusted accordingly
	void Trap() { m_vVelocity.Zero(); }

	Vector2D OldPos()const { return m_vOldPos; }

	//This places the ball at the desired location and sets its velocity to zero.
	void PlaceAtPosition(Vector2D NewPos);

};

#endif // !SOCCERBALL_H

