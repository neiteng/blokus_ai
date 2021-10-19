#ifndef PIECE_H
#define PIECE_H

#include <array>
#include <functional>
#include <vector>
#include "vec2.hpp"

constexpr int PIECE_NUM = 21;
extern const std::array<std::vector<vec2>, PIECE_NUM> PIECE_DEF;

//ピースの状態を定義する
class Piece
{
	public:
		explicit Piece(int defId);

		void rotate(int n = 1);
		void reflectX(int n = 1);
		void reflectY(int n = 1);

		int getDefId() const;

		const std::vector<vec2>& getCellList() const;

	private:
		std::vector<vec2> cellList;
		int defId;

		void translate(std::function<vec2(const vec2&)> f);//todo
};

#endif
