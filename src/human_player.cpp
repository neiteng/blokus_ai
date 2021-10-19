#include <ncurses.h>
#include <limits>
#include "human_player.hpp"
#include "blokus_dumper.hpp"

HumanPlayer::HumanPlayer(PlayerColor color_) :
	Player(color_) {}

std::tuple<vec2, Piece> HumanPlayer::turn(const Field& field, int yourId, const ConstPlayerPtrList& constPlayerPtrList)
{
	int selectedPieceIndex = 0;
	cursorPiece = Piece(unusedPieceDefIdList[selectedPieceIndex]);

	cursorX = Field::SIZE / 2;
	cursorY = Field::SIZE / 2;

	if(auto dumper = getDumper())
	{
		dumper->dump();
	}

	int command;
	while((command = getch()) != '\n')
	{
		switch(command)
		{
			case 'l':
			case KEY_RIGHT:
				cursorX++;
				break;
			case 'h':
			case KEY_LEFT:
				cursorX--;
				break;
			case 'j':
			case KEY_DOWN:
				cursorY++;
				break;
			case 'k':
			case KEY_UP:
				cursorY--;
				break;
			case 'd':
				cursorPiece.rotate();
				break;
			case 'f':
				selectedPieceIndex = (selectedPieceIndex + 1) % unusedPieceDefIdList.size();
				cursorPiece = Piece(unusedPieceDefIdList[selectedPieceIndex]);
				break;
			case 'g':
				selectedPieceIndex = (selectedPieceIndex + unusedPieceDefIdList.size() - 1) % unusedPieceDefIdList.size();
				cursorPiece = Piece(unusedPieceDefIdList[selectedPieceIndex]);
				break;
			case 's':
				cursorPiece.reflectX();
				break;
			case 'a':
				cursorPiece.reflectY();
				break;
		}

		int minX = std::numeric_limits<int>::max();
		int minY = std::numeric_limits<int>::max();
		int maxX = std::numeric_limits<int>::min();
		int maxY = std::numeric_limits<int>::min();
		for(auto [ dx, dy ] : cursorPiece.getCellList())
		{
			minX = std::min(minX, cursorX + dx);
			minY = std::min(minY, cursorY + dy);
			maxX = std::max(maxX, cursorX + dx);
			maxY = std::max(maxY, cursorY + dy);
		}

		cursorX += std::max(0, 0 - minX);
		cursorX -= std::max(0, maxX - (Field::SIZE - 1));
		cursorY += std::max(0, 0 - minY);
		cursorY -= std::max(0, maxY - (Field::SIZE - 1));

		if(auto dumper = getDumper())
		{
			dumper->dump();
		}
	}

	return { vec2{ cursorX, cursorY } , cursorPiece };
}
