#include "piece_detail.hpp"
#include "field.hpp"
#include <queue>
#include <set>
#include <complex>

const std::array<vec2, 4> PieceDetail::NEIGHBOR_TABLE =
{ {
	vec2(1, 0),
	vec2(0, 1),
	vec2(-1, 0),
	vec2(0, -1)
} };

bool isCong(const std::vector<vec2>& a, const std::vector<vec2>& b)
{
	struct cmp
	{
		bool operator()(const vec2& x, const vec2& y)
		{
			return x.x + x.y * 2 * Field::SIZE > y.x + y.y * 2 * Field::SIZE;
		}
	};

	//そもそもセルの数が違うならまず一致しない
	if(a.size() != b.size()) return false;

	//セルを左上から順になるように並び替える
	std::priority_queue<vec2, std::vector<vec2>, cmp> aq;
	for(const auto& cell : a)
	{
		aq.push(cell);
	}
	std::priority_queue<vec2, std::vector<vec2>, cmp> bq;
	for(const auto& cell : b)
	{
		bq.push(cell);
	}

	//セルのズレが全て同じなら一致
	int dx = aq.top().x - bq.top().x;
	int dy = aq.top().y - bq.top().y;
	while(!aq.empty())
	{
		if(aq.top().x - bq.top().x != dx) return false;
		if(aq.top().y - bq.top().y != dy) return false;
		aq.pop(); bq.pop();
	}
	return true;
}

std::vector<PieceDetail::EdgeDetail> getEdgeList(const std::vector<vec2>& cellList)
{
	std::vector<PieceDetail::EdgeDetail> ret;
	for(const auto& cellPos : cellList)
	{
		PieceDetail::EdgeDetail detail = { cellPos };
		for(int i = 0; i < (int)PieceDetail::NEIGHBOR_TABLE.size(); i++)
		{
			int x = cellPos.x + PieceDetail::NEIGHBOR_TABLE[i].x;
			int y = cellPos.y + PieceDetail::NEIGHBOR_TABLE[i].y;

			bool has = false;
			for(const auto& otherPos : cellList)
			{
				if(otherPos.x == x && otherPos.y == y)
				{
					has = true;
					break;
				}
			}
			detail.hasNeighbor[i] = has;
		}
		if(!((detail.hasNeighbor[0] && detail.hasNeighbor[2]) || (detail.hasNeighbor[1] && detail.hasNeighbor[3])))
		{
			ret.push_back(detail);
		}
	}
	return ret;
}

std::vector<vec2> getNeighborList(const std::vector<vec2>& cellList)
{
	struct cmp
	{
		bool operator()(const vec2& a, const vec2& b)
		{
			return a.x + a.y * 10 * PIECE_NUM< b.x + b.y * 10 * PIECE_NUM;
		}
	};
	std::set<vec2, cmp> neighborSet;
	for(const auto& pos : cellList)
	{
		neighborSet.insert(pos);

		int x = pos.x;
		int y = pos.y;
		int dx = 1;
		int dy = 0;
		for(int i = 0; i < 4; i++)
		{
			int nx = x + dx;
			int ny = y + dy;
			neighborSet.insert( { nx, ny } );

			int temp = dx;
			dx = -dy;
			dy = temp;
		}
	}
	std::vector<vec2> ret;
	for(const auto& pos : neighborSet)
	{
		ret.push_back(pos);
	}
	return ret;
}

std::vector<PieceDetail::NecDetail> getNecessaryCellList(const std::vector<vec2>& cellList)
{
	struct cmp
	{
		bool operator()(const vec2& a, const vec2& b)
		{
			return a.x + a.y * 10 * PIECE_NUM< b.x + b.y * 10 * PIECE_NUM;
		}
	};
	std::set<vec2, cmp> necSet;
	for(const auto& cellPos : cellList)
	{
		int dx = 1;
		int dy = 1;
		for(int i = 0; i < 4; i++)
		{
			int nx = cellPos.x + dx;
			int ny = cellPos.y + dy;
			necSet.insert( { nx, ny } );

			int temp = dx;
			dx = -dy;
			dy = temp;
		}
	}

	std::vector<PieceDetail::NecDetail> ret;
	for(const auto& pos : necSet)
	{
		int dx = 1;
		int dy = 0;
		bool isNec = true;
		for(int i = 0; i < 5; i++)
		{
			int x = pos.x + dx;
			int y = pos.y + dy;
			if(i == 4)
			{
				x = pos.x;
				y = pos.y;
			}
			for(const auto& cellPos : cellList)
			{
				if(cellPos.x == x && cellPos.y == y)
				{
					isNec = false;
					i = 4;
					break;
				}
			}
			int temp = dx;
			dx = -dy;
			dy = temp;
		}
		if(isNec)
		{
			PieceDetail::NecDetail detail = { pos , std::bitset<4>(0b1111) };
			int dirNum = PieceDetail::NEIGHBOR_TABLE.size();
			for(int i = 0; i < dirNum; i++)
			{
				int j = (i + 1) % dirNum;
				int nx = pos.x + PieceDetail::NEIGHBOR_TABLE[i].x + PieceDetail::NEIGHBOR_TABLE[j].x;
				int ny = pos.y + PieceDetail::NEIGHBOR_TABLE[i].y + PieceDetail::NEIGHBOR_TABLE[j].y;
				bool has = false;
				for(const auto& otherPos : cellList)
				{
					if(otherPos.x == nx && otherPos.y == ny)
					{
						has = true;
						break;
					}
				}
				if(has)
				{
					detail.allowNeighbor[i] = false;
					detail.allowNeighbor[j] = false;
				}
			}
			ret.push_back(detail);
		}
	}

	return ret;
}

PieceAnalyst::PieceAnalyst(const std::array<std::vector<vec2>, PIECE_NUM>& def)
{
	for(int defId = 0; defId < PIECE_NUM; defId++)
	{
		std::vector<vec2> cellList = def[defId];
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 2; j++)
			{
				//今まで追加したピースと合同であるものがあるなら追加しない
				bool cong = false;
				for(std::size_t k = 0; k < pTable[defId].size(); k++)
				{
					if(isCong(pTable[defId][k].cellList, cellList))
					{
						cong = true;
						break;
					}
				}
				if(!cong)
				{
					pTable[defId].push_back( { cellList, getEdgeList(cellList), getNeighborList(cellList), getNecessaryCellList(cellList), defId, i, j } );
				}

				//refrectX
				for(auto&& pos : cellList)
				{
					pos.y = -pos.y;
				}
			}
			//rotate
			for(auto&& pos : cellList)
			{
				int temp = pos.x;
				pos.x = -pos.y;
				pos.y = temp;
			}
		}
	}
}

int PieceAnalyst::variationNum(int defId) const
{
	return pTable[defId].size();
}

const PieceDetail& PieceAnalyst::get(int defId, int varId) const
{
	return pTable[defId][varId];
}
