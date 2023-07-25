#include "Window32app.h"

ID2D1Factory* Window32app::d2dFactory = nullptr;
ID2D1HwndRenderTarget* Window32app::renderTarget = nullptr;


D2D1_POINT_2F Window32app::defaultWinSize = { 1280, 720 };
D2D1_POINT_2F Window32app::minWinSize = { 1280, 720 };

ChessBoard chessBoard;
BoardPieces boardPieces;

int selectedtileX = 0;
int selectedtileY = 0;

int Window32app::Run(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    OutputDebugStringW(L"Init run. Creating Window...\n");
    // Init win32
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    if (FAILED(hr))
        return 0;

    //calculate  center screen
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    int windowX = (screenWidth - Window32app::defaultWinSize.x) / 2; // Center the window horizontally
    int windowY = (screenHeight - Window32app::defaultWinSize.y) / 2; // Center the window vertically


    // Create the window
    HWND hwnd;
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ChessBoardWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        0,
        L"ChessBoardWindowClass",
        L"DirectX Chess Board",
        WS_OVERLAPPEDWINDOW,
        windowX, windowY, Window32app::defaultWinSize.x, Window32app::defaultWinSize.y,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (!hwnd)
        return 0;

    OutputDebugStringW(L"Window created!\n");

    // Show the window
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);


    Window32app::DirectXsetup(hwnd);

    // Enter the message loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

LRESULT Window32app::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
    {
        return 0;
    }
    case WM_PAINT:
    {
        Window32app::RenderBoard();
        return 0;
    }   
    case WM_GETMINMAXINFO:
    {
        // Limit the minimum size of the window
        MINMAXINFO* pMinMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);
        pMinMaxInfo->ptMinTrackSize.x = Window32app::minWinSize.x; // Set your desired minimum width
        pMinMaxInfo->ptMinTrackSize.y = Window32app::minWinSize.y; // Set your desired minimum height
        return 0;
    }
    case WM_SIZE:
    {
        int newWidth = LOWORD(lParam);
        int newHeight = HIWORD(lParam);

        if (renderTarget)
        {
            renderTarget->Resize(D2D1::SizeU(newWidth, newHeight));
            // No need to recreate the render target, just resize it.
        }

        // Redraw the window after resizing
        InvalidateRect(hwnd, NULL, FALSE);

        break;
    }
    case WM_LBUTTONDOWN:
    {
        LeftClickFunction(hwnd, uMsg, wParam, lParam);
        return 0;
    }
    case WM_KEYDOWN:
    {
        if (wParam == VK_ESCAPE) // Check if the pressed key is the "Escape" key
        {
            PostMessage(hwnd, WM_CLOSE, 0, 0); // Send WM_CLOSE message to initiate window close
        }
        break;
    }
    case WM_DESTROY:
    {
        CleanUp();
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

HRESULT Window32app::DirectXsetup(HWND hwnd)
{
    // Initialize Direct2D factory
    D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &d2dFactory);

    // Get the size of the client area to create the render target
    RECT rc;
    GetClientRect(hwnd, &rc);

    // Create Direct2D render target
    d2dFactory->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)),
        &renderTarget
    );

    OutputDebugStringW(L"Direct X context created.\n");

    OutputDebugStringW(L"Creating Brushes for each tile.\n");

    for (int row = 0; row < chessBoard.GetRows(); row++)
    {
        for (int col = 0; col < chessBoard.GetColumns(); col++)
        {
            auto& tile = chessBoard.board[col][row];
            HRESULT hr = renderTarget->CreateSolidColorBrush(tile.currentColor, &tile.pBrush);
        }
    }

    wchar_t buffer[MAX_PATH];
    GetModuleFileName(GetModuleHandle(NULL), buffer, MAX_PATH);

    // Extract the path from the buffer
    wchar_t* lastBackslash = wcsrchr(buffer, L'\\');
    if (lastBackslash != NULL) {
        *lastBackslash = L'\0'; // Null-terminate the string at the last backslash
    }


    wchar_t pawnTest_w_imagePath[MAX_PATH];
    wcscpy_s(pawnTest_w_imagePath, MAX_PATH, buffer);
    wcscat_s(pawnTest_w_imagePath, MAX_PATH, L"\\img\\pawn_w.png");
    HRESULT hr1 = ChessBoard::LoadChessPieceTexture(renderTarget, pawnTest_w_imagePath, &ChessBoard::pPawnBitmap_w);

    if (SUCCEEDED(hr1))
    {
        // Successfully loaded the white pawn bitmap
        OutputDebugStringW(L"pawn_w.png success!\n");
    }
    else
    {
        OutputDebugStringW(L"pawn_w.png failed! \n");
        // Failed to load the white pawn bitmap
    }


    wchar_t pawnTest_b_imagePath[MAX_PATH];
    wcscpy_s(pawnTest_b_imagePath, MAX_PATH, buffer);
    wcscat_s(pawnTest_b_imagePath, MAX_PATH, L"\\img\\pawn_b.png");
    HRESULT hr2 = ChessBoard::LoadChessPieceTexture(renderTarget, pawnTest_b_imagePath, &ChessBoard::pPawnBitmap_b);

    if (SUCCEEDED(hr2))
    {
        // Successfully loaded the white pawn bitmap
        OutputDebugStringW(L"pawn_b.png success! \n");
    }
    else
    {
        OutputDebugStringW(L"pawn_b.png failed! \n");
        // Failed to load the white pawn bitmap
    }


    ChessPiece newP;

    newP.team = TileTeam::WHITE;
    newP.type = TileType::PAWN;
    newP.bitmap = ChessBoard::pPawnBitmap_w;
    newP.x = 5;
    newP.y = 5;
    newP.tileSize = 60;

    boardPieces.pieces[newP.x][newP.y] = newP;

    return S_OK;
}

