#ifndef PLAYER_H
#define PLAYER_H

#include <tuple>
#include <array>
#include <functional>
#include <memory>

class Player;

constexpr int PLAYER_NUM = 4;
using PlayerPtrList = std::array<std::shared_ptr<Player>, PLAYER_NUM>;
using ConstPlayerPtrList = std::array<std::shared_ptr<const Player>, PLAYER_NUM>;

#include "piece.hpp"
#include "dumpable.hpp"
#include "player_color.hpp"
#include "field.hpp"
#include "vec2.hpp"

class Field;

//指し手を入力するクラス
class Player : public Dumpable
{
	protected:
		int cursorX;
		int cursorY;
		Piece cursorPiece;

		PlayerColor color;

		std::vector<int> unusedPieceDefIdList;

	public:
		Player();
		Player(PlayerColor color_);
		virtual std::tuple<vec2, Piece> turn(const Field& field, int yourId, const ConstPlayerPtrList& constPlayerPtrList) = 0;

		void setPlayerColor(PlayerColor color_);

		int getCursorX() const;
		int getCursorY() const;
		const Piece& getCursorPiece() const;
		PlayerColor getColor() const;
		void setUnusedPieceDefIdList(const std::vector<int>& unusedPieceDefIdList_);
		const std::vector<int>& getUnusedPieceDefIdList() const;
};

#endif
