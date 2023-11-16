#pragma once
#include <random>
#include <ctime>

class Random{
	static std::mt19937 random;
public:
	static int Number(int min, int max);
};

