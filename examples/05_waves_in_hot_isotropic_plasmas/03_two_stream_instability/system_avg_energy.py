# Waves in hot isotropic plasmas - Two-Stream Instability
# Average energy in the system
#

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.colors as colors
import glob
from natsort import natsorted

# constants
a_h = 1         # thermal speed of
v_d = 2 * a_h   # drift speed

# import data energy.csv
f = pd.read_csv('./energy.csv', 'r', delimiter=',')

# extract data
dE1 = f[' dE1^2/2']                         # electric field wave energy density
ps1_mv1 = f[' part_species(1) mv1^2/2']
ps2_mv1 = f[' part_species(2) mv1^2/2']
cs_mv1 = f[' cold_species(1) mv1^2/2']
ked = ps1_mv1 + ps2_mv1 + cs_mv1            # total kinetic energy density
t = f[' time']
steps = len(f['step'])

# plot
fig = plt.subplots(2, 1, figsize=(12, 15))
fig1 = plt.subplot(211)
plt.scatter(t, np.log10(dE1), s=2, label=r'$E_{1,x}$')
plt.xticks(np.arange(0, 2100, 100),
           ['0', '', '', '', '', '500', '', '', '', '', '1000', '', '', '', '', '1500', '', '', '', '', '2000'])
#plt.xticks(np.arange(0, 2100, 100), ['{:d}'.format(x) for x in range(0, 2100, 100) if x%500==0])
plt.legend(edgecolor='None', facecolor='None')

fig2 = plt.subplot(212)
plt.plot(t, dE1-dE1[0], label=r'$E_{1,x}$', c='r')
plt.plot(t, ked-ked[0], label=r'$KE-KE_0$', c='b')
plt.plot(t, np.zeros(steps), '--', color='k', label='total')
plt.xticks(np.arange(0, 2100, 100),
           ['0', '', '', '', '', '500', '', '', '', '', '1000', '', '', '', '', '1500', '', '', '', '', '2000'])
plt.legend(edgecolor='None', facecolor='None')

plt.show()


