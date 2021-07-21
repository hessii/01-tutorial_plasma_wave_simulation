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
a_h = 1  # thermal speed of
v_d = 2 * a_h  # drift speed

# import data energy.csv
f = pd.read_csv('./energy.csv', 'r', delimiter=',')

# extract data
dE1 = f[' dE1^2/2']  # electric field wave energy density
ps1_mv1 = f[' part_species(1) mv1^2/2']
ps2_mv1 = f[' part_species(2) mv1^2/2']
cs_mv1 = f[' cold_species(1) mv1^2/2']
ked = ps1_mv1 + ps2_mv1 + cs_mv1  # total kinetic energy density
t = f[' time']
steps = len(f['step'])


# plot
#fig = plt.subplots(2, 1, figsize=(12, 15))
#fig1 = plt.subplot(211)
#plt.scatter(t, np.log10(dE1), s=2, label=r'$E_{1,x}$')
#plt.xticks(np.arange(0, 2100, 100),
#           ['0', '', '', '', '', '500', '', '', '', '', '1000', '', '', '', '', '1500', '', '', '', '', '2000'])
# plt.xticks(np.arange(0, 2100, 100), ['{:d}'.format(x) for x in range(0, 2100, 100) if x%500==0])
#plt.tick_params(axis='both', which='both', labelsize=20)
#plt.legend(edgecolor='None', facecolor='None', fontsize=20)

#fig2 = plt.subplot(212)
#plt.plot(t, dE1 - dE1[0], label=r'$E_{1,x}$', c='r')
#plt.plot(t, ked - ked[0], label=r'$KE-KE_0$', c='b')
#plt.plot(t, np.zeros(steps), '--', color='k', label='total')
#plt.xticks(np.arange(0, 2100, 100),
#           ['0', '', '', '', '', '500', '', '', '', '', '1000', '', '', '', '', '1500', '', '', '', '', '2000'])
#plt.minorticks_on()
#plt.tick_params(axis='both', which='both', labelsize=20)
#plt.legend(edgecolor='None', facecolor='None', fontsize=20)


# graph smoothing
def chunk(list, n):
    return [list[i:i + n] for i in range(len(list))]  # split list into chunks of n data

def smooth(data, n):  # smooth curve by averaging 10 data points
    y = []
    data_chunked = chunk(data, n)
    for i in data_chunked:
        yavg = np.average(i)
        y.append(yavg)
    return y

# smoothed graph
fig3, (ax1, ax2) = plt.subplots(2, 1, figsize=(12, 15))
t_smth = smooth(t, 100)
dE1_smth = smooth(dE1, 100)
ked_smth = smooth(ked, 100)

# first graph - electric field energy curve
ax1.plot(t_smth, np.log10(dE1_smth), 'k', label=r'$E_{1,x}$')
plt.xlim(0, 2000)
ax1.set_xticks(np.arange(0, 2100, 500))
ax1.set_xticklabels(['0', '500', '1000', '1500', '2000'])
ax1.minorticks_on()
ax1.tick_params(axis='both', which='both', labelsize=20)
ax1.grid(b=True, which='major', alpha=0.7)
ax1.legend(edgecolor='None', facecolor='None', fontsize=20)

# curve fitting. finding growth rate gamma
x = t[250:1500]
y = dE1_smth[250:1500]
y_fit = np.polyfit(x, np.log10(y), 1)
ax1.plot(x, y_fit[0]*x + y_fit[1], c='r', ls='--', lw=2)
ax1.text(250, 10e-5, r'$\gamma=0.010\omega_{pe}$', c='r', size=15 ,ha='right')

# second graph - comparison of the field energy and the kinetic energy
ax2.plot(t, dE1_smth - dE1_smth[0], label=r'$E_{1,x}$', c='r')
ax2.plot(t, ked_smth - ked_smth[0], label=r'$KE-KE_0$', c='b')
ax2.plot(t, np.array(dE1_smth - dE1_smth[0]) + np.array(ked_smth - ked_smth[0]), '--', label='total', c='k')
plt.xlim(0, 2000)
ax2.set_xticks(np.arange(0, 2100, 500))
ax2.set_xticklabels(['0', '500', '1000', '1500', '2000'])
ax2.minorticks_on()
ax2.tick_params(axis='both', which='both', labelsize=20)
ax2.grid(b=True, which='major', alpha=0.7)
ax2.legend(edgecolor='None', facecolor='None', fontsize=20)
plt.xlabel(r'$t\omega_{pe}$', fontsize=25)

plt.show()
plt.savefig('./system_avg_energy.pdf')
