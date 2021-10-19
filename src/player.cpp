#include "player.hpp"

int Player::getCursorX() const { return cursorX; }

int Player::getCursorY() const { return cursorY; }

const Piece& Player::getCursorPiece() const { return cursorPiece; }

PlayerColor Player::getColor() const
{
	return color;
}

Player::Player(PlayerColor color_) :
	cursorX(0),
	cursorY(0),
	cursorPiece(0),
	color(color_)
{
	for(int i = 0; i < PIECE_NUM; i++)
	{
		unusedPieceDefIdList.push_back(i);
	}
}

Player::Player() : Player(PlayerColor::RED) {}

void Player::setPlayerColor(PlayerColor color_) { color = color_; }

void Player::setUnusedPieceDefIdList(const std::vector<int>& unusedPieceDefIdList_) { unusedPieceDefIdList = unusedPieceDefIdList_; }

const std::vector<int>& Player::getUnusedPieceDefIdList() const { return unusedPieceDefIdList; }
