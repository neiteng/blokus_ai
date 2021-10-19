#ifndef COMPUTER_PLAYER_HPP
#define COMPUTER_PLAYER_HPP

#include <tuple>
#include <memory>
#include <random>
#include "player.hpp"
#include "piece.hpp"
#include "player_color.hpp"
#include "field.hpp"
#include "vec2.hpp"
#include "piece_detail.hpp"
#include "move_cand.hpp"

class ComputerPlayerV1 : public Player
{
	private:
		void randomWalk();
		void moveCursorTo(const MoveCand& move, int interval);
		void remedialSqueezeout();

		std::mt19937 mt_rand;
		std::uniform_real_distribution<> real_dist;

		std::shared_ptr<PieceAnalyst> pieceAnalyst;

		int selectedIndex;
	public:
		ComputerPlayerV1() = default;
		ComputerPlayerV1(PlayerColor color_);
		std::tuple<vec2, Piece> turn(const Field& field, int yourId, const ConstPlayerPtrList& constPlayerPtrList) override;
};

#endif
