#include "generaltools.h"

// returns true if data is increasing (cumulative sum)
bool DecreaseChecker(vector<float> &data) {

    const int length = data.size();

    if(length < 2) {return false;}

    for(int i = 1; i < length; i++) {
        if(data[i-1] > data[i]) {return true;}
    }

    return false;
}

void CumulativeAdd(vector<float> &data) {
    const int length = data.size();

    if(length < 2) {return;}

    for(int i = 1; i < length; i++) {
        data[i] += data[i-1];
    }
}
