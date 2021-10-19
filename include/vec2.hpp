#ifndef VEC2_HPP
#define VEC2_HPP

struct vec2
{
	int x;
	int y;
	vec2(){}
	vec2(int x_, int y_)
		: x(x_), y(y_) {}
	bool operator==(const vec2& value) const
	{
		return x == value.x && y == value.y;
	}
};

#endif
