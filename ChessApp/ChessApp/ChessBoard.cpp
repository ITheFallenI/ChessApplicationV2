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
	for (int row = 0; row < GetRows(); row++)
	{
		for (int col = 0; col < GetColumns(); col++)
		{
			D2D1_COLOR_F genColor = (col + row) % 2 == 0 ? D2D1::ColorF(1, 0.8, 0.61, 1) :
				D2D1::ColorF(0.81, 0.54, 0.275, 1);


			// Set values for each ChessTile in the board
			board[row][col].x = col;
			board[row][col].y = row;
			board[row][col].pBrush = nullptr;
			board[row][col].type = TileType::NONE;
			board[row][col].team = TileTeam::NONE;
			board[row][col].defaultColor = genColor;
			board[row][col].currentColor = genColor;

		}
	}
}

ChessBoard::~ChessBoard()
{

}