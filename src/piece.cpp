#include "piece.hpp"

const std::array<std::vector<vec2>, PIECE_NUM> PIECE_DEF =
{ {
	std::vector<vec2> { { 0, 0 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { 0, 1 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { -1, 0 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { 0, 1 }, { 1, 1 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 0, 1 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 2, 0 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 1, 1 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { 0, 1 }, { -1, 1 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { -1, 0 }, { -2, 0 }, { 1, 1 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 0, 1 }, { 0, 2 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { -1, 0 }, { -1, 1 }, { -1, 2 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { 2, 0 }, { 0, 1 }, { -1, 1 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { -1, 0 }, { 1, -1 }, { -1, 1 } },
	std::vector<vec2> { { 0, 0 }, { 1, 0 }, { -1, 0 }, { -2, 0 }, { 2, 0 } },
	std::vector<vec2> { { 0, 0 }, { -1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 } },
	std::vector<vec2> { { 0, 0 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { -1, 1 } },
	std::vector<vec2> { { 0, 0 }, { 0, -1 }, { 1, -1 }, { 0, 1 }, { 1, 1 } },
	std::vector<vec2> { { 0, 0 }, { 0, -1 }, { 1, -1 }, { -1, 0 }, { 0, 1 } },
	std::vector<vec2> { { 0, 0 }, { 0, -1 }, { 1, 0 }, { -1, 0 }, { 0, 1 } },
	std::vector<vec2> { { 0, 0 }, { 0, -1 }, { 1, 0 }, { -1, 0 }, { 2, 0 } }
} };

Piece::Piece(int defId_) :
	cellList(PIECE_DEF[defId_]),
	defId(defId_) {}

const std::vector<vec2>& Piece::getCellList() const
{
	return cellList;
}

int Piece::getDefId() const
{
	return defId;
}

void Piece::translate(std::function<vec2(const vec2&)> f)
{
	for(auto& v : cellList) v = f(v);
}

void Piece::rotate(int n)
{
	for(int i = 0; i < n; i++)
	{
		translate([](const vec2& v)
				{
				return vec2(-v.y, v.x);
				} );
	}
}

void Piece::reflectX(int n)
{
	for(int i = 0; i < n; i++)
	{
		translate([](const vec2& v)
				{
				return vec2(v.x, -v.y);
				} );
	}
}

void Piece::reflectY(int n)
{
	for(int i = 0; i < n; i++)
	{
		translate([](const vec2& v)
				{
				return vec2(-v.x, v.y);
				} );
	}
}

