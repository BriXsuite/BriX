#ifndef LIBRARYTOOLS_H
#define LIBRARYTOOLS_H

#include <string>
#include <sstream>
#include <cmath>

#include "structures.h"
#include "generaltools.h"

using namespace std;

void LibraryReader(string library_name, string library_path, LibInfo &library);
void IsoBuilder(string library_path, IsoInfo &iso);
double FluxFinder(string library_path);
void StructReader(string library_path, float &struct_prod, float &struct_dest);

#endif // LIBRARYTOOLS_H
