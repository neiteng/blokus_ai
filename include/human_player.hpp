#ifndef HUMAN_PLAYER_HPP
#define HUMAN_PLAYER_HPP

#include <tuple>
#include <memory>
#include "player_color.hpp"
#include "player.hpp"
#include "piece.hpp"
#include "field.hpp"
#include "vec2.hpp"

class HumanPlayer : public Player
{
	public:
		HumanPlayer() = default;
		HumanPlayer(PlayerColor color_);
		std::tuple<vec2, Piece> turn(const Field& field, int yourId, const ConstPlayerPtrList& constPlayerPtrList) override;
};

#endif
