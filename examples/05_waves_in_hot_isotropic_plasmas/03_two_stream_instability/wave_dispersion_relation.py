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

# split data at four different time ranges
def chunk(list, n):
    return [list[i:i+n] for i in range(0, len(list), n)]

dE1_split = chunk(dE1, 500)

# 2D FFT
fft1 = np.fft.fftshift(np.fft.fft2(dE1_split[0]))
fft2 = np.fft.fftshift(np.fft.fft2(dE1_split[1]))
fft3 = np.fft.fftshift(np.fft.fft2(dE1_split[2]))
fft4 = np.fft.fftshift(np.fft.fft2(dE1_split[3]))
fft = np.array([fft1, fft2, fft3, fft4])
freq2 = np.fft.fftshift(np.fft.fftfreq(fft1.shape[1], Dx)) * (2 * np.pi)        # wave number k
freq1 = np.fft.fftshift(np.fft.fftfreq(fft1.shape[0], Dt)) * (2 * np.pi)        # frequency w

# calculate power spectral density
psd = []
for i in fft:
    # windowing
    wind = np.hamming(len(freq1))
    wind = np.meshgrid(wind, wind)
    fft_wind = i * wind[0][:, :480]

    # power spectral density
    cal = np.abs(fft_wind[250:410, 240:337])**2 / (Dt * Dx)
    psd.append(cal)

# plot
fig, axes = plt.subplots(2, 2, figsize=(10, 10))
plt.subplots_adjust(hspace=0.2, wspace=0.2)
labels = [r'$0<t\omega_{pe}<500$', r'$500<t\omega_{pe}<1000$', r'$1000<t\omega_{pe}<1500$', r'$1500<t\omega_{pe}<2000$']
for i, ax in enumerate(axes.flat):
    norm = colors.Normalize(vmin=-2, vmax=np.log10(np.max(psd[i])))
    ax.imshow(np.log10(psd[i]), origin='lower', norm=norm, cmap='jet', extent=(0, freq2[337], 0, freq1[410]))

    ax.set_xticks(np.arange(0, 2.5, 0.5))
    ax.set_xticklabels(['', '0.5', '1.0', '1.5', '2.0'])
    ax.minorticks_on()

    ax.text(1.2, 0.15, labels[i], color='white', fontsize=12)

plt.show()

# windowing
