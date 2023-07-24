// ChessApp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Window32app.h"

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    Window32app app;
    return app.Run(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
}
