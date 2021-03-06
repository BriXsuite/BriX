#ifndef GENERALTOOLS_H
#define GENERALTOOLS_H

#include <string>
#include <sstream>
#include <cmath>
#include <vector>

#include "cyclus.h"

using namespace std;

bool DecreaseChecker(vector<float> &data);
void CumulativeAdd(vector<float> &data);
float Interpolate(float y0, float y1, float x0, float x1, float x);
void UpdateStorage(std::vector<int> &decay_times);
void OutputToTerm(cyclus::Material::Ptr mat);

#endif // GENERALTOOLS_H
