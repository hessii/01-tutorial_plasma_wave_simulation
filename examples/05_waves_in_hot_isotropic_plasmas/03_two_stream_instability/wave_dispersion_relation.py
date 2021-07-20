# Waves in hot isotropic plasmas - Two-Stream Instability
# Wave dispersion relation
#

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.colors as colo
import glob
from natsort import natsorted

# constants
Dx = 6.283185307179586e-01
Nx = 480
Dt = 1

# import files
field_files = glob.glob('./field*.csv')
field = natsorted(field_files)

# read data
d1=[]; d2=[]; d3=[]; d4=[]
for i in np.arange(len(field)):
    f = pd.read_csv(field[i], 'r', skiprows=1, delimiter=',')
    de1 = f[' dE1']
    if i < 400: d1.append(de1)
    elif i < 1000: d2.append(de1)
    elif i < 1500: d3.append(de1)
    elif i < 2000: d4.append(de1)
dE1 = [d1, d2, d3, d4]      # x components of the electric field

# 2D FFT
FFT = []
freq1 = []
freq2 = []
for i in dE1:
    fft = np.fft.fftshift(np.fft.fft2(i))
    freqw = np.fft.fftshift(np.fft.fftfreq(fft.shape[0], Dt)) * (2 * np.pi)   # frequency w
    freqk = np.fft.fftshift(np.fft.fftfreq(fft.shape[1], Dx)) * (2 * np.pi)   # wave number k
    FFT.append(fft)
    freq1.append(freqw)
    freq2.append(freqk)

# calculate power spectral density
psd = np.abs(FFT)**2 / (Dx * Dt)


# plot
#fig, axes = plt.subplots(2, 2, figsize=(15, 15))
#for i, ax in enumerate(axes.flat):
#    ax.imshow(np.log10(psd[i]))





