# Waves in cold plasmas
# 01_parallel
# propagation angle theta = 0 deg

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from scipy import signal
import seaborn as sns
import glob
from natsort import natsorted

Dx = 1.5e-1
Nx = 960
Dt = 1.5e-1

# import all field files
field = glob.glob('field*.csv')
field_files = natsorted(field)

# function that extracts [index] column values into 2D array (time, [index] column values)
def ext(index):
    alldata = []
    for i in range(len(field_files)):
        f = pd.read_csv(field_files[i], 'r', delimiter=',', skiprows=1)
        val = f[index]
        alldata.append(val)
    return np.array(alldata)

# dEx values(space, time 2D array)
dEx = ext(' dE2')

# 2D FFT
fft = np.fft.fft2(dEx, norm='forward')
fft = np.fft.fftshift(fft)
freq1 = np.fft.fftshift(np.fft.fftfreq(fft.shape[0], Dt)) * (2*np.pi)
freq2 = np.fft.fftshift(np.fft.fftfreq(fft.shape[1], Dx)) * (2*np.pi)
dk = freq2[1] - freq2[0]
dw = freq1[1] - freq1[0]
psd = np.abs(fft)**2 / (dk*dw)      # power spectral density
i = freq2 > 0

# plot
fig1 = plt.figure(figsize=(10, 15))
plt.contourf(freq2, freq1, np.log10(psd))
plt.xlim(0, 5)
plt.ylim(0, 10)

plt.colorbar()
plt.xticks(np.arange(6), ['', '1', '2', '3', '4', '5'], fontsize=24)
plt.yticks(fontsize=24)
plt.xlabel(r'$kd_e$', fontsize=28)
plt.ylabel(r'$\omega/\Omega_{ce}$', fontsize=28)
plt.title(r'$E_{1,x}$', fontsize=28, position=(0.9, 0))

plt.savefig('./parallel_Ex.pdf')

plt.show()
