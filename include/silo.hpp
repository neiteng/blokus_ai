#ifndef SILO_HPP
#define SILO_HPP

template<typename T, std::size_t N>
class silo : public std::array<T, N>
{
	private:
		std::size_t n;
	public:
		void push_back(const T& x)
		{
			std::array<T, N>::operator[](n) = x;
			n++;
		}

		void push_back()
		{
			n++;
		}

		void pop_back()
		{
			n--;
		}

		void resize(std::size_t x)
		{
			n = x;
		}

		std::size_t elem_num() const
		{
			return n;
		}

		bool empty() const
		{
			return (n == 0);
		}

		bool full() const
		{
			return (n == N);
		}

		silo() : n(0) {}
};

#endif
