#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <string>
using namespace std;
vector<int> generateArray(long arrSize, const string& distType, int seed);
bool checkSorted(const vector<int>& arr);
double getTime();
#endif
