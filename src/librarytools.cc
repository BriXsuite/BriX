#include "librarytools.h"

// Reads from library in library_path to build all_iso
void LibraryReader(string library_name, string library_path, LibInfo &library) {

    library.name = library_name;

    ifstream inf (library_path); // Opens manifest file

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

// Reads the specific isotope information
void IsoBuilder(string library_path, IsoInfo &iso) {
    //type = cyclus::Env::GetInstallPath() + "/share/brightlite/" + type;
    const double flux_value = FluxFinder(library_path);
    cout << "returned flux: " << flux_value << endl;

    ifstream inf(library_path + "/" + to_string(iso.name) + ".txt");
    if(!inf){
        cout << "Failed to read file for " + library_path + " " +  to_string(iso.name) << endl;
    }
    //BuildIsotope2(inf, isos[i], flux_value);
    inf.close();
}

// Returns the library base flux value
double FluxFinder(string library_path) {

    // Open the params file where flux info is stored
    ifstream inf(library_path + "/params.txt");

    if(!inf){
        cout << "Error in finding flux in " <<library_path << "/params.txt" << endl;
    }
    string buffer;
    double value;
    string line;
    while(getline(inf, line)){

        istringstream iss(line);
        iss >> buffer >> value;
        if (buffer == "FLUX"){
            cout << "Flux is: " << value << endl;
            return value;
        }
    }
}





