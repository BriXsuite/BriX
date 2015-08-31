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


void PrintIso(IsoInfo isotope) {
    const int times = 7;
    std::cout << std::endl << "Isotope lib: " << isotope.name << std::endl;

    if (isotope.fluence.size() < 1) {
        std::cout << "  !Region not built." << std::endl;
        return;
    }

    std::cout << "Fluence: ";
    for (int i = 0; i < times; i++) {
        std::cout << isotope.fluence[i] << " ";
    } std::cout << std::endl;

    std::cout << "NProd: ";
    for (int i = 0; i < times; i++) {
        std::cout << isotope.neutron_prod[i] << " ";
    } std::cout << std::endl;

    std::cout << "NDest: ";
    for (int i = 0; i < times; i++) {
        std::cout << isotope.neutron_dest[i] << " ";
    } std::cout << std::endl;

    std::cout << "BU: ";
    for (int i = 0; i < times; i++) {
        std::cout << isotope.BU[i] << " ";
    } std::cout << std::endl;

    std::cout << "----------------------------" << std::endl;
}
