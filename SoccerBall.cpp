#include "2D/geometry.h"
#include "2D/Wall2D.h"
#include "Debug/DebugConsole.h"
#include "misc/Cgdi.h"
#include "ParamLoader.h"
#include "SoccerBall.h"

//---------------------------------AddNoiseToKick-----------------------------------
//
// This can be used to vary the accuracy of a player's kick. Just call it prior to kicking
// the ball using the ball's position and the ball target as parameters.
//
//-----------------------------------------------------------------------------------
Vector2D AddNoiseToKick(Vector2D BallPos, Vector2D BallTarget) {

	double displacement = (Pi - Pi * Prm.PlayerKickingAccuracy) * RandomClamped();
	Vector2D toTarget = BallTarget - BallPos;
	Vec2DRotateAroundOrigin(toTarget, displacement);
	return toTarget + BallPos;

}

//--------------------------------------Kick---------------------------------------
//
// Applies a force to the ball in the direction of heading. Truncates the new 
// velocity to make sure it doesn't exceed the max allowable.
//
//----------------------------------------------------------------------------------
void SoccerBall::Kick(Vector2D direction, double force) {

	//Ensure direction is normalized
	direction.Normalize();

	//Calculate the acceleration
	Vector2D acceleration = (direction * force) / m_dMass;

	//Update the velocity
	m_vVelocity = acceleration;

}

//-------------------------------------Update--------------------------------------
//
// Updates the ball physics, tests for any collisions and adjusts the ball's velocity accordingly
//
//----------------------------------------------------------------------------------
void SoccerBall::Update() {

	//Keep a record of the old position so the goal::scored method can used it for goal testing
	m_vOldPos = m_vPosition;

	//Tests for collisions
	TestCollisionWithWalls(m_PitchBoundary);

	//Simulate Prm.Friction. Make sure the speed is positive
	if (m_vVelocity.LengthSq() > Prm.Friction * Prm.Friction) {

		m_vVelocity += Vec2DNormalize(m_vVelocity) * Prm.Friction;
		m_vPosition += m_vVelocity;

		//Update heading
		m_vHeading = Vec2DNormalize(m_vVelocity);

	}

}

//-------------------------------TimeToCoverDistance-------------------------------
//
// Given a force and a distance to cover given by two vectors, this method calculates
// how long it will take the ball to travel between the two points
//
//----------------------------------------------------------------------------------
double SoccerBall::TimeToCoverDistance(Vector2D A, Vector2D B, double force)const {

	//This will be the velocity of the ball in the next time step IF the player was to make the pass.
	double speed = force / m_dMass;

	//Calculates the velocity at B using the equation
	//
	//v^2 = u^2 + 2as
	
	//First calculate s (distance between the two positions)
	double DistanceToCover = Vec2DDistance(A, B);
	double term = speed * speed + 2.0 * DistanceToCover * Prm.Friction;

	//If (u^2 + 2as) is negative it means the ball cannot reach point B.
	if (term <= 0.0) return -1.0;

	double v = sqrt(term);

	//It is possible for the ball to reach B and we know its speed when it gets there,
	//so now it's easy to calculate the time using the equation:
	//
	// t = v - u
	//     -----
	//       a
	return (v - speed) / Prm.Friction;

}

//---------------------------------FuturePosition----------------------------------
//
// Given a time this method returns the ball position at that time in the future.
//
//----------------------------------------------------------------------------------
Vector2D SoccerBall::FuturePosition(double time)const {

	//Using the equation s = ut + 1/2at^2, where s = distance, a = friction, u = start velocity.

	//Calculate the ut term, which is a vector
	Vector2D ut = m_vVelocity * time;

	//Calculate the 1/2at^2 term, which is scalar
	double half_a_t_squared = 0.5 * Prm.Friction * time * time;

	//Turn the scalar quantity into a vector by multiplying the value with the normalized
	//velocity vector (because that gives the direction)
	Vector2D ScalarToVector = half_a_t_squared * Vec2DNormalize(m_vVelocity);

	//The predicted position is the balls position plus these two terms.
	return Pos() + ut + ScalarToVector;

}

