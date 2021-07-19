# Longitudinal wave dispersion relation
#

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.colors as colors
import glob
from natsort import natsorted

w_pe = 1            # plasma frequency
alpha = 1           # thermal velocity of the Maxwellian distribution
Dx = np.pi / 10     # grid size
Nx = 480            # number of grid points
Dt = 0.04 / w_pe    # time step

# import data files
field = glob.glob('./field*.csv')
field_files = natsorted(field)

# extract data
Ex = []
for i in field_files:
    f = pd.read_csv(i, 'r', skiprows=1, delimiter=',')
    Ex.append(f[' dE1'])

# 2D FFT
fft = np.fft.fftshift(np.fft.fft2(Ex))
freq1 = np.fft.fftshift(np.fft.fftfreq(fft.shape[0], Dx)) * (2 * np.pi)     # wave number ka/w_pe
freq2 = np.fft.fftshift(np.fft.fftfreq(fft.shape[1], Dt))                   # frequency w/w_pe

# power spectrum density
psd = np.abs(fft)**2 / (Dx * Dt)
psd_plot = psd[250:376, 240:337]

# plot power spectrum
fig = plt.figure(figsize=(10,10))
norm = colors.Normalize(vmin=1.8, vmax=np.max(np.log10(psd_plot)))
plt.imshow(np.log10(psd_plot), cmap='jet', origin='lower', norm=norm, extent=[0, freq2[337], 0, freq1[376]])

plt.colorbar(fraction=0.045)

plt.title(r'$E_{1,x}$', loc='right', fontsize=27)
plt.xlabel(r'$k\alpha/\omega_{pe}$', fontsize=25)
plt.ylabel(r'$\omega/\omega_{pe}$', fontsize=25)

plt.xticks(np.arange(6), ['', '1', '2', '3', '4', '5'], fontsize=20)
plt.yticks(fontsize=18)
plt.minorticks_on()

# dispersion relation
kap = np.sqrt(freq1[250:376])
plt.plot(kap, freq1[250:376], '--', color='white')

plt.show()

plt.savefig('./longitudinal_wave_dispersion_relation.pdf', dpi=300)