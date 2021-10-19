#ifndef PIECE_DETAIL_HPP
#define PIECE_DETAIL_HPP

#include <vector>
#include <array>
#include <bitset>
#include "vec2.hpp"
#include "piece.hpp"

constexpr int MAX_VAR_NUM = 8;

struct PieceDetail
{
	static const std::array<vec2, 4> NEIGHBOR_TABLE;
	struct EdgeDetail
	{
		vec2 pos;
		std::bitset<4> hasNeighbor;//このセルの４近傍それぞれブロックが存在するか
	};
	struct NecDetail
	{
		vec2 pos;
		std::bitset<4> allowNeighbor;//このセルの４近傍それぞれブロックが存在するのを許容するか
	};
	std::vector<vec2> cellList;
	std::vector<EdgeDetail> edgeList;
	std::vector<vec2> neighborList;//近傍セルと自分自身のセルも含む
	std::vector<NecDetail> necessaryCellList;
	//ピース定義のピースをrotate回時計回りに回転させた後、reflectX回x軸対象に反射させることでこの形を作ることができる
	int defId;
	int rotate;
	int reflectX;
};

//ピース定義から多くの情報を抜き出す
class PieceAnalyst
{
	private:
		std::array<std::vector<PieceDetail>, PIECE_NUM> pTable;

	public:
		PieceAnalyst(const std::array<std::vector<vec2>, PIECE_NUM>& def);
		int variationNum(int defId) const;//defIdのピース定義を回転したりひっくり返したりしてできる図形が何種類あるか返す
		const PieceDetail& get(int defId, int varId) const;
};

#endif
