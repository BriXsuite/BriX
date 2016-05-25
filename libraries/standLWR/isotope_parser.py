# Reads in a Bright-lite file and plots the isotopic composition of a Brightlite
# nuclide datatable.

import numpy as np
import scipy as sp
import matplotlib.pyplot as plt
from pyne import nucname
from matplotlib.backends.backend_pdf import PdfPages
import sys

def make_plots(iso_number):
    pp = PdfPages(iso_number + '.pdf')
    iso_name = iso_number +'.txt'
    iso_file = open(iso_name)
    lines = iso_file.readlines()
    iso_file.close()
    # building the time array
    time = np.array(lines[0].split()[1:len(lines[0].split())]).astype(np.float)

    # plotting flux over time
    flux = np.array(lines[1].split()[1:len(lines[1].split())]).astype(np.float)
    plot_flux(time, flux, pp)

    # plotting k and neutron behavior over time
    n_prod = np.array(lines[2].split()[1:len(lines[2].split())]).astype(np.float)
    n_dest = np.array(lines[3].split()[1:len(lines[3].split())]).astype(np.float)
    plot_k(time, n_prod, n_dest, pp)
    
    # plotting burnup over time
    BUd = np.array(lines[4].split()[1:len(lines[4].split())]).astype(np.float)
    plot_burnup(time, BUd, pp)

    # plotting isotopes
    iso_dic = {}
    i = 5
    while i < len(lines):
        iso_ts = lines[i].split()
        iso_array = np.array(iso_ts[1:len(iso_ts)])
        iso_array = iso_array.astype(np.float)
        if max(iso_array) < 1.E-9:
            i+=1
        else:
            iso_dic[iso_ts[0]] = iso_array
            i+=1
    for iso in sorted(iso_dic):
        y = np.array(iso_dic[iso])
        iso_name = nucname.zzzaaa(iso)
        iso_name = iso_name // 1000
        plt.figure(iso_name)
        y = np.append(y, np.zeros(len(time)-len(y)))
        plt.plot(time, y, label=iso)
    i=0
    while i < 110:
        for iso in sorted(iso_dic):
            iso_name = nucname.zzzaaa(iso)
            iso_name = iso_name // 1000
            if iso_name == i:
                plt.figure(iso_name)
                set_axis_iso()
                pp.savefig(iso_name)
                plt.clf()
                i+=1
        i+=1
    pp.close()

def set_axis_iso():
    plt.legend()
    plt.yscale('log')
    plt.xlabel('Days')
    plt.ylabel('Concentration')

def plot_flux(time, flux, pp):
    plt.figure(111)
    plt.plot(time, flux)
    plt.title('Flux')
    plt.xlabel('Days')
    plt.ylabel('n/s/cm^2')
    pp.savefig(111)

def plot_k(time, n_prod, n_dest, pp):
    k = n_prod/n_dest
    plt.figure(112)
    #plt.plot(time, n_prod, label='production (n/cm^2)')
    #plt.plot(time, n_dest, label='destruction (n/cm^2)')
    plt.plot(time, k, label='k')
    plt.legend()
    plt.title('Criticality')
    plt.xlabel('Days')
    plt.ylabel('k')
    pp.savefig(112)

def plot_burnup(time, BU, pp):
    BUd = np.cumsum(BU) 	
    plt.figure(113)
    plt.plot(time, BUd)
    plt.title('Burnup')
    plt.xlabel('Days')
    plt.ylabel('MWd/kgIHM')
    pp.savefig(113)

del(sys.argv[0])
for iso in sys.argv:
    make_plots(iso)
