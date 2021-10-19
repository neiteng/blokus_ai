#include "field.hpp"

const Cell& Field::get(int x, int y) const { return table[y][x]; }

bool Field::canSet(int x, int y, const Piece& p, int playerId) const
{
	static const std::array<std::pair<int, int>, 4> neighbor = { { std::make_pair(1, 0), std::make_pair(0, 1), std::make_pair(-1, 0), std::make_pair(0, -1)} };
	static const std::array<std::pair<int, int>, 4> slanting = { { std::make_pair(1, 1), std::make_pair(1, -1), std::make_pair(-1, 1), std::make_pair(-1, -1)} };

	bool doesSlantingExist = false;

	for(auto [ dx, dy ] : p.getCellList())
	{
		int rx = x + dx;
		int ry = y + dy;
		if(rx < 0 || rx >= SIZE || ry < 0 || ry >= SIZE) return false;
		if(table[ry][rx]) return false;
		for(const auto& v : neighbor)
		{
			int nx = rx + v.first;
			int ny = ry + v.second;
			if(nx < 0 || nx >= SIZE || ny < 0 || ny >= SIZE) continue;
			if(table[ny][nx] && table[ny][nx].playerId == playerId) return false;
		}
		for(const auto& v : slanting)
		{
			int nx = rx + v.first;
			int ny = ry + v.second;
			if((nx == -1 && ny == -1) || (nx == -1 && ny == SIZE) || (nx == SIZE && ny == -1) || (nx == SIZE && ny == SIZE)) doesSlantingExist = true;
			if(nx < 0 || nx >= SIZE || ny < 0 || ny >= SIZE) continue;
			if(table[ny][nx] && table[ny][nx].playerId == playerId) doesSlantingExist = true;
		}
	}
	return doesSlantingExist;
}

bool Field::set(int x, int y, const Piece& p, int playerId)
{
	if(!canSet(x, y, p, playerId)) return false;

	for(auto [ dx, dy ] : p.getCellList())
	{
		int rx = x + dx;
		int ry = y + dy;
		table[ry][rx] = { true, playerId };
	}

	return true;
}

void Field::remove(int x, int y, const Piece& p)
{
	for(auto [ dx, dy ] : p.getCellList())
	{
		int rx = x + dx;
		int ry = y + dy;
		if(rx < 0 || rx >= SIZE || ry < 0 || ry >= SIZE) continue;
		table[ry][rx].isFilled = false;
	}
}

void Field::clear()
{
	for(int x = 0; x < SIZE; x++)
	{
		for(int y = 0; y < SIZE; y++)
		{
			table[x][y].isFilled = false;
		}
	}
}
