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
    const float flux_value = FluxFinder(library_path);

    ifstream inf(library_path + "/" + to_string(iso.name) + ".txt");
    if(!inf){
        cout << "Failed to read file for " + library_path + " " +  to_string(iso.name) << endl;
        return;
    }

    int i = 0;
    float value;
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
float FluxFinder(string library_path) {

    // Open the params file where flux info is stored
    ifstream inf(library_path + "/params.txt");

    if(!inf){
        cout << "Error attempting to open " << library_path << "/params.txt" << endl;
        return 0;
    }
    string buffer;
    float value;
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

// Finds the structural material contribution
void StructReader(string library_path, float &struct_prod, float &struct_dest) {
// Uses the compositions in structural.txt to combine cross section info in TAPE9.INP

    ifstream struct_file(library_path + "/structural.txt");
    if (!struct_file) {
        cout << "Error! StructReader can't open " << library_path << "/structural.txt  "
        << "Structural neutron production and destruction info will not be used." << endl;
        return;
    }

    ifstream tape9_file(library_path + "/TAPE9.INP");
    if (!tape9_file) {
        cout << "Error! StructReader can't open " << library_path << "/TAPE9.INP  "
        << "Structural neutron production and destruction info will not be used." << endl;
        return;
    }

    unsigned int nucid;
    float fraction;
    float tot_dest = 0;
    float tot_prod = 0;
    string line, library, iso;
    nonActinide x_sections;                     // This stores tape9 cross-sections
    vector<nonActinide> nonActinides;           // This stores isotope compositions
    float sng, sn2n, sna, snp, sngx, sn2nx, yyn;

    // Read the TAPE9 file for cross sections
    while (getline(tape9_file, line)) {
        istringstream iss(line);
        iss >> iso >> iso >> iso;

        if (iso == "MATERIAL"){
            while (getline(tape9_file, line)) {
                istringstream iss1(line);
                iss1 >> library;
                if (library == "-1"){
                    tape9_file.close();
                    for (int i = 0; i < nonActinides.size(); i++) {
                        nonActinides[i].total_prod = 2*nonActinides[i].sn2n + 2*nonActinides[i].sn2nx;
                        nonActinides[i].total_dest = nonActinides[i].snp + nonActinides[i].sng +
                        nonActinides[i].sngx + nonActinides[i].sn2n + nonActinides[i].sn2nx;
                    }
                    goto next;
                }
                iss1 >> iso >> sng >> sn2n >> snp >> sngx >> sn2nx >> yyn;
                x_sections.name = atoi(iso.c_str());
                x_sections.sng = sng;
                x_sections.sn2n = sn2n;
                x_sections.snp = snp;
                x_sections.sngx = sngx;
                x_sections.sn2nx = sn2nx;
                x_sections.yyn = yyn;
                nonActinides.push_back(x_sections);
            }
        }
    }

    next: // Breaks out of the previous nested loop

    // Combine total cross sections with mass compositions
	while (getline(struct_file, line)) {
        istringstream iss(line);
        iss >> nucid >> fraction;

        for(int i = 0; i < nonActinides.size(); i++){
            if(nonActinides[i].name == nucid){
                nucid = nucid % 10000;
                nucid = nucid / 10;

                tot_prod += nonActinides[i].total_prod * fraction * 1000*0.602/nucid;
                tot_dest += nonActinides[i].total_dest * fraction * 1000*0.602/nucid;
            }
        }
    }

    struct_prod = tot_prod;
    struct_dest = tot_dest;
}

// Calculates fuel disadvantage factor
void DACalc(ReactorLiteInfo &reactor_core){
// DA = phi_thermal_Mod / phi_thermal_Fuel

    float a = reactor_core.DA_.a; // radius of the fuel rod
    float b = reactor_core.DA_.b; // radius of the equivalent cell
    float Sig_sF = reactor_core.DA_.fuel_Sig_s; // macroscopic scatter CS of fuel
    float Sig_sM = reactor_core.DA_.mod_Sig_s; //macroscopic scatter CS of moderator
    float Sig_aM = reactor_core.DA_.mod_Sig_a; // macroscopic abs. CS of moderator

    float L_F;      // diffusion length of fuel
    float L_M;      // diffusion length of moderator
    float Sig_aF;   // macroscopic abs. CS of fuel
    float V_F;      // volume of fuel
    float V_M;      // volume of moderator
    float Sig_trF;  // macroscopic transport CS of fuel
    float Sig_trM;  // macroscopic transport CS of moderator
    float Sig_tF;   // macroscopic total CS of fuel
    float Sig_tM;   //macroscopic total CS of moderator
    float D_F;      // diffusion coef. of fuel
    float D_M;      // diffusion coef. of moderator
    float A_F;      // A number of fuel
    float A_M;      // A number of moderator
    float x, y, z;  // calculated equivalent dimensions
    float F, E;     // lattice functions
    float f;        // flux of fuel divided by total flux(fuel+moderator)

    // Moderator calculations
    Sig_tM = Sig_aM + Sig_sM;
    A_F = 235;
    A_M = 18;
    Sig_trM = Sig_tM - 2/3/A_M*Sig_sM;
    D_M = 1 / (3 * Sig_trM);
    L_M = sqrt(D_M/Sig_aM);
    y = a/L_M;
    z = b/L_M;
    V_M = pow(b,2)*3.141592 - pow(a,2)*3.141592;
    V_F = pow(a,2)*3.141592;

    for (int i = 0; i < reactor_core.region.size(); i++) {
/// INCOMPLETE
        //Sig_aF = siga_finder(reactor_core.region[i]);
        //cout << "Siga: " << Sig_aF << endl;

        Sig_tF = Sig_aF+Sig_sF;
        Sig_trF = Sig_tF - 2/3/A_F*Sig_sF;
        D_F = 1 / (3 * Sig_trF);
        L_F = sqrt(D_F/Sig_aF);
        x = a/L_F;

        /*****book example***
        //should get f = 0.8272 with the values below
        //Lamarsh pg.316
        a=1.02;
        b=14.3;
        x=0.658;
        y=0.0173;
        z=0.242;
        V_M=195.6;
        V_F=1;
        Sig_aM=0.0002728;
        Sig_aF=0.3668;
        ******************/

        F = x * boost::math::cyl_bessel_i(0,x) / (2 * boost::math::cyl_bessel_i(1, x));

        E = (z*z - y*y) / (2 * y) * ( (boost::math::cyl_bessel_i(0, y) * boost::math::cyl_bessel_k(1, z)+ boost::math::cyl_bessel_k(0, y) * boost::math::cyl_bessel_i(1, z)) / (boost::math::cyl_bessel_i(1, z) * boost::math::cyl_bessel_k(1, y) - boost::math::cyl_bessel_k(1, z) * boost::math::cyl_bessel_i(1, y)));

        f = pow((((Sig_aM * V_M)/(Sig_aF * V_F)) * F + E), (-1.));
        //cout << f << "  Disadvtg: " << (Sig_aF*V_F - f*Sig_aF*V_F)/(f*Sig_aM*V_M)<<endl;

        reactor_core.region[i].DA = (Sig_aF*V_F - f*Sig_aF*V_F)/(f*Sig_aM*V_M);
    }
}






























