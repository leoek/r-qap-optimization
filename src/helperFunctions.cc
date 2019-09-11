#include <iostream>
#include <random>
int GetRndNumberFromRange(int start, int end){
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(start, end); // define the range
    return distr(eng); // return the rnd number
}