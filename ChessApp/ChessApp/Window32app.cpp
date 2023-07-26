#include "Window32app.h"

ID2D1Factory* Window32app::d2dFactory = nullptr;
ID2D1HwndRenderTarget* Window32app::renderTarget = nullptr;


D2D1_POINT_2F Window32app::defaultWinSize = { 1280, 720 };
D2D1_POINT_2F Window32app::minWinSize = { 1280, 720 };
ChessBoard board;

ChessPiece* draggablePiece = nullptr;
bool dragging = false;

int selectedtileCol = -1;
int selectedtileRow = -1;

TileTeam myTeam = TileTeam::WHITE;

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
    wc.lpszClassName = L"boardWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClass(&wc);

    hwnd = CreateWindowEx(
        0,
        L"boardWindowClass",
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
    case WM_MOUSEMOVE: {
        if (dragging) {
            if (draggablePiece != nullptr) {
                //OutputDebugStringW(L"draggable piece! here.\n");
                draggablePiece->row = static_cast<float>(LOWORD(lParam));
                draggablePiece->col = static_cast<float>(HIWORD(lParam));
            }
            InvalidateRect(hwnd, NULL, FALSE);
        }

        return 0;
    }
    case WM_LBUTTONUP: {
        LeftMouseUp(hwnd, uMsg, wParam, lParam);
        InvalidateRect(hwnd, NULL, FALSE);
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
        LeftMouseDown(hwnd, uMsg, wParam, lParam);
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

    for (int row = 0; row < board.GetRows(); row++)
    {
        for (int col = 0; col < board.GetColumns(); col++)
        {
            auto& tile = board.tile[col][row];
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

    //Window32app::LoadTexturePiece(L"\\img\\pawn_b.png", &ChessBoard::pPawnBitmap_b);
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


    wchar_t knight_b_imagePath[MAX_PATH];
    wcscpy_s(knight_b_imagePath, MAX_PATH, buffer);
    wcscat_s(knight_b_imagePath, MAX_PATH, L"\\img\\knight_b.png");
    HRESULT hr3 = ChessBoard::LoadChessPieceTexture(renderTarget, knight_b_imagePath, &ChessBoard::pKnightBitmap_b);

    if (SUCCEEDED(hr3))
    {
        // Successfully loaded the white pawn bitmap
        OutputDebugStringW(L"knight_b.png success! \n");
    }
    else
    {
        OutputDebugStringW(L"knight_b.png failed! \n");
        // Failed to load the white pawn bitmap
    }

    wchar_t knight_w_imagePath[MAX_PATH];
    wcscpy_s(knight_w_imagePath, MAX_PATH, buffer);
    wcscat_s(knight_w_imagePath, MAX_PATH, L"\\img\\knight_w.png");
    HRESULT hr4 = ChessBoard::LoadChessPieceTexture(renderTarget, knight_w_imagePath, &ChessBoard::pKnightBitmap_w);

    if (SUCCEEDED(hr4))
    {
        // Successfully loaded the white pawn bitmap
        OutputDebugStringW(L"knight_w.png success! \n");
    }
    else
    {
        OutputDebugStringW(L"knight_w.png failed! \n");
        // Failed to load the white pawn bitmap
    }


    AddDefaultBoard();

    return S_OK;
}

void Window32app::LeftMouseDown(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    int mouseX = GET_X_LPARAM(lParam); // Get the X position of the mouse cursor
    int mouseY = GET_Y_LPARAM(lParam); // Get the Y position of the mouse cursor

    const std::wstring rawMouseCoords = L"Mouse Down: Mouse X: " + std::to_wstring(mouseX) + L", Mouse y:" + std::to_wstring(mouseY) + L"\n";
    OutputDebugStringW(rawMouseCoords.c_str());

    for (int row = board.GetRows() - 1; row >= 0; row--)
    {
        for (int col = 0; col < board.GetColumns(); col++)
        {
            auto& tile = board.tile[col][row];
            if (board.IsPointInsidePolygon(mouseX, mouseY, tile.polygon.x0, tile.polygon.y0, tile.polygon.x1, tile.polygon.y1, tile.polygon.x2, tile.polygon.y2, tile.polygon.x3, tile.polygon.y3)) 
            {
                const std::wstring clickedTileDATA = L"Clicked tile: X: " + std::to_wstring(tile.col) + L",  Y:" + std::to_wstring(tile.row) + L"\n";
                OutputDebugStringW(clickedTileDATA.c_str());                
                
                if (tile.piece != nullptr && !dragging) {
                        OutputDebugStringW(L"Has a piece on.\n");
                        if (tile.piece->type == TileType::PAWN) {
                            OutputDebugStringW(L"we're a pawn, get valid pawn moves.\n");
                            //we're a pawn check for valid pawn moves..
                            validTiles = getValidPawnMove(tile, board);
                        }
                        else if (tile.piece->type == TileType::KNIGHT) {
                            OutputDebugStringW(L"we're a knight, get valid knight moves.\n");
                            validTiles = getValidKnightMove(tile, board);
                        }
                        else {
                            validTiles.clear();
                        }
                        selectedtileCol = col;
                        selectedtileRow = row;

                        tile.piece->col = mouseY;
                        tile.piece->row = mouseX;
                        draggablePiece = tile.piece;
                        tile.piece = nullptr;
                        dragging = true;
                    break;
                }
                else {
                    OutputDebugStringW(L"no piece, (nullptr)\n");
                }
                break;
            }
        }
    }

}

void Window32app::LeftMouseUp(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
    int mouseX = GET_X_LPARAM(lParam); // Get the X position of the mouse cursor
    int mouseY = GET_Y_LPARAM(lParam); // Get the Y position of the mouse cursor

    const std::wstring rawMouseCoords = L"Mouse Up: Mouse X: " + std::to_wstring(mouseX) + L", Mouse y:" + std::to_wstring(mouseY) + L"\n";
    OutputDebugStringW(rawMouseCoords.c_str());

    for (int row = board.GetRows() - 1; row >= 0; row--)
    {
        for (int col = 0; col < board.GetColumns(); col++)
        {
            auto& tile = board.tile[col][row];
            if (board.IsPointInsidePolygon(mouseX, mouseY, tile.polygon.x0, tile.polygon.y0, tile.polygon.x1, tile.polygon.y1, tile.polygon.x2, tile.polygon.y2, tile.polygon.x3, tile.polygon.y3)) 
            {
                // Check if the current tile is in the validTiles list
                auto it = std::find(validTiles.begin(), validTiles.end(), tile);

                if (it != validTiles.end()) {
                    //if slot is valid move check to go here
                    if (dragging && draggablePiece && !tile.piece) {
                        //doesn't have a piece on slot
                        OutputDebugStringW(L"Placed on valid tile.\n\n");

                        draggablePiece->col = tile.col;
                        draggablePiece->row = tile.row;

                        selectedtileCol = tile.col;
                        selectedtileRow = tile.row;

                        tile.piece = draggablePiece;
                        draggablePiece = nullptr;
                        dragging = false;
                        validTiles.clear();
                        break;
                    }
                    else if (dragging && draggablePiece && tile.piece) {
                        //has a piece on slot
                        OutputDebugStringW(L"Placed on valid tile but it has a piece, lets take it?.\n\n");
                        draggablePiece->col = tile.col;
                        draggablePiece->row = tile.row;

                        selectedtileCol = tile.col;
                        selectedtileRow = tile.row;

                         //should check here is takeable really
                        delete tile.piece;
                        tile.piece = nullptr;
                        tile.piece = draggablePiece;
                        draggablePiece = nullptr;
                        dragging = false;
                        validTiles.clear();

                    }
                }else{
                    if (dragging && draggablePiece) {
                        draggablePiece->col = selectedtileCol;
                        draggablePiece->row = selectedtileRow;
                        board.tile[selectedtileCol][selectedtileRow].piece = draggablePiece;
                        draggablePiece = nullptr;
                        dragging = false;
                    }
                    OutputDebugStringW(L"Place Error: invalid tile.\n\n");
                }
            }
        }
    }

    if (dragging && draggablePiece) {
        draggablePiece->col = selectedtileCol;
        draggablePiece->row = selectedtileRow;
        board.tile[selectedtileCol][selectedtileRow].piece = draggablePiece;
        draggablePiece = nullptr;
        dragging = false;
    }
}



void Window32app::CleanUp()
{
    for (int row = 0; row < board.GetRows(); row++)
    {
        for (int col = 0; col < board.GetColumns(); col++)
        {
            auto& tile = board.tile[col][row];

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

    for (int row = board.GetRows() - 1; row >= 0; row--)
    {
        for (int col = 0; col < board.GetColumns(); col++)
        {

            auto& tile = board.tile[col][row];

            if (tile.piece != nullptr) {
                delete board.tile[col][row].piece;
                board.tile[col][row].piece = nullptr;
                OutputDebugStringW(L"Pointer destroyed.\n");
            }
        }
    }

    if (draggablePiece) {
        delete draggablePiece;
        draggablePiece = nullptr;
    }
    OutputDebugStringW(L"Cleanup all piece pointers.\n");
}


// Function to draw a bitmap at a specific position on the render target
void Window32app::DrawBitmap(ID2D1RenderTarget* renderTarget, ID2D1Bitmap* bitmap, int x, int y, float width, float height)
{
    if (bitmap)
    {
        auto& tile = board.tile[y][x];
        float posX = tile.polygon.x0 + (tile.piece ? static_cast<float>(tile.piece->row - x) : 0.0f);
        float posY = tile.polygon.y0 + (tile.piece ? static_cast<float>(tile.piece->col - y) : 0.0f);

        D2D1_RECT_F destRect = D2D1::RectF(posX, posY, posX + width, posY + height);
        renderTarget->DrawBitmap(bitmap, destRect);
    }
}

void Window32app::AddDefaultBoard() 
{
    ChessPiece* newP = new ChessPiece;

    newP->team = TileTeam::WHITE;
    newP->type = TileType::PAWN;
    newP->bitmap = ChessBoard::pPawnBitmap_w;
    newP->col = 1;
    newP->row = 0;
    newP->tileSize = 60;

    board.tile[newP->col][newP->row].piece = newP;


    ChessPiece* newP2 = new ChessPiece;

    newP2->team = TileTeam::WHITE;
    newP2->type = TileType::PAWN;
    newP2->bitmap = ChessBoard::pPawnBitmap_w;
    newP2->col = 4;
    newP2->row = 3;
    newP2->tileSize = 60;

    board.tile[newP2->col][newP2->row].piece = newP2;


    ChessPiece* newK1 = new ChessPiece;

    newK1->team = TileTeam::WHITE;
    newK1->type = TileType::KNIGHT;
    newK1->bitmap = ChessBoard::pKnightBitmap_w;
    newK1->col = 5;
    newK1->row = 5;
    newK1->tileSize = 60;

    board.tile[newK1->col][newK1->row].piece = newK1;


    //black
    ChessPiece* newP_b = new ChessPiece;

    newP_b->team = TileTeam::BLACK;
    newP_b->type = TileType::PAWN;
    newP_b->bitmap = ChessBoard::pPawnBitmap_b;
    newP_b->col = 1;
    newP_b->row = 6;
    newP_b->tileSize = 60;


    board.tile[newP_b->col][newP_b->row].piece = newP_b;


    ChessPiece* newP_b2 = new ChessPiece;

    newP_b2->team = TileTeam::BLACK;
    newP_b2->type = TileType::PAWN;
    newP_b2->bitmap = ChessBoard::pPawnBitmap_b;
    newP_b2->col = 2;
    newP_b2->row = 6;
    newP_b2->tileSize = 60;


    board.tile[newP_b2->col][newP_b2->row].piece = newP_b2;
}


void Window32app::DeleteAllPieces()
{
    for (size_t i = 0; i < board.GetRows(); i++)
    {
        for (size_t i = 0; i < board.GetColumns(); i++)
        {
            delete board.tile[i]->piece;
            board.tile[i]->piece = nullptr;
        }
    }
}
/*
void Window32app::LoadTexturePiece(std::wstring pngFile, ID2D1Bitmap** ppBitmap) {

    wchar_t buffer[MAX_PATH];
    GetModuleFileName(GetModuleHandle(NULL), buffer, MAX_PATH);

    wchar_t pawnTest_b_imagePath[MAX_PATH];
    wcscpy_s(pawnTest_b_imagePath, MAX_PATH, buffer);
    wcscat_s(pawnTest_b_imagePath, MAX_PATH, pngFile.c_str());
    HRESULT hr2 = ChessBoard::LoadChessPieceTexture(renderTarget, pawnTest_b_imagePath, ppBitmap);

    if (SUCCEEDED(hr2))
    {
        // Successfully loaded the white pawn bitmap
        OutputDebugStringW(L"success! \n");
    }
    else
    {
        OutputDebugStringW(L"failed! \n");
        // Failed to load the white pawn bitmap
    }
}
*/
void Window32app::RenderBoard()
{
    if (!renderTarget)
        return;

    if (renderTarget)
    {
        D2D1_SIZE_F renderTargetSize = renderTarget->GetSize();
        int rows = board.GetRows();
        int cols = board.GetColumns();
        int minDimension = static_cast<int>(min(renderTargetSize.width, renderTargetSize.height));

        int padding = 100; // Padding on all sides (adjust this value as desired)
        int boardSize = minDimension - (2 * padding);

        // Calculate the size of each tile based on the available space and padding
        float tileSize = static_cast<float>(boardSize) / static_cast<float>(max(rows, cols));// / 1.25f

        // Calculate the total size of the board including padding
        float boardWidth = tileSize * cols + (2 * padding);
        float boardHeight = tileSize * rows + (2 * padding);

        // Calculate the position to center the board within the render target
        float boardX = (renderTargetSize.width - boardWidth) / 2;
        float boardY = (renderTargetSize.height - boardHeight) / 2 + boardHeight - tileSize; // Shift the board down

        // Clear the render target
        renderTarget->BeginDraw();
        renderTarget->Clear(D2D1::ColorF(D2D1::ColorF(0.15, 0.15, 0.15, 1)));


        for (int row = board.GetRows() - 1; row >= 0; row--)
        {
            for (int col = 0; col < board.GetColumns(); col++)
            {
                auto& tile = board.tile[col][row];

                float x = boardX + padding + tile.col * tileSize;
                float y = boardY - padding - tile.row * tileSize;

                tile.polygon = TilePolygon{
                    x, y,                  // Top-left
                    x + tileSize, y,       // Top-right
                    x + tileSize, y + tileSize, // Bottom-right
                    x, y + tileSize        // Bottom-left
                };


                renderTarget->FillRectangle(D2D1::RectF(x, y, x + tileSize, y + tileSize), tile.pBrush);
            }
        }

        //valid tiles render.
        for (const ChessTile& tile : validTiles) {
            // Calculate the center of the tile
            int col = tile.col;
            int y = tile.row;
            ChessTile tileOnBoard = board.tile[col][y];
            float centerX = (tileOnBoard.polygon.x0 + tileOnBoard.polygon.x1 + tileOnBoard.polygon.x2 + tileOnBoard.polygon.x3) / 4.0f;
            float centerY = (tileOnBoard.polygon.y0 + tileOnBoard.polygon.y1 + tileOnBoard.polygon.y2 + tileOnBoard.polygon.y3) / 4.0f;

            // Create the ellipse centered at the tile's center
            ID2D1SolidColorBrush* test = nullptr;
            HRESULT hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(0.250f, 1.0f, 0.25f, 0.5f), &test);
            D2D1_ELLIPSE dec;
            float dotRadius = tileSize / 5; // Adjust the dot radius as needed (smaller value for smaller dots)

            dec.point = D2D1::Point2F(centerX, centerY);
            dec.radiusX = dec.radiusY = dotRadius;

            // Draw the ellipse on the valid tile
            renderTarget->FillEllipse(dec, test);

            // Release the dot brush after use
            test->Release();
            test = nullptr;
        }

        //debug selected tile
        auto& tile = board.tile[selectedtileCol][selectedtileRow];
        if (tile.col == board.tile[selectedtileCol][selectedtileRow].col && tile.row == board.tile[selectedtileCol][selectedtileRow].row && selectedtileRow >= 0 && selectedtileCol >= 0) {

            // Create a solid color brush for the dots
            ID2D1SolidColorBrush* dotBrush = nullptr;
            HRESULT hr = renderTarget->CreateSolidColorBrush(D2D1::ColorF(1.0f, 0.9f, 0), &dotBrush);
            if (FAILED(hr))
            {
                // Handle the error if necessary
            }

            // Draw four dots for each point of the tile polygon
            D2D1_ELLIPSE dotEllipse;
            float dotRadius = tileSize / 10; // Adjust the dot radius as needed

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


        //pieces
        for (int row = 0; row < board.GetRows(); row++)
        {
            for (int col = 0; col < board.GetColumns(); col++)
            {
                if (board.tile[col][row].piece) {
                    board.tile[col][row].padding = padding;
                    DrawBitmap(renderTarget, board.tile[col][row].piece->bitmap, board.tile[col][row].piece->row, board.tile[col][row].piece->col, tileSize, tileSize);
                }
            }
        }

        if (draggablePiece) {
            int dragRow = draggablePiece->row - (tileSize / 2);
            int dragCol = draggablePiece->col - (tileSize / 2);
            D2D1_RECT_F destRect = D2D1::RectF(dragRow, dragCol, dragRow + tileSize, dragCol + tileSize);
            renderTarget->DrawBitmap(draggablePiece->bitmap, destRect);
        }

    // End drawing
        renderTarget->EndDraw();
    }
}