//-------------------------------------Render--------------------------------------
//
// Renders the ball
//
//----------------------------------------------------------------------------------
void SoccerBall::Render() {

	gdi->BlackBrush();
	gdi->Circle(m_vPosition, m_dBoundingRadius);

	/*
	gdi->GreenBush();
	for (int i = 0; i < IPPoints.size(); ++i) gdi->Circle(IPPoints[i], 3);
	*/

}

//-----------------------------TestCollisionWithWalls------------------------------
//
//---------------------------------------------------------------------------------
void SoccerBall::TestCollisionWithWalls(const std::vector<Wall2D>& walls) {

	//Test ball against each wall, find out which is closest
	int idxClosest = -1;

	Vector2D VelNormal = Vec2DNormalize(m_vVelocity);
	Vector2D IntersectionPoint, CollisionPoint;
	double DistToIntersection = MaxFloat;

	//Iterate through each wall and calculate if the ball intersects.
	//If it does then store the index into the closest intersecting wall.
	for (unsigned int w = 0; w < walls.size(); ++w) {

		//Assuming a collision if the ball continued on its current heading
		//calculate the point on the ball that would hit the wall.
		//This is simply the wall's normal (inversed) multiplied by the ball's
		//radius and added to the balls center (its position)
		Vector2D ThisCollisionPoint = Pos() - (walls[w].Normal() * BRadius());

		//Calculate exactly where the collision point will hit the plane.
		if (WhereIsPoint(ThisCollisionPoint, walls[w].From(), walls[w].Normal()) == plane_backside) {

			double DistToWall = DistanceToRayPlaneIntersection(ThisCollisionPoint, walls[w].Normal(), walls[w].From(), walls[w].Normal());
			IntersectionPoint = ThisCollisionPoint + (DistToWall * walls[w].Normal());

		}
		else {

			double DistToWall = DistanceToRayPlaneIntersection(ThisCollisionPoint, VelNormal, walls[w].From(), walls[w].Normal());
			IntersectionPoint = ThisCollisionPoint + (DistToWall * VelNormal);

		}
		
		//Check to make sure the intersection point is actually on the line segment.
		bool OnLineSegment = false;
		if (LineIntersection2D(walls[w].From(), walls[w].To(), ThisCollisionPoint - walls[w].Normal()*20.0, ThisCollisionPoint - walls[w].Normal()*20.0)) OnLineSegment = true;

		//N.B: there is no test for collision with the end of a line segment now check to see if the collision point is within range of the velocity vector.
		//Work in distance squared to avoid sqrt and if it's the closest hit found so far.
		//If it is that means the ball will collide with the wall sometimes between this time step and the next one.
		double distSq = Vec2DDistanceSq(ThisCollisionPoint, IntersectionPoint);

		if ((distSq <= m_vVelocity.LengthSq()) && (distSq < DistToIntersection) && OnLineSegment) {

			DistToIntersection = distSq;
			idxClosest = w;
			CollisionPoint = IntersectionPoint;

		}

	}

	//To prevent having to calculate the exact time of collision we can just check if the velocity is opposite to the wall normal before reflecting it.
	//This prevents the case where there is overshoot and the ball gets reflected back over the line before it has completely reentered the playing area.
	if ((idxClosest >= 0) && VelNormal.Dot(walls[idxClosest].Normal()) < 0) m_vVelocity.Reflect(walls[idxClosest].Normal());

}

//--------------------------------PlaceAtLocation----------------------------------
//
// Positions the ball at the desired location and sets the ball's velocity to zero.
//
//----------------------------------------------------------------------------------
void SoccerBall::PlaceAtPosition(Vector2D NewPos) {

	m_vPosition = NewPos;
	m_vOldPos = m_vPosition;
	m_vVelocity.Zero();

}