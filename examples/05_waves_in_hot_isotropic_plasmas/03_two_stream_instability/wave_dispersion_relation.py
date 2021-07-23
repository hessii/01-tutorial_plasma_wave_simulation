# Waves in hot isotropic plasmas - Two-Stream Instability
# Wave dispersion relation
#

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.colors as colors
import glob
from natsort import natsorted
from scipy.signal import get_window

# constants
Dx = 6.283185307179586e-01
Nx = 480
Dt = 1

# import files
field_files = glob.glob('./field*.csv')
field = natsorted(field_files)

# read data
dE1 = []
for i in field:
    f = pd.read_csv(i, 'r', skiprows=1, delimiter=',')
    dE1.append(f[' dE1'])

# split dE1 data at four different time ranges
def chunk(list, n):
    return [list[i:i+n] for i in range(0, len(list), n)]

dE1_split = chunk(dE1, 500)

# windowing
wind = np.hanning(500)
wind = np.array(np.meshgrid(wind, wind))

dE1_wind = dE1_split * wind[1][:, :480]

# 2D FFT
fft1 = np.fft.fftshift(np.fft.fft2(dE1_wind[0]))
fft2 = np.fft.fftshift(np.fft.fft2(dE1_wind[1]))
fft3 = np.fft.fftshift(np.fft.fft2(dE1_wind[2]))
fft4 = np.fft.fftshift(np.fft.fft2(dE1_wind[3]))
fft = [fft1, fft2, fft3, fft4]
freq2 = np.fft.fftshift(np.fft.fftfreq(fft1.shape[1], Dx)) * (2 * np.pi)        # wave number k
freq1 = np.fft.fftshift(np.fft.fftfreq(fft1.shape[0], Dt)) * (2 * np.pi)        # frequency w

# calculate power spectral density
psd = []
for i in fft:
    cal = np.abs(i)**2 / (Dt * Dx)
    psd.append(cal)

# plot
fig, axes = plt.subplots(2, 2, figsize=(15, 15))
plt.subplots_adjust(hspace=0.2, wspace=0.2)
labels = [r'$0<t\omega_{pe}<500$', r'$500<t\omega_{pe}<1000$', r'$1000<t\omega_{pe}<1500$', r'$1500<t\omega_{pe}<2000$']
for i, ax in enumerate(axes.flat):
    norm = colors.Normalize(vmin=-3, vmax=7)
    im = ax.imshow(np.log10(psd[i][250:410, 240:337]), origin='lower', norm=norm, cmap='jet', extent=(0, freq2[337], 0, freq1[410]))
    ax.plot(freq2[240:289], 2*freq2[240:289], color='white', ls='--', lw=1.5)

    ax.set_xticks(np.arange(0, 2.5, 0.5))
    ax.set_xticklabels(['', '0.5', '1.0', '1.5', '2.0'])
    ax.set_yticks(np.arange(0, 2.5, 0.5))
    ax.set_yticklabels(['0', '0.5', '1.0', '1.5', '2.0'])
    ax.minorticks_on()
    ax.tick_params(axis='both', labelsize=15)

    ax.text(1.6, 0.15, labels[i], color='white', fontsize=15, ha='center')
    ax.text(0.9, 1.6, r'$v_d$', color='white', fontsize=18, ha='left')

# colorbar
cbar_ax = fig.add_axes([0.92, 0.15, 0.02, 0.7])
cb = fig.colorbar(im, cax=cbar_ax, label=r'$|E_x|^2$', pad=1)
cb.set_label(label=r'$|E_x|^2$', size=22)
cb.ax.tick_params(labelsize=15)

# Get extents of subplot
x0 = min([ax.get_position().x0 for ax in axes.flat])
y0 = min([ax.get_position().y0 for ax in axes.flat])
x1 = max([ax.get_position().x1 for ax in axes.flat])
y1 = max([ax.get_position().y1 for ax in axes.flat])

# Hidden ax for common x and y labels
plt.axes([x0, y0, x1 - x0, y1 - y0/2], frameon=False)
plt.tick_params(labelcolor='none', top=False, bottom=False, left=False, right=False)

# Labelize
plt.title('Wave dispersion relation', fontsize=37)
plt.xlabel(r'$k\alpha_h/\omega_{pe}$', fontsize=28, labelpad=25)
plt.ylabel(r'$\omega/\omega_{pe}$', fontsize=28, labelpad=25)

plt.show()

plt.savefig('./wave_dispersion_relation.pdf')
