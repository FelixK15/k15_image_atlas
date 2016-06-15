#include <windows.h>
#include <stdio.h>
#include <time.h>

#define K15_IA_IMPLEMENTATION
#include "../K15_ImageAtlas.h"

#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")

#define K15_FALSE 0
#define K15_TRUE 1

typedef unsigned char bool8;
typedef unsigned char byte;
typedef unsigned int uint32;
typedef unsigned short uint16;
typedef unsigned char uint8;

typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

HDC textDC = 0;
HDC backbufferDC = 0;
HBITMAP backbufferBitmap = 0;
HBRUSH transparentBrush = 0;
HPEN redPen = 0;
HPEN greenPen = 0;
HPEN whitePen = 0;
HPEN magentaPen = 0;
bool appRunning = true;
uint32 screenWidth = 1024;
uint32 screenHeight = 768;

struct pos
{
	int x;
	int y;
	int width;
	int height;
};
K15_ImageAtlas atlas;
K15_ImageAtlas lastAtlas;

const uint32 numNodes = 200;
uint32 insertedNodes = 0;
pos positions[20];
pos posPlace[numNodes];
bool8 pressedLastFrame = K15_FALSE;
bool8 lastAtlasActive = K15_TRUE;

void saveAtlas()
{
	K15_IAImageNode* nodes = lastAtlas.imageNodes;
	K15_IASkyline* skylines = lastAtlas.skylines;
	K15_IARect* wastedSpaceRects = lastAtlas.wastedSpaceRects;

	memcpy(nodes, atlas.imageNodes, sizeof(K15_IAImageNode) * atlas.numImageNodes);
	memcpy(skylines, atlas.skylines, sizeof(K15_IASkyline) * atlas.numSkylines);
	memcpy(wastedSpaceRects, atlas.wastedSpaceRects, sizeof(K15_IARect) * atlas.numWastedSpaceRects);

	lastAtlas = atlas;
	lastAtlas.imageNodes = nodes;
	lastAtlas.skylines = skylines;
	lastAtlas.wastedSpaceRects = wastedSpaceRects;
}

void restoreAtlas()
{
	K15_IAImageNode* nodes = atlas.imageNodes;
	K15_IASkyline* skylines = atlas.skylines;
	K15_IARect* wastedSpaceRects = atlas.wastedSpaceRects;

	memcpy(nodes, lastAtlas.imageNodes, sizeof(K15_IAImageNode) * lastAtlas.numImageNodes);
	memcpy(skylines, lastAtlas.skylines, sizeof(K15_IASkyline) * lastAtlas.numSkylines);
	memcpy(wastedSpaceRects, lastAtlas.wastedSpaceRects, sizeof(K15_IARect) * lastAtlas.numWastedSpaceRects);

	atlas = lastAtlas;
	atlas.imageNodes = nodes;
	atlas.skylines = skylines;
	atlas.wastedSpaceRects = wastedSpaceRects;
}

void K15_WindowCreated(HWND p_HWND, UINT p_Message, WPARAM p_wParam, LPARAM p_lParam)
{

}

void K15_WindowClosed(HWND p_HWND, UINT p_Message, WPARAM p_wParam, LPARAM p_lParam)
{
	appRunning = false;
}

void K15_KeyInput(HWND p_HWND, UINT p_Message, WPARAM p_wParam, LPARAM p_lParam)
{
	bool8 wasDown = ((p_lParam & (1 << 30)) != 0);
	bool8 isDown = ((p_lParam & (1 << 31)) == 0);
	uint16 key = (uint16)p_wParam;

	if (isDown != wasDown)
	{
		if (key == VK_LEFT && !lastAtlasActive && isDown)
		{
			lastAtlasActive = K15_TRUE;
			insertedNodes -= 1;
			
			//copy last atlas to current atlas
			restoreAtlas();
		}
		else if (isDown)
		{
			pressedLastFrame = K15_TRUE;
			lastAtlasActive = K15_FALSE;
		}
	}
}

void K15_MouseButtonInput(HWND p_HWND, UINT p_Message, WPARAM p_wParam, LPARAM p_lParam)
{

}

void K15_MouseMove(HWND p_HWND, UINT p_Message, WPARAM p_wParam, LPARAM p_lParam)
{

}

void K15_MouseWheel(HWND p_HWND, UINT p_Message, WPARAM p_wParam, LPARAM p_lParam)
{

}

