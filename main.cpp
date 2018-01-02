#pragma warning (disable:4786)

#include <Windows.h>
#include <time.h>

#include "constants.h"
#include "ParamLoader.h"
#include "Resource.h"
#include "SoccerPitch.h"
#include "Debug/DebugConsole.h"
#include "misc/Cgdi.h"
#include "misc/utils.h"
#include "misc/WindowUtils.h"
#include "time/PrecisionTimer.h"

//GLOBALS
char* g_szApplicationName = "SimpleSoccer";
char* g_szWindowClassName = "MyWindowClass";

SoccerPitch* g_SoccerPitch;

//Create a timer.
PrecisionTimer timer(Prm.FrameRate);

//Used when a user clicks on a menu item to ensure the option is 'checked' correctly.
void CheckAllMenuItemsAppropriately(HWND hwnd) {

	CheckMenuItemAppropriately(hwnd, IDM_SHOW_REGIONS, Prm.bRegions);
	CheckMenuItemAppropriately(hwnd, IDM_SHOW_STATES, Prm.bStates);
	CheckMenuItemAppropriately(hwnd, IDM_SHOW_IDS, Prm.bIDs);
	CheckMenuItemAppropriately(hwnd, IDM_AIDS_SUPPORTSPOTS, Prm.bSupportSpots);
	CheckMenuItemAppropriately(hwnd, ID_AIDS_SHOWTARGETS, Prm.bViewTargets);
	CheckMenuItemAppropriately(hwnd, IDM_AIDS_HIGHLITE, Prm.bHighlightIfThreatened);

}

//---------------------------------------WindowProc---------------------------------------
//
// This is the callback function which handles all the windows messages
//
//----------------------------------------------------------------------------------------
LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	//These hold the dimensions of the client window area
	static int cxClient, cyClient;

	//Used to create the back buffer
	static HDC hdcBackBuffer;
	static HBITMAP hBitmap;
	static HBITMAP hOldBitmap;

	switch (msg) {

		//A WM_CREATE msg is sent when your application window is first created
		case WM_CREATE: {

			//To get the size of the client window first we need to create a RECT and then ask Windows to fill in our RECT structure
			//with the client window size. Then we assign to cxClient and cyClient accordingly.
			RECT rect;
			GetClientRect(hwnd, &rect);
			cxClient = rect.right;
			cyClient = rect.bottom;

			//Seed random number generator
			srand((unsigned)time(NULL));

			//Create a surface to render to backbuffer
			hdcBackBuffer = CreateCompatibleDC(NULL);

			//Get the DC for the front buffer
			HDC hdc = GetDC(hwnd);
			hBitmap = CreateCompatibleBitmap(hdc, cxClient, cyClient);

			//Select the bitmap into the memory device context
			hOldBitmap = (HBITMAP)SelectObject(hdcBackBuffer, hBitmap);

			//Release the DC
			ReleaseDC(hwnd, hdc);

			g_SoccerPitch = new SoccerPitch(cxClient, cyClient);
			CheckAllMenuItemsAppropriately(hwnd);

		}
		break;

		case WM_COMMAND: {
			
			switch (wParam) {

			case ID_AIDS_NOAIDS:

				Prm.bStates = 0;
				Prm.bRegions = 0;
				Prm.bIDs = 0;
				Prm.bSupportSpots = 0;
				Prm.bViewTargets = 0;

				CheckAllMenuItemsAppropriately(hwnd);

				break;

			case IDM_SHOW_REGIONS:

				Prm.bRegions = !Prm.bRegions;

				CheckAllMenuItemsAppropriately(hwnd);

				break;

			case IDM_SHOW_STATES:

				Prm.bStates = !Prm.bStates;

				CheckAllMenuItemsAppropriately(hwnd);

				break;

			case IDM_SHOW_IDS:

				Prm.bIDs = !Prm.bIDs;

				CheckAllMenuItemsAppropriately(hwnd);

				break;

			case IDM_AIDS_SUPPORTSPOTS:

				Prm.bSupportSpots = !Prm.bSupportSpots;

				CheckAllMenuItemsAppropriately(hwnd);

				break;

			case ID_AIDS_SHOWTARGETS:

				Prm.bViewTargets = !Prm.bViewTargets;

				CheckAllMenuItemsAppropriately(hwnd);

				break;

			case IDM_AIDS_HIGHLITE:

				Prm.bHighlightIfThreatened = !Prm.bHighlightIfThreatened;

				CheckAllMenuItemsAppropriately(hwnd);

				break;

			}

		}
		break;

		case WM_KEYUP: {

			switch(wParam) {

				case VK_ESCAPE:{
					SendMessage(hwnd, WM_DESTROY, NULL, NULL);
				}
				break;

				case 'R': {
					delete g_SoccerPitch;
					g_SoccerPitch = new SoccerPitch(cxClient, cyClient);
				}
				break;

				case 'P': {
					g_SoccerPitch->TogglePause();
				}
				break;

			}

		}

		case WM_PAINT: {

			PAINTSTRUCT ps;
			BeginPaint(hwnd, &ps);

			gdi->StartDrawing(hdcBackBuffer);
			g_SoccerPitch->Render();
			gdi->StopDrawing(hdcBackBuffer);

			//Now blit backbuffer to front
			BitBlt(ps.hdc, 0, 0, cxClient, cyClient, hdcBackBuffer, 0, 0, SRCCOPY);
			EndPaint(hwnd, &ps);

		}
		break;

		//Has the user resized the client area?
		case WM_SIZE: {

			//If so we need to update our variables so that any drawing we do using cxClient and cyClient is scaled accordingly.

			cxClient = LOWORD(lParam);
			cyClient = HIWORD(lParam);

			//Now to resize the back buffer accordingly. First select the old bitmap back into the DC.
			SelectObject(hdcBackBuffer, hOldBitmap);

			//Don't forget to do this or you will get resource leaks.
			DeleteObject(hBitmap);

			//Get the DC for the application.
			HDC hdc = GetDC(hwnd);

			//Create another bitmap of the same size and mode as the application.
			hBitmap = CreateCompatibleBitmap(hdc, cxClient, cyClient);
			ReleaseDC(hwnd, hdc);

			//Select the new bitmap into the DC
			SelectObject(hdcBackBuffer, hBitmap);

		}
		break;

		case WM_DESTROY: {

			//Clean up our backbuffer objects
			SelectObject(hdcBackBuffer, hOldBitmap);
			DeleteDC(hdcBackBuffer);
			DeleteObject(hBitmap);

			//Kill the application, this sends a WM_QUIT message.
			PostQuitMessage(0);

		}
		break;

	}

	//This is where all the messages not specifically handled by our windproc are sent to be processed.
	return DefWindowProc(hwnd, msg, wParam, lParam);

}

