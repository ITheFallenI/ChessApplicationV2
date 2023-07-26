#pragma once
#include "HeaderCollection.h"

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

    int x = 0;
    int y = 0;
    int tileSize = 60;


    ChessPiece() = default;
};

struct ChessTile
{
    TilePolygon polygon;
    D2D1_COLOR_F defaultColor;
    D2D1_COLOR_F currentColor;
    ID2D1SolidColorBrush* pBrush = nullptr;

    int x = -1;
    int y = -1;
    int padding = 0;
    ChessPiece* piece = nullptr;
    ChessTile() = default;
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

    static HRESULT LoadChessPieceTexture(ID2D1RenderTarget* pRenderTarget, const wchar_t* filePath, ID2D1Bitmap** ppBitmap);

    static ID2D1Bitmap* pPawnBitmap_w;
    static ID2D1Bitmap* pPawnBitmap_b;
};