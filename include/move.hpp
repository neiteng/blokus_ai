#ifndef MOVE_HPP
#define MOVE_HPP

struct Move
{
	Move() {}
	Move(int x_, int y_, int defId_, int varId_, bool isValid_ = true)
		: x(x_), y(y_), defId(defId_), varId(varId_), isValid(isValid_) {}
	int x;
	int y;
	int defId;
	int varId;
	bool isValid;
};

#endif
