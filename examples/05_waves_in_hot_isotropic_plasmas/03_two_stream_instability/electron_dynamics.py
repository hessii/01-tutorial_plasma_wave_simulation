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

# read vx at t = 400, 800, 1200, 1600, 2000
v400 = v_sp1[1].append(v_sp2[1])
v800 = v_sp1[3].append(v_sp2[3])
v1200 = v_sp1[5].append(v_sp2[5])
v1600 = v_sp1[7].append(v_sp2[7])
v2000 = v_sp1[9].append(v_sp2[9])
v = (v400, v800, v1200, v1600, v2000)

# plot histogram
fig = plt.figure(figsize=(15, 12))

colors = ['purple', 'b', 'g', 'orange', 'r']
labels = ['400', '800', '1200', '1600', '2000']

for i in np.arange(5):
    hist, bins = np.histogram(v[i], bins=50, density=True)
    binsize = (bins[1] - bins[0]) / 2
    binmean = bins[:-1] + binsize

    plt.plot(binmean, hist, c=colors[i], lw=3, label=labels[i])

plt.minorticks_on()
plt.tick_params(axis='both', which='both', direction='in', labelsize=23)

plt.title('Electron dynamics', fontsize=35, pad=20)
plt.xlabel(r'$v_x/\alpha_h$', fontsize=30, labelpad=10)
plt.ylabel(r'$F_{h,x}$', fontsize=30, labelpad=10)

plt.grid(True, which='major', alpha=0.7)
plt.legend(fontsize=25, facecolor='None', edgecolor='None')

plt.show()

plt.savefig('./electron_dynamics.pdf')