LRESULT CALLBACK K15_WNDPROC(HWND p_HWND, UINT p_Message, WPARAM p_wParam, LPARAM p_lParam)
{
	bool8 messageHandled = K15_FALSE;

	switch (p_Message)
	{
	case WM_CREATE:
		K15_WindowCreated(p_HWND, p_Message, p_wParam, p_lParam);
		break;

	case WM_CLOSE:
		K15_WindowClosed(p_HWND, p_Message, p_wParam, p_lParam);
		messageHandled = K15_TRUE;
		break;

	case WM_KEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYDOWN:
	case WM_SYSKEYUP:
		K15_KeyInput(p_HWND, p_Message, p_wParam, p_lParam);
		break;

	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP:
	case WM_XBUTTONUP:
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_XBUTTONDOWN:
		K15_MouseButtonInput(p_HWND, p_Message, p_wParam, p_lParam);
		break;

	case WM_MOUSEMOVE:
		K15_MouseMove(p_HWND, p_Message, p_wParam, p_lParam);
		break;

	case WM_MOUSEWHEEL:
		K15_MouseWheel(p_HWND, p_Message, p_wParam, p_lParam);
		break;
	}

	if (messageHandled == K15_FALSE)
	{
		return DefWindowProc(p_HWND, p_Message, p_wParam, p_lParam);
	}

	return 0;
}

HWND setupWindow(HINSTANCE p_Instance, int p_Width, int p_Height)
{
	WNDCLASS wndClass;
	ZeroMemory(&wndClass, sizeof(wndClass));

	wndClass.style = CS_HREDRAW | CS_OWNDC | CS_VREDRAW;
	wndClass.hInstance = p_Instance;
	wndClass.lpszClassName = "K15_Win32Template";
	wndClass.lpfnWndProc = K15_WNDPROC;
	RegisterClass(&wndClass);

	HWND hwnd = CreateWindowA("K15_Win32Template", "Win32 Template",
		WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		p_Width, p_Height, 0, 0, p_Instance, 0);

	if (hwnd == INVALID_HANDLE_VALUE)
		MessageBox(0, "Error creating Window.\n", "Error!", 0);
	else
		ShowWindow(hwnd, SW_SHOW);

	return hwnd;
}

uint32 getTimeInMilliseconds(LARGE_INTEGER p_PerformanceFrequency)
{
	LARGE_INTEGER appTime = {};
	QueryPerformanceCounter(&appTime);

	appTime.QuadPart *= 1000; //to milliseconds

	return (uint32)(appTime.QuadPart / p_PerformanceFrequency.QuadPart);
}

int sortFnc(const void* p1, const void* p2)
{
	pos* _p1 = (pos*)p1;
	pos* _p2 = (pos*)p2;

	int _ap1 = _p1->x * _p1->y;
	int _ap2 = _p2->x * _p2->y;

	return _ap2 - _ap1;
}

void setup(HWND p_HWND)
{
	srand((uint32)time(NULL));

	HDC originalDC = GetDC(p_HWND);
	textDC = CreateCompatibleDC(originalDC);
	backbufferDC = CreateCompatibleDC(originalDC);
	backbufferBitmap = CreateCompatibleBitmap(originalDC, screenWidth, screenHeight);

	SetTextColor(backbufferDC, RGB(255, 255, 255));
	SetBkColor(backbufferDC, RGB(0, 0, 0));

	SelectObject(backbufferDC, backbufferBitmap);

	if (K15_IACreateAtlas(&atlas, numNodes) != K15_IA_RESULT_SUCCESS)
		MessageBox(0, "Error creating atlas!", "Error", 0);

	if (K15_IACreateAtlas(&lastAtlas, numNodes) != K15_IA_RESULT_SUCCESS)
		MessageBox(0, "Error creating atlas!", "Error", 0);

	redPen = CreatePen(0, 1, RGB(255, 0, 0));
	greenPen = CreatePen(0, 1, RGB(0, 255, 0));
	whitePen = CreatePen(0, 1, RGB(255, 255, 255));
	magentaPen = CreatePen(0, 4, RGB(255, 0, 255));
	transparentBrush = CreateSolidBrush(TRANSPARENT);

	for (uint32 nodeIndex = 0;
		nodeIndex < numNodes;
		++nodeIndex)
	{
		posPlace[nodeIndex].x = rand() % 45 + 5;
		posPlace[nodeIndex].y = rand() % 45 + 5;
	}

	qsort(posPlace, numNodes, sizeof(pos), sortFnc);

	return;
}

void swapBuffers(HWND p_HWND)
{
	HDC originalDC = GetDC(p_HWND);

	//blit to front buffer
	BitBlt(originalDC, 0, 0, screenWidth, screenHeight, backbufferDC, 0, 0, SRCCOPY);

	//clear backbuffer
	BitBlt(backbufferDC, 0, 0, screenWidth, screenHeight, backbufferDC, 0, 0, BLACKNESS);
}

