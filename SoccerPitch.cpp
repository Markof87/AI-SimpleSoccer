#include "2D/geometry.h"
#include "2D/Transformations.h"
#include "Debug/DebugConsole.h"
#include "Game/EntityManager.h"
#include "Game/Region.h"
#include "misc/FrameCounter.h"

#include "Goal.h"
#include "ParamLoader.h"
#include "PlayerBase.h"
#include "SoccerBall.h"
#include "SoccerPitch.h"
#include "SoccerTeam.h"
#include "TeamStates.h"

const int NumRegionsHorizontal = 6;
const int NumRegionsVertical = 3;

SoccerPitch::SoccerPitch(int cx, int cy) : m_cxClient(cx), m_cyClient(cy), m_bPaused(false), m_bGoalKeeperHasBall(false), m_Regions(NumRegionsHorizontal * NumRegionsVertical), m_bGameOn(true) {

	//Define the playing area.
	m_pPlayingArea = new Region(20, 20, cx - 20, cy - 20);

	//Create the regions.
	CreateRegions(PlayingArea()->Width() / (double)NumRegionsHorizontal, PlayingArea()->Height() / (double)NumRegionsVertical);

	//Create the goals.
	m_pRedGoal = new Goal(Vector2D(m_pPlayingArea->Left(), (cy - Prm.GoalWidth) / 2), Vector2D(m_pPlayingArea->Left(), (cy - Prm.GoalWidth) / 2), Vector2D(1, 0));
	m_pBlueGoal = new Goal(Vector2D(m_pPlayingArea->Right(), (cy - Prm.GoalWidth) / 2), Vector2D(m_pPlayingArea->Right(), (cy - Prm.GoalWidth) / 2), Vector2D(-1, 0));

	//Create the soccer ball.
	m_pBall = new SoccerBall(Vector2D((double)m_cxClient / 2.0, (double)m_cyClient / 2.0), Prm.BallSize, Prm.BallMass, m_vecWalls);

	//Create the teams.
	m_pRedTeam = new SoccerTeam(m_pRedGoal, m_pBlueGoal, this, SoccerTeam::red);
	m_pBlueTeam = new SoccerTeam(m_pBlueGoal, m_pRedGoal, this, SoccerTeam::blue);

	//Make sure each team knows who their opponents are.
	m_pRedTeam->SetOpponents(m_pBlueTeam);
	m_pBlueTeam->SetOpponents(m_pRedTeam);

	//Create the walls.
	Vector2D TopLeft(m_pPlayingArea->Left(), m_pPlayingArea->Top());
	Vector2D TopRight(m_pPlayingArea->Right(), m_pPlayingArea->Top());
	Vector2D BottomLeft(m_pPlayingArea->Left(), m_pPlayingArea->Bottom());
	Vector2D BottomRight(m_pPlayingArea->Right(), m_pPlayingArea->Bottom());

	m_vecWalls.push_back(Wall2D(BottomLeft, m_pRedGoal->RightPost()));
	m_vecWalls.push_back(Wall2D(m_pRedGoal->LeftPost(), TopLeft));
	m_vecWalls.push_back(Wall2D(TopLeft, TopRight));
	m_vecWalls.push_back(Wall2D(TopRight, m_pBlueGoal->LeftPost()));
	m_vecWalls.push_back(Wall2D(m_pBlueGoal->RightPost(), BottomRight));
	m_vecWalls.push_back(Wall2D(BottomRight, BottomLeft));

	ParamLoader* p = ParamLoader::Instance();

}

SoccerPitch::~SoccerPitch() {

	delete m_pBall;

	delete m_pRedTeam;
	delete m_pBlueTeam;

	delete m_pRedGoal;
	delete m_pBlueGoal;

	delete m_pPlayingArea;

	for (unsigned int i = 0; i < m_Regions.size(); ++i) delete m_Regions[i];

}

