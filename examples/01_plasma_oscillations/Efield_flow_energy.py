# Electric Field and Flow Energy
# field.csv
# moment.csv

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import glob
import matplotlib

field_files = glob.glob('./data/field*.csv')    # make list of all field files
moment_files = glob.glob('./data/moment*.csv')  # make list of all moment files

# functions that extract dE1, v1 values at n-th x grid
def dE1_n(n):
    dE1 = []
    for i in np.arange(1000):
        f = pd.read_csv(field_files[i], skiprows=1)     #read csv file, skip first row(information in the first row)
        dE1.append(f[' dE1'][n])
    return dE1

def v1_n(n):
    v1 = []
    for i in np.arange(1000):
        f = pd.read_csv(moment_files[i], skiprows=1)
        v1.append(f[' cold_species(1) <v1>'][n])
    return v1

# time
f = pd.read_csv('./data/energy.csv')
t = f[' time']

# dE1, v1 at 1st x grid
dE1 = dE1_n(0)
v1 = v1_n(0)

# E_x, U_x
E_x = dE1
U_x = v1

# plot Ux, Ex against time
fig = plt.figure()
plt.plot(t, E_x, 'r', label=r'$E_x$')
plt.plot(t, U_x, 'b', label=r'$U_x$')
plt.xlim(0, 6*np.pi)

plt.gca().spines['bottom'].set_position(('data', 0))        # plt.gca(): current axis location
                                                            # this step is needed to relocate the axis
plt.gca().spines['top'].set_visible(False)
plt.xticks(np.arange(0, 7*np.pi, np.pi), ['', r'$\pi$', r'$2\pi$', r'$3\pi$', r'$4\pi$', r'$5\pi$', ''],
           fontsize=20)
plt.yticks(fontsize=20)
plt.minorticks_on()     # use minor ticks
plt.tick_params(which='minor', axis='x', bottom=False)      # turn off x axis minor ticks
plt.tick_params(axis='y', direction='in')                   # place major ticks inside
plt.tick_params(which='minor', axis='y', direction='in')    # place minor ticks inside

plt.title(r'Phase Difference Between $E_x\ and\ U_x$', fontsize=27)     # align x axis at 0
plt.xlabel(r'$t\omega_{pe}$', fontsize=22, loc='right', labelpad=-20)
#matplotlib.axis.Axis.set_label_coords()
plt.ylabel('', fontsize=22)

plt.grid(alpha=0.7)
plt.legend(loc='best', fontsize=22, edgecolor='none', facecolor='none')

plt.show()

'''
fig2 = plt.figure()
dx = 0.200781
for i in np.arange(1000):
    x = np.arange(dx, dx*11, dx)
    f = pd.read_csv(field_files[i], skiprows=1)
    y = f[' dE1']
    plt.plot(x, y)
plt.show()
'''
