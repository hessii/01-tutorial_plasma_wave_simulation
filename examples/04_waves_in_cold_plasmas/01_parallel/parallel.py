# Waves in cold plasmas
# 01_parallel
# propagation angle theta = 0 deg

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import glob
from natsort import natsorted

Dx = 1.5e-1
Nx = 960

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

