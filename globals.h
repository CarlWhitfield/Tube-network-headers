#ifndef GLOBALS_H
#define GLOBALS_H

#include <iostream>
#include <vector>

//general functions

namespace globals{

	inline void abort_on_failure()   //generic abort function
	{
		exit(EXIT_FAILURE);
	}

	template <typename I, typename T>
	inline I int_round(const T & num)   //round number to size_t
	{
		I nfloor = I(num);
		if (num - ((T)nfloor) >= 0.5)
		{
			return nfloor + 1;
		}
		else
		{
			return nfloor;
		}
	}

	template <typename T>
	std::vector<T> vectorise(const T * value_array, const int & N)    //turn c array into vector
	{
		std::vector<T> v;
		for (int n = 0; n < N; n++)
		{
			v.push_back(value_array[n]);
		}
		return v;
	}

	class ProgressBar{
	private:
		int _barWidth;
	public:
		ProgressBar(){
			_barWidth = 50;
		}

		ProgressBar(const int & bw){
			_barWidth = bw;
		}

		inline void initialise() const{
			this->update(0.0);
		}

		inline void update(const double & progress) const{
			std::cout << "[";
			int pos = int(_barWidth * progress);
			for (int i = 0; i < _barWidth; ++i) {
				if (i < pos) std::cout << "=";
				else if (i == pos) std::cout << ">";
				else std::cout << " ";
			}
			std::cout << "] " << int(progress * 100.0) << " %\r";
			std::cout.flush();
		}

		inline void terminate() const {
			std::cout << std::endl;
		}
	};
}

#endif