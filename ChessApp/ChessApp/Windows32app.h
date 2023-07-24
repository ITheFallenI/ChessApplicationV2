#pragma once

#include <iostream>
#include "Windows.h"
#include <io.h>
#include <fcntl.h>
#include <d2d1.h>

class Windows32app
{
public:
	int Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
private:
	const D2D1_POINT_2F defaultWinSize = { 1280, 720 };
	const D2D1_POINT_2F minWinSize = { 1280, 720 };
};

