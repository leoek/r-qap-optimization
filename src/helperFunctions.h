#ifndef AGENT_ADDON_HELPER_FUNCTIONS_H_
#define AGENT_ADDON_HELPER_FUNCTIONS_H_

int GetRndNumberFromRange(int, int);
bool vectorContains(std::vector<int> vector, int val);
/**
 * @param vector to return from
 * @param vector to filter by
 * @returns first value from input (first vector) which is not in the filter (second vector),
 *  return -1 if all values from input are in filter
 */
int getFirstNotInList(std::vector<int> input, std::vector<int> filter);

#endif  // AGENT_ADDON_HELPER_FUNCTIONS_H_