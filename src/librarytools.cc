#include "librarytools.h"


void LibraryReader(std::string library_path, LibInfo &library){
    std::cout << "Library reader begin. Path:" << library_path << std::endl;

    std::ifstream inf(library_path); // Opens manifest file

    std::string line;
    std::string iso_name;

    library.name = libraries[0]; //for now only one entry in here

}