void Window32app::LeftClickFunction(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int mouseX = GET_X_LPARAM(lParam); // Get the X position of the mouse cursor
    int mouseY = GET_Y_LPARAM(lParam); // Get the Y position of the mouse cursor

    const std::wstring rawMouseCoords = L"Mouse X: " + std::to_wstring(mouseX) + L", Mouse y:" + std::to_wstring(mouseY) + L"\n";
    OutputDebugStringW(rawMouseCoords.c_str());

    for (int row = 0; row < chessBoard.GetRows(); row++)
    {
        for (int col = 0; col < chessBoard.GetColumns(); col++)
        {
            auto& tile = chessBoard.board[col][row];
            if (chessBoard.IsPointInsidePolygon(mouseX, mouseY, tile.polygon.x0, tile.polygon.y0, tile.polygon.x1, tile.polygon.y1, tile.polygon.x2, tile.polygon.y2, tile.polygon.x3, tile.polygon.y3)) {
                selectedtileX = tile.x;
                selectedtileY = tile.y;
                const std::wstring clickedTileDATA = L"Clicked tile: X: " + std::to_wstring(tile.x) + L",  Y:" + std::to_wstring(tile.y) + L"\n";
                OutputDebugStringW(clickedTileDATA.c_str());
                break;
            }
        }
    }


}

void Window32app::CleanUp()
{
    for (int row = 0; row < chessBoard.GetRows(); row++)
    {
        for (int col = 0; col < chessBoard.GetColumns(); col++)
        {
            auto& tile = chessBoard.board[col][row];

            if (tile.pBrush != nullptr) {
                tile.pBrush->Release();
                tile.pBrush = nullptr;
            }
        }
    }

    OutputDebugStringW(L"Cleanup all brushes.\n");

    if (ChessBoard::pPawnBitmap_w)
    {
        ChessBoard::pPawnBitmap_w->Release();
        ChessBoard::pPawnBitmap_w = nullptr;
    }

    if (ChessBoard::pPawnBitmap_b)
    {
        ChessBoard::pPawnBitmap_b->Release();
        ChessBoard::pPawnBitmap_b = nullptr;
    }

    OutputDebugStringW(L"Cleanup all textures.\n");
}


// Function to draw a bitmap at a specific position on the render target
void Window32app::DrawBitmap(ID2D1RenderTarget* renderTarget, ID2D1Bitmap* bitmap, int x, int y, float width, float height)
{
    if (bitmap)
    {
        D2D1_RECT_F destRect = D2D1::RectF(chessBoard.board[y][x].polygon.x0, chessBoard.board[y][x].polygon.y0, 
            chessBoard.board[y][x].polygon.x0 + width, chessBoard.board[y][x].polygon.y0 + height);
        renderTarget->DrawBitmap(bitmap, destRect);
    }
}


