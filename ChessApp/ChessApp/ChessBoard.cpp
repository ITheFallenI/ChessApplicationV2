#include "ChessBoard.h"


ID2D1Bitmap* ChessBoard::pPawnBitmap_w = nullptr;
ID2D1Bitmap* ChessBoard::pPawnBitmap_b = nullptr;

ID2D1Bitmap* ChessBoard::pKnightBitmap_w = nullptr;
ID2D1Bitmap* ChessBoard::pKnightBitmap_b = nullptr;

ID2D1Bitmap* ChessBoard::pRookBitmap_w = nullptr;
ID2D1Bitmap* ChessBoard::pRookBitmap_b = nullptr;

ID2D1Bitmap* ChessBoard::pBishopBitmap_w = nullptr;
ID2D1Bitmap* ChessBoard::pBishopBitmap_b = nullptr;

// Overload the << operator for PieceType enum
std::ostream& operator<<(std::ostream& os, const TileType& pieceType)
{
	switch (pieceType)
	{
	case TileType::NONE:
		os << "Empty";
		break;
	case TileType::PAWN:
		os << "Pawn";
		break;
    case TileType::KNIGHT:
        os << "Knight";
        break;
		// Add more cases for other PieceType values if needed
	default:
		os << "Unknown";
		break;
	}
	return os;
}

// Overload the << operator for PieceType enum
std::ostream& operator<<(std::ostream& os, const TileTeam& PieceTeam)
{
	switch (PieceTeam)
	{
	case TileTeam::NONE:
		os << "None";
		break;
	case TileTeam::WHITE:
		os << "White";
		break;
	case TileTeam::BLACK:
		os << "Black";
		break;
		// Add more cases for other PieceType values if needed
	default:
		os << "Unknown";
		break;
	}
	return os;
}

ChessBoard::ChessBoard()
{

	OutputDebugStringW(L"Init ChessBoard, Generating tiles.\n");

	//Generate 64 Chess tiles.
	for (int row = GetRows() - 1; row >= 0; row--)
	{
		for (int col = 0; col < GetColumns(); col++)
		{
            //1, 0.8, 0.61, 1
			D2D1_COLOR_F genColor = (col + row) % 2 == 0 ? D2D1::ColorF(0.81, 0.54, 0.275, 1) :
				D2D1::ColorF(1, 0.8, 0.61, 1);

			// Set values for each ChessTile in the board
            tile[col][row].col = col;
            tile[col][row].row = row;
            tile[col][row].pBrush = nullptr;
            tile[col][row].defaultColor = genColor;
            tile[col][row].currentColor = genColor;

		}
	}
}

ChessBoard::~ChessBoard()
{

	OutputDebugStringW(L"Chess Board destructor.\n");
}

bool ChessBoard::IsPointInsidePolygon(float x, float y, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
	// Perform point-in-polygon test using cross product
	float crossProduct1 = (x1 - x0) * (y - y0) - (x - x0) * (y1 - y0);
	float crossProduct2 = (x2 - x1) * (y - y1) - (x - x1) * (y2 - y1);
	float crossProduct3 = (x3 - x2) * (y - y2) - (x - x2) * (y3 - y2);
	float crossProduct4 = (x0 - x3) * (y - y3) - (x - x3) * (y0 - y3);

	return (crossProduct1 >= 0 && crossProduct2 >= 0 && crossProduct3 >= 0 && crossProduct4 >= 0) ||
		(crossProduct1 <= 0 && crossProduct2 <= 0 && crossProduct3 <= 0 && crossProduct4 <= 0);
}


HRESULT ChessBoard::CreateBitmapFromTexture(ID2D1RenderTarget* pRenderTarget, const wchar_t* filePath, ID2D1Bitmap** ppBitmap)
{
    ID2D1Bitmap* pBitmap = nullptr;
    IWICImagingFactory* pWICFactory = nullptr;
    IWICBitmapDecoder* pDecoder = nullptr;
    IWICBitmapFrameDecode* pFrame = nullptr;
    IWICFormatConverter* pConverter = nullptr;

    HRESULT hr = CoCreateInstance(
        CLSID_WICImagingFactory,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pWICFactory)
    );

    if (FAILED(hr))
    {
        // Handle the error if WIC factory creation fails.
        OutputDebugString(L"Failed to create WIC imaging factory.\n");
        return hr;
    }

    hr = pWICFactory->CreateDecoderFromFilename(
        filePath,
        nullptr,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &pDecoder
    );

    if (FAILED(hr))
    {
        // Handle the error if image decoding fails.
        OutputDebugString(L"Failed to decode image file.\n");
        pWICFactory->Release();
        return hr;
    }

    hr = pDecoder->GetFrame(0, &pFrame);

    if (FAILED(hr) || !pFrame)
    {
        // Handle the error if frame retrieval fails or pFrame is null.
        OutputDebugString(L"Failed to get image frame or pFrame is null.\n");
        pDecoder->Release();
        pWICFactory->Release();
        return hr;
    }

    hr = pWICFactory->CreateFormatConverter(&pConverter);

    if (FAILED(hr))
    {
        // Handle the error if format conversion fails.
        OutputDebugString(L"Failed to create WIC format converter.\n");
        pFrame->Release();
        pDecoder->Release();
        pWICFactory->Release();
        return hr;
    }

    hr = pConverter->Initialize(
        pFrame,
        GUID_WICPixelFormat32bppPBGRA,
        WICBitmapDitherTypeNone,
        nullptr,
        0.0f,
        WICBitmapPaletteTypeMedianCut
    );

    if (FAILED(hr))
    {
        // Handle the error if format conversion initialization fails.
        OutputDebugString(L"Failed to initialize WIC format converter.\n");
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pWICFactory->Release();
        return hr;
    }

    hr = pRenderTarget->CreateBitmapFromWicBitmap(pConverter, nullptr, &pBitmap);

    if (FAILED(hr))
    {
        // Handle the error if creating Direct2D bitmap from WIC bitmap fails.
        OutputDebugString(L"Failed to create Direct2D bitmap from WIC bitmap.\n");
        pConverter->Release();
        pFrame->Release();
        pDecoder->Release();
        pWICFactory->Release();
        return hr;
    }

    // Release resources used for loading the image
    pConverter->Release();
    pFrame->Release();
    pDecoder->Release();
    pWICFactory->Release();

    *ppBitmap = pBitmap;

    return hr;
}
