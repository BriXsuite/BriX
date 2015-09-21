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

    // Erases zero beginnings since these zero-zero points are nonphysical
    if(iso.neutron_prod[0] == 0 || iso.neutron_dest[0] == 0){
        iso.neutron_prod.erase(iso.neutron_prod.begin());
        iso.neutron_dest.erase(iso.neutron_dest.begin());
        iso.BU.erase(iso.BU.begin());
        iso.fluence.erase(iso.fluence.begin());
        for(int j = 0; j < iso.iso_vector.size(); j++){
            iso.iso_vector[j].mass.erase(iso.iso_vector[j].mass.begin());
        }
    }

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
void DACalc(ReactorLiteInfo &core){
// DA = phi_thermal_Mod / phi_thermal_Fuel

    float a = core.DA_.a; // radius of the fuel rod
    float b = core.DA_.b; // radius of the equivalent cell
    float Sig_sF = core.DA_.fuel_Sig_s; // macroscopic scatter CS of fuel
    float Sig_sM = core.DA_.mod_Sig_s; //macroscopic scatter CS of moderator
    float Sig_aM = core.DA_.mod_Sig_a; // macroscopic abs. CS of moderator

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

    for (int i = 0; i < core.region.size(); i++) {
/// INCOMPLETE
        //Sig_aF = siga_finder(core.region[i]);
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

        ///core.region[i].DA = (Sig_aF*V_F - f*Sig_aF*V_F)/(f*Sig_aM*V_M);
    }
}

// Determines the operating mode of reactor and burns fuel accordingly
void BurnFuel(ReactorLiteInfo &core) {

    if(core.target_CR_ < 0) {
        // Reactor is in stop at k=1 mode
        ///TODO normal fuel burn
        CriticalityBurn(core);
    } else {
        ///TODO CR burn
    }

}

// Burns the fuel by advancing fluence based only on criticality
void CriticalityBurn(ReactorLiteInfo &core) {
// yes this IS old burnupcalc
    float kcore = 1.5;
    float kcore_prev;
    unsigned const int regions = core.region.size();
    while(kcore > 1) {
        kcore_prev = kcore; // Save previous k for final interpolation
        FluxCalc(core); // Update relative flux of regions
        // Calculate DA
        if(core.DA_mode_ == 1) {
            DACalc(core);
        }
        // Update fluences
        for(unsigned int reg_i = 0; reg_i < regions; reg_i++) {
            core.region[reg_i].fluence_ += core.region[reg_i].rflux_
                    * core.fluence_timestep_ * core.base_flux_;
        }

        // Recalculate k
        kcore = kCalc(core);
        //std::cout << "k: " << kcore << " BU: " << core.region[0].CalcBU() << std::endl;
    }
    // Find the discharge fluences
    for(int reg_i = 0; reg_i < regions; reg_i++) {
        ///The subtraction here is meh
        core.region[reg_i].fluence_ = Interpolate((core.region[reg_i].fluence_ -
                   core.region[reg_i].rflux_ * core.fluence_timestep_ * core.base_flux_),
                   core.region[reg_i].fluence_, kcore_prev, kcore, 1);
    }
}

///TODO complete all four modes
// Determines the flux calculation method and calls flux function accordingly
void FluxCalc(ReactorLiteInfo &core) {
    const unsigned int mode = core.flux_mode_;
    if(mode == 0) {
        // Simplest mode, all fluxes are 1
        for(unsigned int reg_i = 0; reg_i < core.region.size(); reg_i++){
            core.region[reg_i].rflux_ = 1;
            return;
        }

    }
    else if (mode == 1) {EqPowPhi(core);}
    else if (mode == 2) {}
    else if (mode == 3) {}
    else {
        std::cout << "  Error in flux mode input for ReactorLite" << std::endl;
    }
}

// Calculates the k-value of the core
float kCalc(ReactorLiteInfo &core) {
    unsigned const int regions = core.region.size();
    float prod_tot = 0;
    float dest_tot = 0;

    for(int reg_i = 0; reg_i < regions; reg_i++) {
        prod_tot += ( (core.region[reg_i].CalcProd() +
                       core.struct_prod_ * core.region[reg_i].DA)
                    * core.region[reg_i].rflux_);

        dest_tot += ( (core.region[reg_i].CalcDest() +
                       core.struct_dest_ * core.region[reg_i].DA)
                    * core.region[reg_i].rflux_);
    }

    if(dest_tot <= 0) {return 0;}
    return core.pnl * prod_tot / dest_tot;
}

// Calculates relative fluxes based on the equal power sharing assumption (1)
void EqPowPhi(ReactorLiteInfo &core) {
    // Operates on: Regions of equal power have equal burnup
    // this function updates relative fluxes instead of directly calculating burnup

    float max_fluence = core.fluence_timestep_ * core.base_flux_;
    float bu_old, bu_next, delta_bu, batch_bu;
    float batch_fluence;
    unsigned const int N = core.region.size();
    float max_flux = -1;
    float min_flux = 10;
    unsigned int jk;
    core.region[0].rflux_ = 1;

    // Find the current burnup of the oldest batch
    bu_old = core.region[0].CalcBU();

    // Find the burnup for next step
    // This assumes oldest batch will have the least burnup for a given change in fluence
    bu_next = core.region[0].CalcBU(core.region[0].fluence_ + max_fluence);
    delta_bu = bu_next - bu_old;
    for(int i = 0; i < N; i++){
        batch_bu = core.region[i].CalcBU();
        // find the discrete points before and after batch bu
        for(jk = 0; core.region[i].iso.BU[jk] < batch_bu + delta_bu; jk++){
        }

        batch_fluence = Interpolate(core.region[i].iso.fluence[jk-1], core.region[i].iso.fluence[jk],
                                    core.region[i].iso.BU[jk-1], core.region[i].iso.BU[jk], batch_bu + delta_bu);
        core.region[i].rflux_ = (batch_fluence - core.region[i].fluence_)/(max_fluence);
        if(core.region[i].rflux_ > max_flux){max_flux = core.region[i].rflux_;}
        if(core.region[i].rflux_ < 0){core.region[i].rflux_ = 0;}
        if(core.region[i].rflux_ < min_flux){min_flux = core.region[i].rflux_;}
    }

    for(int i = 0; i < N; i++){
        if(core.region[i].rflux_ == 0){
            core.region[i].rflux_ = min_flux/max_flux;
        } else {
            core.region[i].rflux_ = core.region[i].rflux_ / max_flux;
        }
    }
}




















