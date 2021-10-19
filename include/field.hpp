#ifndef FIELD_HPP
#define FIELD_HPP

#include <array>
#include <memory>
#include "piece.hpp"

struct Cell
{
	bool isFilled = false;
	int playerId;

	explicit operator bool() const { return isFilled; }
};

class Field
{
	public:
		static constexpr int SIZE = 20;

		const Cell& get(int x, int y) const;

		bool canSet(int x, int y, const Piece& p, int playerId) const;

		bool set(int x, int y, const Piece& p, int playerId);
		void remove(int x, int y, const Piece& p);
		void clear();
	private:
		std::array<std::array<Cell, SIZE>, SIZE> table;
};

#endif
