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

struct ChessTile
{
    TilePolygon polygon;
    TileType type;
    TileTeam team;
    ID2D1Bitmap* bitmap = nullptr;
    D2D1_COLOR_F defaultColor;
    D2D1_COLOR_F currentColor;
    ID2D1SolidColorBrush* pBrush = nullptr;

    int x = -1;
    int y = -1;

    ChessTile() = default;
};

class ChessBoard
{
public:
    ChessBoard();
    ~ChessBoard();

    ChessTile board[8][8]; // 2D array of ChessTile objects

    int GetRows() const { return 8; }
    int GetColumns() const { return 8; }
};

