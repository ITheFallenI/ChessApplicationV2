#pragma once
#include "HeaderCollection.h"


class ChessBoard;

enum class TileTeam {
    NONE,
    WHITE,
    BLACK
};

struct TilePolygon {
    float x0, y0;
    float x1, y1;
    float x2, y2;
    float x3, y3;
};


std::ostream& operator<<(std::ostream& os, const TileTeam& TileTeam);

enum class TileType {
    NONE,
    PAWN,
    BISHOP,
    KNIGHT,
    ROOK,
    QUEEN,
    KING
};

std::ostream& operator<<(std::ostream& os, const TileType& TileType);

struct ChessPiece {
    TileTeam team;
    TileType type;
    ID2D1Bitmap* bitmap = nullptr;

    int row = 0;
    int col = 0;
    int tileSize = 60;


    ChessPiece() = default;
};

struct ChessTile
{
    TilePolygon polygon;
    D2D1_COLOR_F defaultColor;
    D2D1_COLOR_F currentColor;
    ID2D1SolidColorBrush* pBrush = nullptr;

    int col = -1;
    int row = -1;
    int padding = 0;
    ChessPiece* piece = nullptr;
    ChessTile() = default;

    bool operator==(const ChessTile& other) const
    {
        // Define the comparison logic based on your criteria for two tiles being equal.
        // For example, if two tiles are considered equal if their x and y coordinates match:
        return col == other.col && row == other.row;
    }
};



class ChessBoard
{
public:
    ChessBoard();
    ~ChessBoard();

    ChessTile tile[8][8]; // 2D array of ChessTile objects

    int GetRows() const { return 8; }
    int GetColumns() const { return 8; }
    bool IsPointInsidePolygon(float x, float y, float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3);

    static HRESULT CreateBitmapFromTexture(ID2D1RenderTarget* pRenderTarget, const wchar_t* filePath, ID2D1Bitmap** ppBitmap);

    static ID2D1Bitmap* pPawnBitmap_w;
    static ID2D1Bitmap* pPawnBitmap_b;

    static ID2D1Bitmap* pKnightBitmap_w;
    static ID2D1Bitmap* pKnightBitmap_b;
};


static std::vector<ChessTile> validTiles;

static std::vector<ChessTile> getValidPawnMove(const ChessTile& currentTile, ChessBoard& board);
static std::vector<ChessTile> getValidKnightMove(const ChessTile& currentTile, ChessBoard& board);
static std::vector<ChessTile> getValidRookMove(const ChessTile& currentTile, ChessBoard& b1);

std::vector<ChessTile> getValidPawnMove(const ChessTile& currentTile, ChessBoard& b1)
{
    validTiles.clear();

    if (currentTile.piece->team == TileTeam::WHITE) {
        OutputDebugString(L"Calculating valid moves for white pawn....\n");

        std::wstring str = L"This tile is at: X: " + std::to_wstring(currentTile.col) +
            L" Y: " + std::to_wstring(currentTile.row) + L"\n";
        OutputDebugString(str.c_str());

        // Check if the destination tile is within the board bounds and is empty (NONE)
        if (currentTile.row + 1 < 8 && !b1.tile[currentTile.col][currentTile.row + 1].piece) {
            // Add the valid move to the list of valid tiles
            validTiles.push_back(b1.tile[currentTile.col][currentTile.row + 1]);
        }

        if (currentTile.row == 1 && currentTile.row < 8) 
        {
            if (currentTile.row + 2 < 8 && !b1.tile[currentTile.col][currentTile.row + 2].piece) {
                // Add the valid move to the list of valid tiles
                validTiles.push_back(b1.tile[currentTile.col][currentTile.row + 2]);
            }
        }

        if (currentTile.row + 1 < 8 && currentTile.col + 1 < 8 && b1.tile[currentTile.col + 1][currentTile.row + 1].piece) {
            if(b1.tile[currentTile.col + 1][currentTile.row + 1].piece->team == TileTeam::BLACK)
                validTiles.push_back(b1.tile[currentTile.col + 1][currentTile.row + 1]);
        }
    }


    for (size_t i = 0; i < validTiles.size(); i++)
    {
        const ChessTile tile = validTiles[i];
        std::wstring str = L"Valid move: X: " + std::to_wstring(tile.col) +
            L" Y: " + std::to_wstring(tile.row) + L"\n";
        OutputDebugString(str.c_str());
    }

    return validTiles;
}

std::vector<ChessTile> getValidKnightMove(const ChessTile& currentTile, ChessBoard& b1) 
{
    validTiles.clear();

    int directionRow[] = { 2, 1, -1, -2, -2, -1, 1, 2 };
    int directionCol[] = { 1, 2, 2, 1, -1, -2, -2, -1 };

    if (currentTile.piece->team == TileTeam::WHITE) 
    {
        OutputDebugString(L"Calculating valid moves for white knight....\n");
        std::wstring str = L"This tile is at: X: " + std::to_wstring(currentTile.col) +
            L" Y: " + std::to_wstring(currentTile.row) + L"\n";

        OutputDebugString(str.c_str());

        for (size_t i = 0; i < 8; i++)
        {
            int destRow = currentTile.row + directionRow[i];
            int destCol = currentTile.col + directionCol[i];

            if (destRow >= 0 && destRow <= 7 && destCol >= 0 && destCol <= 7 && !b1.tile[destCol][destRow].piece) {
                validTiles.push_back(b1.tile[destCol][destRow]);
            }
        }
    }
    return validTiles;
}


std::vector<ChessTile> getValidRookMove(const ChessTile& currentTile, ChessBoard& b1) 
{
    validTiles.clear();

    int directionRow[] = { -1, 1, 0, 0 };
    int directionCol[] = { 0, 0, -1, 1 };

    if (currentTile.piece->team == TileTeam::WHITE)
    {
        OutputDebugString(L"Calculating valid moves for white rook....\n");
        std::wstring str = L"This tile is at: X: " + std::to_wstring(currentTile.col) +
            L" Y: " + std::to_wstring(currentTile.row) + L"\n";


        for (size_t i = 0; i < 4; i++)
        {
            int destRow = currentTile.row + directionRow[i];
            int destCol = currentTile.col + directionCol[i];

            if (destRow >= 0 && destRow <= 7 && destCol >= 0 && destCol <= 7 && !b1.tile[destCol][destRow].piece) {
                validTiles.push_back(b1.tile[destCol][destRow]);
            }
        }
    }

    return validTiles;
}