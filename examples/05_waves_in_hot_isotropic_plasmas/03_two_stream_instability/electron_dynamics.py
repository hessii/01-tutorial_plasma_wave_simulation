# Waves in hot isotropic plasmas - Two-Stream Instability
# Electron dynamics
#

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.colors as colors
import glob
from natsort import natsorted

# import files
f1 = natsorted(glob.glob('./particle-sp_1*.csv'))
f2 = natsorted(glob.glob('./particle-sp_2*.csv'))

# read data
v_sp1 = []
v_sp2 = []
for i in np.arange(len(f1)):
    f = pd.read_csv(f1[i], 'r', skiprows=1, delimiter=',')
    v_sp1.append(f['v1'])

for i in np.arange(len(f2)):
    f = pd.read_csv(f2[i], 'r', skiprows=1, delimiter=',')
    v_sp2.append(f['v1'])

#
#v0 =
v400 = v_sp1[1].append(v_sp2[1])
v800 = v_sp1[3].append(v_sp2[3])
v1200 = v_sp1[5].append(v_sp2[5])
v1600 = v_sp1[7].append(v_sp2[7])
v2000 = v_sp1[9].append(v_sp2[9])

# plot histogram
fig = plt.figure(figsize=(15, 12))
y400, bins = np.histogram(v400, bins=50, density=True)      # density: returns probability density
y800 = np.histogram(v800, bins=50, density=True)      # density: returns probability density
y1200 = np.histogram(v1200, bins=50, density=True)      # density: returns probability density
y1600 = np.histogram(v1600, bins=50, density=True)      # density: returns probability density
y2000 = np.histogram(v2000, bins=50, density=True)      # density: returns probability density

binsize = (bins[1] - bins[0]) / 2
binmean = bins[:-1] + binsize

plt.plot(binmean, y400, c='purple', lw=3, label='400')
plt.plot(binmean, y800[0], c='b', lw=3, label='800')
plt.plot(binmean, y1200[0], c='g', lw=3, label='1200')
plt.plot(binmean, y1600[0], c='orange', lw=3, label='1600')
plt.plot(binmean, y2000[0], c='r', lw=3, label='2000')

plt.minorticks_on()
plt.tick_params(axis='both', which='both', direction='in', labelsize=23)

plt.title('Electron dynamics', fontsize=35, pad=20)
plt.xlabel(r'$v_x/\alpha_h$', fontsize=30, labelpad=10)
plt.ylabel(r'$F_{h,x}$', fontsize=30, labelpad=10)

plt.grid(True, which='major', alpha=0.7)
plt.legend(fontsize=25, facecolor='None', edgecolor='None')

plt.show()

plt.savefig('./electron_dynamics.pdf')



