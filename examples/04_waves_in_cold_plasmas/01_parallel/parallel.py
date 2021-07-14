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

## dEx
# dEx values(space, time 2D array)
dEx = ext(' dE2')

# 2D FFT
fft = np.fft.fft2(dEx, norm='forward')
fft = np.fft.fftshift(fft)
freq1 = np.fft.fftshift(np.fft.fftfreq(fft.shape[0], Dt)) * (2*np.pi)   # frequency w
freq2 = np.fft.fftshift(np.fft.fftfreq(fft.shape[1], Dx)) * (2*np.pi)   # wave number k
dk = freq2[1] - freq2[0]
dw = freq1[1] - freq1[0]
psd = np.abs(fft)**2 / (dk*dw)      # power spectral density
i = freq2 > 0

# plot
fig = plt.subplots(1, 3, figsize=(25, 15))

plt.subplot(1, 3, 2)
plt.contourf(freq2, freq1, np.log10(psd))
plt.xlim(0, 5)
plt.ylim(0, 10)

plt.xticks(np.arange(6), ['', '1', '2', '3', '4', '5'], fontsize=24)
plt.yticks(fontsize=24)
plt.title(r'$E_{1,x}$', fontsize=28, position=(0.9, 0))


## dEy
# dEy values(space, time 2D array)
dEy = ext(' dE3')

# 2D FFT
fft = np.fft.fft2(dEy, norm='forward')
fft = np.fft.fftshift(fft)
psd = np.abs(fft)**2 / (dk*dw)      # power spectral density
i = freq2 > 0

# plot
plt.subplot(1, 3, 3)
plt.contourf(freq2, freq1, np.log10(psd))
plt.xlim(0, 5)
plt.ylim(0, 10)

plt.colorbar()
plt.xticks(np.arange(6), ['', '1', '2', '3', '4', '5'], fontsize=24)
plt.yticks(fontsize=24)
plt.title(r'$E_{1,y}$', fontsize=28, position=(0.9, 0))


## dEz
# dEz values(space, time 2D array)
dEz = ext(' dE1')

# 2D FFT
fft = np.fft.fft2(dEz, norm='forward')
fft = np.fft.fftshift(fft)
psd = np.abs(fft)**2 / (dk*dw)      # power spectral density
i = freq2 > 0

# plot
plt.subplot(1, 3, 1)
plt.contourf(freq2, freq1, np.log10(psd))
plt.xlim(0, 5)
plt.ylim(0, 10)

plt.xticks(np.arange(6), ['', '1', '2', '3', '4', '5'], fontsize=24)
plt.yticks(fontsize=24)
plt.title(r'$E_{1,z}$', fontsize=28, position=(0.9, 0))

#plt.savefig('./parallel_Ex.pdf')
plt.show()