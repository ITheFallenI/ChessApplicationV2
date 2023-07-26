#pragma once

#include "HeaderCollection.h"
#include "ChessBoard.h"

class Window32app
{
public:
	int Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	static ID2D1Factory* d2dFactory; // Add this member for Direct2D factory
	static ID2D1HwndRenderTarget* renderTarget; // Add this member for the render target

	static D2D1_POINT_2F defaultWinSize;
	static D2D1_POINT_2F minWinSize;
	static HRESULT DirectXsetup(HWND hwnd);
	static void RenderBoard();
	static void LeftMouseDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void LeftMouseUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static void CleanUp();

	static void DrawBitmap(ID2D1RenderTarget* renderTarget, ID2D1Bitmap* bitmap, int x, int y, float width, float height);
};

