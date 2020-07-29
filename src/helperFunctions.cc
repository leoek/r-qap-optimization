#include "helperFunctions.h"

int GetRndNumberFromRange(int start, int end){
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 eng(rd()); // seed the generator
    std::uniform_int_distribution<> distr(start, end); // define the range
    return distr(eng); // return the rnd number
}

bool vectorContains(std::vector<int> vector, int val){
    return std::find(vector.begin(), vector.end(), val) != vector.end();
} 

int getFirstNotInList(std::vector<int> input, std::vector<int> filter){
    int i = 0;
    while(i < input.size()){
        if (!vectorContains(filter, input[i])){
            return input[i];
        }
        i++;
    }
    return -1;
}

bool containsSolution(std::vector<Solution*> &population, Solution &sol){
    for (unsigned int i = 0; i < population.size(); i++){
        if (sol.permutation == population[i]->permutation){
            return true;
        }
    }
    return false;
}
bool containsSolution(std::deque<Solution*> &population, Solution &sol){
    for (unsigned int i = 0; i < population.size(); i++){
        if (sol.permutation == population[i]->permutation){
            return true;
        }
    }
    return false;
}
