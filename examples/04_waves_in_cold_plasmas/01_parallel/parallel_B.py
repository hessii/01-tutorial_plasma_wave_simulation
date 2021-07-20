# Waves in cold plasmas
# 01_parallel
# propagation angle theta = 0 deg
# Magnetic field analysis

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

## dBx
# dBx values(space, time 2D array)
dBx = ext(' dB2')

# 2D FFT
fftx = np.fft.fft2(dBx, norm='forward')
fftx = np.fft.fftshift(fftx)
freq1 = np.fft.fftshift(np.fft.fftfreq(fftx.shape[0], Dt)) * (2*np.pi)   # frequency w
freq2 = np.fft.fftshift(np.fft.fftfreq(fftx.shape[1], Dx)) * (2*np.pi)   # wave number k
dk = freq2[1] - freq2[0]
dw = freq1[1] - freq1[0]
psdx = np.abs(fftx)**2 / (dk*dw)     # power spectral density

## dBy
# dBy values(space, time 2D array)
dBy = ext(' dB3')

# 2D FFT
ffty = np.fft.fft2(dBy, norm='forward')
ffty = np.fft.fftshift(ffty)
psdy = np.abs(ffty)**2 / (dk*dw)      # power spectral density

## dBz
# dBz values(space, time 2D array)
dBz = ext('dB1')

# 2D FFT
fftz = np.fft.fft2(dBz, norm='forward')
fftz = np.fft.fftshift(fftz)
psdz = np.abs(fftz)**2 / (dk*dw)      # power spectral density

# plot
psd = [psdz, psdx, psdy]
label = [r'$dB_z$', r'$dB_x$', r'$dB_y$']
fig, axes = plt.subplots(1, 3, sharex=True, figsize=(25, 15))
for i, ax in enumerate(axes.flat):
    im = ax.contourf(freq2, freq1, np.log10(psd[i]))
    ax.set_xlim(0, 5)
    ax.set_ylim(0, 10)
x    ax.set_xticklabels(['', '1', '2', '3', '4', '5'])
    ax.minorticks_on()
    ax.tick_params(axis='both', which='both', direction='in', labelsize=24)
    ax.tick_params(axis='both', which='major', length=7, width=1.5)
    ax.set_title(label[i], loc='right', fontsize=26)

# need to study the codes below
# Get extents of subplot
x0 = min([ax.get_position().x0 for ax in axes])
y0 = min([ax.get_position().y0 for ax in axes])
x1 = max([ax.get_position().x1 for ax in axes])
y1 = max([ax.get_position().y1 for ax in axes])

# Hidden ax for common x and y labels
plt.axes([x0, y0, x1 - x0, y1 - y0/2], frameon=False)
plt.tick_params(labelcolor='none', top=False, bottom=False, left=False, right=False)

# Labelize
plt.xlabel(r'$kd_e$', fontsize=30)
plt.ylabel(r'$\omega/\Omega_{ce}$', fontsize=30)
plt.title(r'Magnetic field analysis at $\theta=0^\circ$(parallel)', fontsize=35)

plt.show()
#plt.savefig('./parallel_B.pdf')