void Window32app::RenderBoard()
{
    if (!renderTarget)
        return;

    if (renderTarget)
    {
        D2D1_SIZE_F renderTargetSize = renderTarget->GetSize();
        int rows = chessBoard.GetRows();
        int cols = chessBoard.GetColumns();
        int minDimension = static_cast<int>(min(renderTargetSize.width, renderTargetSize.height));

        int padding = 100; // Padding on all sides (adjust this value as desired)
        int chessboardSize = minDimension - (2 * padding);

        // Calculate the size of each tile based on the available space and padding
        float tileSize = static_cast<float>(chessboardSize) / static_cast<float>(max(rows, cols));// / 1.25f

        // Calculate the total size of the chessboard including padding
        float chessboardWidth = tileSize * cols + (2 * padding);
        float chessboardHeight = tileSize * rows + (2 * padding);

        // Calculate the position to center the chessboard within the render target
        float chessboardX = (renderTargetSize.width - chessboardWidth) / 2;
        float chessboardY = (renderTargetSize.height - chessboardHeight) / 2;

        // Clear the render target
        renderTarget->BeginDraw();
        renderTarget->Clear(D2D1::ColorF(D2D1::ColorF(0.15, 0.15, 0.15, 1)));


        for (int row = 0; row < chessBoard.GetRows(); row++)
        {
            for (int col = 0; col < chessBoard.GetColumns(); col++)
            {
                auto& tile = chessBoard.board[col][row];

                float x = chessboardX + padding + tile.x * tileSize;
                float y = chessboardY + padding + tile.y * tileSize;

                tile.tileSize = tileSize;

                tile.polygon = TilePolygon{
                    x, y,                  // Top-left
                    x + tileSize, y,       // Top-right
                    x + tileSize, y + tileSize, // Bottom-right
                    x, y + tileSize        // Bottom-left
                };


                renderTarget->FillRectangle(D2D1::RectF(x, y, x + tileSize, y + tileSize), tile.pBrush);

                if (tile.x == chessBoard.board[selectedtileY][selectedtileX].x && tile.y == chessBoard.board[selectedtileY][selectedtileX].y && selectedtileX >= 0 && selectedtileY >= 0) {

                    // Create a solid color brush for the dots
                    ID2D1SolidColorBrush* dotBrush = nullptr;
                    HRESULT hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(0, 1.0f, 0), &dotBrush);
                    if (FAILED(hr))
                    {
                        // Handle the error if necessary
                    }

                    // Draw four dots for each point of the tile polygon
                    D2D1_ELLIPSE dotEllipse;
                    float dotRadius = 4.0f; // Adjust the dot radius as needed

                    // Draw dot for point (x0, y0)
                    dotEllipse.point = D2D1::Point2F(tile.polygon.x0, tile.polygon.y0);
                    dotEllipse.radiusX = dotEllipse.radiusY = dotRadius;
                    renderTarget->FillEllipse(dotEllipse, dotBrush);

                    // Draw dot for point (x1, y1)
                    dotEllipse.point = D2D1::Point2F(tile.polygon.x1, tile.polygon.y1);
                    renderTarget->FillEllipse(dotEllipse, dotBrush);

                    // Draw dot for point (x2, y2)
                    dotEllipse.point = D2D1::Point2F(tile.polygon.x2, tile.polygon.y2);
                    renderTarget->FillEllipse(dotEllipse, dotBrush);

                    // Draw dot for point (x3, y3)
                    dotEllipse.point = D2D1::Point2F(tile.polygon.x3, tile.polygon.y3);
                    renderTarget->FillEllipse(dotEllipse, dotBrush);

                    // Release the dot brush after use
                    dotBrush->Release();
                    dotBrush = nullptr;
                }
            }
        }

        for (int row = 0; row < chessBoard.GetRows(); row++)
        {
            for (int col = 0; col < chessBoard.GetColumns(); col++)
            {
                if (boardPieces.pieces[col][row].type != TileType::NONE) {
                    //OutputDebugStringW(L"FOUND A PIECE!.\n");
                    //DrawBitmap(renderTarget, ChessBoard::pPawnBitmap_w, boardPieces.pieces[col][row].x, boardPieces.pieces[col][row].y, tileSize, tileSize);
                    DrawBitmap(renderTarget, ChessBoard::pPawnBitmap_w, selectedtileX, selectedtileY, tileSize, tileSize);
                }
            }
        }

        /*
    for (size_t i = 0; i < chessBoard.pieces.size(); i++)
    {
        if (chessBoard.pieces[i].bitmap == nullptr) {
            OutputDebugStringW(L"NULLPTR BITMAP.\n");
        }
        else {
            std::wstring test = L"Bitmap here, bitmap width = " + std::to_wstring(chessBoard.pieces[i].bitmap->GetSize().width);
            OutputDebugStringW(test.c_str());
        }

        //std::wstring tester = L"Piece = (" + std::to_wstring(i) + L") x: " + std::to_wstring(chessBoard.pieces[i].x);
        //OutputDebugStringW(tester.c_str());


        DrawBitmap(renderTarget, ChessBoard::pPawnBitmap_w, chessBoard.pieces[i].x,
            chessBoard.pieces[i].y, tileSize, tileSize);
    }*/
    //DrawBitmap(renderTarget, ChessBoard::pPawnBitmap_w, chessBoard.board[0][0].polygon.x0, chessBoard.board[0][0].polygon.y0, tileSize, tileSize);

    // End drawing
        renderTarget->EndDraw();
    }
}