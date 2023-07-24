#include "ChessBoard.h"


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
	for (size_t y = 0; y < rows; y++)
	{
		for (size_t x = 0; x < cols; x++)
		{
			D2D1_COLOR_F genColor = (x + y) % 2 == 0 ? D2D1::ColorF(1, 0.8, 0.61, 1) : 
				D2D1::ColorF(0.81, 0.54, 0.275, 1);

			ChessTile tile;
			tile.x = x;
			tile.y = y;
			tile.pBrush = nullptr;
			tile.type = TileType::NONE;
			tile.team = TileTeam::NONE;
			tile.defaultColor = genColor;
			tile.currentColor = genColor;
			tiles.push_back(tile);
		}
	}

	//log created tiles?
	bool logTiles = true;

	if (logTiles) {
		for (size_t i = 0; i < tiles.size(); i++)
		{
			//std::cout << "Tile " << i << ": Type: " << tiles[i].type << " Team Color: " << tiles[i].team << std::endl;
		}

		std::wstring mesg = L"Generated: (" + std::to_wstring(tiles.size()) + L") tiles.\n";
		OutputDebugStringW(mesg.c_str());
	}
}

ChessBoard::~ChessBoard()
{

}

int ChessBoard::GetRows()
{
	return rows;
}

int ChessBoard::GetColumns()
{
	return cols;
}
