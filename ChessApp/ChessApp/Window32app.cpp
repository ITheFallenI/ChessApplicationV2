#include "Window32app.h"

ID2D1Factory* Window32app::d2dFactory = nullptr;
ID2D1HwndRenderTarget* Window32app::renderTarget = nullptr;


D2D1_POINT_2F Window32app::defaultWinSize = { 1280, 720 };
D2D1_POINT_2F Window32app::minWinSize = { 1280, 720 };

ChessBoard chessBoard;

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
        //win32app.CleanUp();
        PostQuitMessage(0);
        return 0;
    }
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
}

void Window32app::RenderBoard()
{

    if (!renderTarget)
    {
        return; // Return if the render target is not valid
    }



    if (renderTarget)
    {
        D2D1_SIZE_F renderTargetSize = renderTarget->GetSize();
        int rows = chessBoard.GetRows();
        int cols = chessBoard.GetColumns();
        int minDimension = static_cast<int>(min(renderTargetSize.width, renderTargetSize.height));

        int padding = 100; // Padding on all sides (adjust this value as desired)
        int chessboardSize = minDimension - (2 * padding);

        // Calculate the size of each tile based on the available space and padding
        float tileSize = static_cast<float>(chessboardSize) / static_cast<float>(max(rows, cols));

        // Calculate the total size of the chessboard including padding
        float chessboardWidth = tileSize * cols + (2 * padding);
        float chessboardHeight = tileSize * rows + (2 * padding);

        // Calculate the position to center the chessboard within the render target
        float chessboardX = (renderTargetSize.width - chessboardWidth) / 2;
        float chessboardY = (renderTargetSize.height - chessboardHeight) / 2;

        // Clear the render target
        renderTarget->BeginDraw();
        renderTarget->Clear(D2D1::ColorF(D2D1::ColorF(0.15, 0.15, 0.15, 1)));

        // Loop through each tile and draw it on the render target with padding
        for (auto& tile : chessBoard.tiles)
        {
            // Calculate the position to draw the tile with padding
            float x = chessboardX + padding + tile.x * tileSize;
            float y = chessboardY + padding + tile.y * tileSize;

            tile.polygon = TilePolygon{
                x, y,                  // Top-left
                x + tileSize, y,       // Top-right
                x + tileSize, y + tileSize, // Bottom-right
                x, y + tileSize        // Bottom-left
            };

            renderTarget->FillRectangle(D2D1::RectF(x, y, x + tileSize, y + tileSize), tile.pBrush);


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

        // End drawing
        renderTarget->EndDraw();
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
    // Create brushes for each tile based on the color stored in the ChessBoardTile struct
    for (size_t i = 0; i < chessBoard.tiles.size(); i++) {
        HRESULT hr = renderTarget->CreateSolidColorBrush(chessBoard.tiles[i].currentColor, &chessBoard.tiles[i].pBrush);
        if (FAILED(hr)) {
            // std::cout << "error creating BRUSH!!" << std::endl;
        }
    }

    return S_OK;
}

void Window32app::LeftClickFunction(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    int mouseX = GET_X_LPARAM(lParam); // Get the X position of the mouse cursor
    int mouseY = GET_Y_LPARAM(lParam); // Get the Y position of the mouse cursor

    const std::wstring rawMouseCoords = L"Mouse X: " + std::to_wstring(mouseX) + L", Mouse y:" + std::to_wstring(mouseY) + L"\n";
    OutputDebugStringW(rawMouseCoords.c_str());

    // Calculate the position of the click relative to the chessboard 
    D2D1_SIZE_F renderTargetSize = renderTarget->GetSize();


    // Convert screen coordinates to chess coordinates
    OutputDebugStringW(L"");


}