//----------------------------------------WndMain-----------------------------------------
//
// The entry point of the window program.
//
//----------------------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR szCmdLine, int iCmdShow) {

	//Handle to our window
	HWND hWnd;

	//Our window class structure
	WNDCLASSEX winclass;

	//First fill in the window class structure
	winclass.cbSize = sizeof(WNDCLASSEX);
	winclass.style = CS_HREDRAW | CS_VREDRAW;
	winclass.lpfnWndProc = WindowProc;
	winclass.cbClsExtra = 0;
	winclass.cbWndExtra = 0;
	winclass.hInstance = hInstance;
	winclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	winclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	winclass.hbrBackground = NULL;
	winclass.lpszMenuName = MAKEINTRESOURCE(IDR_MENU1);
	winclass.lpszClassName = g_szWindowClassName;
	winclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));

	//Register the window class
	if (!RegisterClassEx(&winclass)) {

		MessageBox(NULL, "Registration failed!", "Error", 0);
		return 0;

	}

	//Create the window and assign its ID to hwnd
	hWnd = CreateWindowEx(NULL, g_szWindowClassName, g_szApplicationName, WS_OVERLAPPED | WS_VISIBLE | WS_CAPTION | WS_SYSMENU, GetSystemMetrics(SM_CXSCREEN) / 2 - WindowWidth / 2, GetSystemMetrics(SM_CYSCREEN) / 2 - WindowHeight / 2, WindowWidth, WindowHeight, NULL, NULL, hInstance, NULL);

	//Mame sure the window creation has gone OK
	if(!hWnd) MessageBox(NULL, "CreateWindowwEx failed!", "Error", 0);

	//Start timer
	timer.Start();

	MSG msg;

	//Enter the message loop
	bool bDone = false;

	while (!bDone) {

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {

			//Stop loop if it's a quit message
			if (msg.message == WM_QUIT) bDone = true;
			else {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

		}

		if (timer.ReadyForNextFrame() && msg.message != WM_QUIT) {

			//Update
			g_SoccerPitch->Update();

			//Render
			RedrawWindow(hWnd, false);

			Sleep(2);

		}

	}

	delete g_SoccerPitch;
	UnregisterClass(g_szWindowClassName, winclass.hInstance);
	return msg.wParam;

}