void doFrame(HWND p_HWND)
{
	uint32 deltaHeight = screenHeight - atlas.height;
	uint32 deltaWidth = screenWidth - atlas.width;

	if (pressedLastFrame && insertedNodes != numNodes)
	{
		int width = posPlace[insertedNodes].x;
		int height = posPlace[insertedNodes].y;
		
		saveAtlas();

		positions[insertedNodes].width = width;
		positions[insertedNodes].height = height;
		K15_IAAddImageToAtlas(&atlas, KIA_PIXEL_FORMAT_R8G8B8A8, (kia_byte*)2, width, height, &positions[insertedNodes].x, &positions[insertedNodes].y);
		++insertedNodes;
	}

	SelectObject(backbufferDC, transparentBrush);
	SelectObject(backbufferDC, greenPen);
	Rectangle(backbufferDC, deltaWidth / 2, deltaHeight / 2, screenWidth - deltaWidth / 2, screenHeight - deltaHeight / 2);

	SelectObject(backbufferDC, whitePen);
	for (uint32 nodeIndex = 0;
	nodeIndex < insertedNodes;
		++nodeIndex)
	{
		HBRUSH tempBrush = CreateSolidBrush(RGB(24, 200, 200));
		SelectObject(backbufferDC, tempBrush);
		uint32 posX = deltaWidth / 2 + positions[nodeIndex].x;
		uint32 posY = deltaHeight / 2 + positions[nodeIndex].y;
		uint32 width = positions[nodeIndex].width;
		uint32 height = positions[nodeIndex].height;

		Rectangle(backbufferDC, posX, posY, posX + width, posY + height);
		DeleteObject(tempBrush);
	}

	SelectObject(backbufferDC, whitePen);
	for (uint32 rectIndex = 0;
		rectIndex < atlas.numWastedSpaceRects;
		++rectIndex)
	{
		HBRUSH tempBrush = CreateSolidBrush(RGB(255, 200, 24));
		SelectObject(backbufferDC, tempBrush);
		uint32 posX = deltaWidth / 2 + atlas.wastedSpaceRects[rectIndex].posX;
		uint32 posY = deltaHeight / 2 + atlas.wastedSpaceRects[rectIndex].posY;
		uint32 width = atlas.wastedSpaceRects[rectIndex].width;
		uint32 height = atlas.wastedSpaceRects[rectIndex].height;

		Rectangle(backbufferDC, posX, posY, posX + width, posY + height);
		DeleteObject(tempBrush);
	}

	SelectObject(backbufferDC, magentaPen);
	for (uint32 skylineIndex = 0;
		skylineIndex < atlas.numSkylines;
		++skylineIndex)
	{
		K15_IASkyline* skyline = atlas.skylines + skylineIndex;

		uint32 posX = skyline->baseLinePosX + deltaWidth / 2;
		uint32 posY = skyline->baseLinePosY + deltaHeight / 2;

		Rectangle(backbufferDC, posX, posY + 1, posX + skyline->baseLineWidth, posY);
	}

	RECT widthTextRect;
	widthTextRect.left = screenWidth / 2 - 50;
	widthTextRect.right = screenWidth / 2 + 50;
	widthTextRect.top = deltaHeight / 2 - 30;
	widthTextRect.bottom = deltaHeight / 2;

	RECT heightTextRect;
	heightTextRect.left = screenWidth / 2 + atlas.width / 2;
	heightTextRect.right = heightTextRect.left + 100;
	heightTextRect.top = screenHeight / 2 - 20;
	heightTextRect.bottom = screenHeight/ 2 + 20;

	char buffer[256];
	sprintf_s(buffer, 256, "Width: %dpx", atlas.width);
	DrawTextA(backbufferDC, buffer, -1, &widthTextRect, DT_CENTER);

	sprintf_s(buffer, 256, "Height: %dpx", atlas.height);
	DrawTextA(backbufferDC, buffer, -1, &heightTextRect, DT_CENTER);

	RECT textRect;
	textRect.left = 70;
	textRect.top = 200;
	textRect.bottom = screenHeight;
	textRect.right = screenWidth;

	char messageBuffer[255];
	sprintf_s(messageBuffer, 255, "NumSkylines: %d", atlas.numSkylines);
	DrawTextA(backbufferDC, messageBuffer, -1, &textRect, DT_LEFT | DT_TOP);

	textRect.top = 220;

	sprintf_s(messageBuffer, 255, "NumWastedAreaRects: %d", atlas.numWastedSpaceRects);
	DrawTextA(backbufferDC, messageBuffer, -1, &textRect, DT_LEFT | DT_TOP);


	pressedLastFrame = K15_FALSE;

	swapBuffers(p_HWND);
}

int CALLBACK WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nShowCmd)
{
	LARGE_INTEGER performanceFrequency;
	QueryPerformanceFrequency(&performanceFrequency);

	HWND hwnd = setupWindow(hInstance, screenWidth, screenHeight);

	if (hwnd == INVALID_HANDLE_VALUE)
		return -1;

	setup(hwnd);

	uint32 timeFrameStarted = 0;
	uint32 timeFrameEnded = 0;
	uint32 deltaMs = 0;
	MSG msg;

	while (appRunning)
	{
		timeFrameStarted = getTimeInMilliseconds(performanceFrequency);

		while (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		doFrame(hwnd);

		timeFrameEnded = getTimeInMilliseconds(performanceFrequency);
		deltaMs = timeFrameEnded - timeFrameStarted;		
	}

	return 0;
}