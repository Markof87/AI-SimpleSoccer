#include "Debug/DebugConsole.h"
#include "time/Regulator.h"

#include "constants.h"
#include "Goal.h"
#include "ParamLoader.h"
#include "PlayerBase.h"
#include "SoccerBall.h"
#include "SoccerTeam.h"
#include "SoccerPitch.h"
#include "SupportSpotCalculator.h"

SupportSpotCalculator::~SupportSpotCalculator() {
	delete m_pRegulator;
}

SupportSpotCalculator::SupportSpotCalculator(int numX, int numY, SoccerTeam* team) :m_pBestSupportingSpot(NULL), m_pTeam(team) {

	const Region* PlayingField = team->Pitch()->PlayingArea();

	//Calculate the positions of each sweet spot, create them and store them in m_Spots
	double HeightOfSSRegion = PlayingField->Height() * 0.8;
	double WidthOfSSRegion = PlayingField->Width() * 0.9;
	double SliceX = WidthOfSSRegion / numX;
	double SliceY = HeightOfSSRegion / numY;

	double left = PlayingField->Left() + (PlayingField->Width() - WidthOfSSRegion) / 2.0 + SliceX / 2.0;
	double right = PlayingField->Right() - (PlayingField->Width() - WidthOfSSRegion) / 2.0 + SliceX / 2.0;
	double top = PlayingField->Top() + (PlayingField->Height() - HeightOfSSRegion) / 2.0 + SliceY / 2.0;

	for (int x = 0; x < (numX / 2) - 1; ++x) {

		for (int y = 0; y < numY; ++y) {

			if (m_pTeam->Color() == SoccerTeam::blue) m_Spots.push_back(SupportSpot(Vector2D(left + x * SliceX, top + y * SliceY), 0.0));
			else m_Spots.push_back(SupportSpot(Vector2D(right - x * SliceX, top + y * SliceY), 0.0));

		}

	}

	//Create the regulator
	m_pRegulator = new Regulator(Prm.SupportSpotUpdateFreq);

}

//-----------------------------DetermineBestSupportingPosition-----------------------------
//
//-----------------------------------------------------------------------------------------
Vector2D SupportSpotCalculator::DetermineBestSupportingPosition() {

	//Only update the spots every few frames
	if (!m_pRegulator->isReady() && m_pBestSupportingSpot) return m_pBestSupportingSpot->m_vPos;

	//Reset the best supporting spot
	m_pBestSupportingSpot = NULL;

	double BestScoreSoFar = 0.0;
	std::vector<SupportSpot>::iterator curSpot;
	for (curSpot = m_Spots.begin(); curSpot != m_Spots.end(); ++curSpot) {

		//First remove any previous score.
		curSpot->m_dScore = 1.0;

		//Test 1: is it possible to make a safe pass from the ball's position to this position?
		if (m_pTeam->IsPassSafeFromAllOpponents(m_pTeam->ControllingPlayer()->Pos(), curSpot->m_vPos, NULL, Prm.MaxPassingForce)) curSpot->m_dScore += Prm.Spot_PassSafeScore;

		//Test 2: determine if a goal can be scored from this position.
		if (m_pTeam->CanShoot(curSpot->m_vPos, Prm.MaxShootingForce)) curSpot->m_dScore += Prm.Spot_CanScoreFromPositionScore;

		//Test 3: calculate how far this spot is away from the controlling player. The further away, the higher the score.
		//Any distances further away than OptimalDistance pixels do not receive a score.
		if (m_pTeam->SupportingPlayer()) {

			const double OptimalDistance = 200.0; //TODO ?????
			double dist = Vec2DDistance(m_pTeam->ControllingPlayer()->Pos(), curSpot->m_vPos);
			double temp = fabs(OptimalDistance - dist);

			//Normalize the distance and add it to the score
			if (temp < OptimalDistance) curSpot->m_dScore += Prm.Spot_DistFromControllingPlayerScore * (OptimalDistance - temp) / OptimalDistance;
		
		}

		//Check to see if this spot has the highest score so far.
		if (curSpot->m_dScore > BestScoreSoFar) {

			BestScoreSoFar = curSpot->m_dScore;
			m_pBestSupportingSpot = &(*curSpot);

		}

		return m_pBestSupportingSpot->m_vPos;

	}

}

//----------------------------------GetBestSupportingSpot----------------------------------
//
//------------------------------------------------------------------------------------------
Vector2D SupportSpotCalculator::GetBestSupportingSpot() {

	if (m_pBestSupportingSpot) return m_pBestSupportingSpot->m_vPos;
	else return DetermineBestSupportingPosition();

}

//------------------------------------------Render-----------------------------------------
//
//------------------------------------------------------------------------------------------
void SupportSpotCalculator::Render()const {

	gdi->HollowBrush();
	gdi->GreyPen();

	for (unsigned int spt = 0; spt < m_Spots.size(); ++spt) gdi->Circle(m_Spots[spt].m_vPos, m_Spots[spt].m_dScore);

	if (m_pBestSupportingSpot) {
		gdi->GreenPen();
		gdi->Circle(m_pBestSupportingSpot->m_vPos, m_pBestSupportingSpot->m_dScore);
	}

}
