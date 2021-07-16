# Waves in cold plasmas
# 02_perpendicular
# propagation angle theta = 90 deg
# Electric field analysis

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.colors as colors
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
fftx = np.fft.fft2(dEx, norm='forward')
fftx = np.fft.fftshift(fftx)
freq1 = np.fft.fftshift(np.fft.fftfreq(fftx.shape[0], Dt)) * (2*np.pi)   # frequency w
freq2 = np.fft.fftshift(np.fft.fftfreq(fftx.shape[1], Dx)) * (2*np.pi)   # wave number k
dk = freq2[1] - freq2[0]
dw = freq1[1] - freq1[0]
psdx = np.abs(fftx)**2 / (dk*dw)     # power spectral density

## dEy
# dEy values(space, time 2D array)
dEy = ext(' dE3')

# 2D FFT
ffty = np.fft.fft2(dEy, norm='forward')
ffty = np.fft.fftshift(ffty)
psdy = np.abs(ffty)**2 / (dk*dw)      # power spectral density

## dEz
# dEz values(space, time 2D array)
dEz = ext(' dE1')

# 2D FFT
fftz = np.fft.fft2(dEz, norm='forward')
fftz = np.fft.fftshift(fftz)
psdz = np.abs(fftz)**2 / (dk*dw)      # power spectral density

# plot
psd = [psdz, psdx, psdy]
label = [r'$E_{1,z}$', r'$E_{1,x}$', r'$E_{1,y}$']
fig, axes = plt.subplots(1, 3, sharex=True, figsize=(25, 15))
for i, ax in enumerate(axes.flat):
    psd_final = psd[i][500:739, 480:595]
    norm = colors.Normalize(vmin=np.log10(2e-13), vmax=np.max(np.log10(psd_final)))
    im = ax.imshow(np.log10(psd_final), cmap='jet', norm=norm, extent=(0, freq2[595], 0, freq1[739]), origin='lower')

    ax.set_xticks(np.arange(6))
    ax.set_xticklabels(['', '1', '2', '3', '4', '5'])
    ax.minorticks_on()
    ax.tick_params(axis='both', which='both', direction='in', labelsize=24)
    ax.tick_params(axis='both', which='major', length=7, width=1.5)

    ax.set_title(label[i], loc='right', fontsize=26)

    # dispersion relation
    x = freq1[500:739]
    alpha = 2
    kap1 = np.sqrt(((x**2 + x - alpha**2)*(x**2 - x - alpha**2))/(alpha**2 * (x**2 - 1 - alpha**2)))

    ax.plot(kap1, x, '--', c='white')

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
plt.title(r'Electric field analysis at $\theta=90^\circ$(perpendicular)', fontsize=35)

plt.show()
plt.savefig('./perpendicular.pdf')

