#include "librarytools.h"

// Reads from library in library_path to build all_iso
void LibraryReader(string library_name, string library_path, LibInfo &library) {

    library.name = library_name;

    ifstream inf (library_path + "/manifest.txt"); // Opens manifest file

    if(!inf){
        cout << "Error! LibraryReader failed to read from " << library_path << endl;
    }

    string line;
    string iso_name;

    // Put the name of every available isotope in the library in all_iso
    while (getline(inf, line)) {
        IsoInfo iso; // Temporary iso to push

        istringstream iss(line);

        iss >> iso_name;
        iso.name = atoi(iso_name.c_str()); // Adds name to iso
        iso.fraction = 0; // Default fraction

        IsoBuilder(library_path, iso);

        library.all_iso.push_back(iso);
    }

}

// Reads the isotope information and puts it in iso
void IsoBuilder(string library_path, IsoInfo &iso) {
    //type = cyclus::Env::GetInstallPath() + "/share/brightlite/" + type;
    const double flux_value = FluxFinder(library_path);

    ifstream inf(library_path + "/" + to_string(iso.name) + ".txt");
    if(!inf){
        cout << "Failed to read file for " + library_path + " " +  to_string(iso.name) << endl;
        return;
    }

    int i = 0;
    double value;
    string buffer;
    string line;

    while(getline(inf, line)){
        istringstream iss(line);
        iss >> buffer;
        if (i >= 4){
            Daughter daughter;
            daughter.name = pyne::nucname::zzaaam(buffer);
            while (iss >> value){
                daughter.mass.push_back(value);
            }
            iso.iso_vector.push_back(daughter);
        } else {
            while (iss >> value){
                if (i == 0){
                    iso.fluence.push_back(value*flux_value*84600);
                } else if (i == 1){
                    iso.neutron_prod.push_back(value);
                } else if (i == 2){
                    iso.neutron_dest.push_back(value);
                } else if (i == 3){
                    iso.BU.push_back(value);
                }
            }
            i++;
        }
    }
    inf.close();

    // Assumes derivative if the burnup vector is not increasing
    if(DecreaseChecker(iso.BU)) {
        CumulativeAdd(iso.BU); // Turns it into cumulative vector
    }
}

// Returns the library base flux value
double FluxFinder(string library_path) {

    // Open the params file where flux info is stored
    ifstream inf(library_path + "/params.txt");

    if(!inf){
        cout << "Error attempting to open " << library_path << "/params.txt" << endl;
        return 0;
    }
    string buffer;
    double value;
    string line;
    while(getline(inf, line)){

        istringstream iss(line);
        iss >> buffer >> value;
        if (buffer == "FLUX"){
            return value;
        }
    }
    cout << "Error in finding flux in " << library_path << "/params.txt" << endl;
    return 0;
}





