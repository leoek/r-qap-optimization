#include <iostream>
#include <random>
#include <algorithm>

int GetRndNumberFromRange(int start, int end){
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(start, end); // define the range
    return distr(eng); // return the rnd number
}

bool vectorContains(std::vector<int> vector, int val){
    return std::find(vector.begin(), vector.end(), val) != vector.end();
} 