//-------------------------------------Update---------------------------------------
//
// This demo works on a fixed frame rate (60 by default) so we don't need to pass a time_elapsed
// as a parameter to the game entities
//
//-----------------------------------------------------------------------------------
void SoccerPitch::Update() {

	if (m_bPaused) return;

	static int tick = 0;

	//Update the balls.
	m_pBall->Update();

	//Update the teams.
	m_pRedTeam->Update();
	m_pBlueTeam->Update();

	//If a goal has been detected reset the pitch ready for kickoff.
	if (m_pBlueGoal->Scored(m_pBall) || m_pRedGoal->Scored(m_pBall)) {

		m_bGameOn = false;

		//Reset the ball.
		m_pBall->PlaceAtPosition(Vector2D((double)m_cxClient / 2.0, (double)m_cyClient / 2.0));

		//Get the teams ready for kickoff.
		m_pRedTeam->GetFSM()->ChangeState(PrepareForKickOff::Instance());
		m_pBlueTeam->GetFSM()->ChangeState(PrepareForKickOff::Instance());

	}

}

//----------------------------------CreateRegions-----------------------------------
//-----------------------------------------------------------------------------------
void SoccerPitch::CreateRegions(double width, double height) {

	//Index into the vector.
	int idx = m_Regions.size() - 1;
	
	for (int col = 0; col < NumRegionsHorizontal; ++col) {

		for (int row = 0; row < NumRegionsVertical; ++row) m_Regions[idx--] = new Region(PlayingArea()->Left() + col * width,
			PlayingArea()->Top() + row * height,
			PlayingArea()->Left() + (col + 1) * width,
			PlayingArea()->Top() + (row + 1) * height, idx);

	}

}

//--------------------------------------Render--------------------------------------
//-----------------------------------------------------------------------------------
bool SoccerPitch::Render() {

	//Draw the grass.
	gdi->DarkGreenPen();
	gdi->DarkGreenBrush();
	gdi->Rect(0, 0, m_cxClient, m_cyClient);

	//Render regions.
	if (Prm.bRegions) {

		for (unsigned int r = 0; r < m_Regions.size(); ++r) m_Regions[r]->Render(true);

	}

	//Render the goals.
	gdi->HollowBrush();
	gdi->RedPen();
	gdi->Rect(m_pPlayingArea->Left(), (m_cyClient - Prm.GoalWidth) / 2, m_pPlayingArea->Left() + 40, m_cyClient - (m_cyClient - Prm.GoalWidth) / 2);

	gdi->BluePen();
	gdi->Rect(m_pPlayingArea->Right(), (m_cyClient - Prm.GoalWidth) / 2, m_pPlayingArea->Right() + 40, m_cyClient - (m_cyClient - Prm.GoalWidth) / 2);

	//Render the pitch markings.
	gdi->WhitePen();
	gdi->Circle(m_pPlayingArea->Center(), m_pPlayingArea->Width() * 0.125);
	gdi->Line(m_pPlayingArea->Center().x, m_pPlayingArea->Top(), m_pPlayingArea->Center().x, m_pPlayingArea->Bottom());
	gdi->WhiteBrush();
	gdi->Circle(m_pPlayingArea->Center(), 2.0);

	//Render the ball.
	gdi->WhitePen();
	gdi->WhiteBrush();
	m_pBall->Render();

	//Render the teams.
	m_pRedTeam->Render();
	m_pBlueTeam->Render();

	//Render the walls.
	gdi->WhitePen();
	for (unsigned int w = 0; w < m_vecWalls.size(); ++w) m_vecWalls[w].Render();

	//Show the score
	gdi->TextColor(Cgdi::red);
	gdi->TextAtPos((m_cxClient / 2) - 50, m_cyClient - 18, "Red: " + ttos(m_pBlueGoal->NumGoalsScored()));

	gdi->TextColor(Cgdi::blue);
	gdi->TextAtPos((m_cxClient / 2) + 10, m_cyClient - 18, "Blue: " + ttos(m_pRedGoal->NumGoalsScored()));

	return true;

}