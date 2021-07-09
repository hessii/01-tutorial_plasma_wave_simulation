# Average Field and Plasma Energy
# energy.csv

import numpy as np
import matplotlib.pyplot as plt
import csv
import pandas as pd


'''
f = open('./data/energy.csv', 'r')
reader = csv.DictReader(f)
time = []
dE1 = []
for row in reader:
    time.append(row[' time'])
    dE1.append(float(row[' dE1^2/2'])/(10^(-7)))
plt.plot(time, dE1)
plt.show()
'''

data = pd.read_csv("./data/energy.csv")     #reading csv file with pandas.read_csv

fig = plt.figure(figsize=(35, 25))
plt.plot(data[' time'], data[' dE1^2/2'], 'r', label=r'$\frac{E^2}{2}$', lw=5)
plt.plot(data[' time'], data[' cold_species(1) mU1^2/2'], 'b', label=r'$\frac{mU^2}{2}$', lw=5)
plt.xlim(0, 2*np.pi)

#plt.xaxis.set_major_formatter(plt.FuncFormatter(format_func))
#plt.xticks(plt.FuncFormatter(format_func(data[' time'], )))
plt.xticks(np.arange(0, 2*np.pi + np.pi/4, np.pi/4),
           ['0', '', r'$\pi/2$', '', r'$\pi$', '', r'$3\pi/2$', '', r'$2\pi$'], fontsize=40)
plt.yticks(fontsize=40)

plt.title('Average Field and Plasma Energy', fontsize=65)
plt.xlabel(r'$t\omega_{pe}$', fontsize=45)
plt.ylabel(r'$Energy x 10^{-8}$', fontsize=45)

plt.grid(alpha=0.7)
plt.legend(bbox_to_anchor=(1, 1), loc='upper left', borderaxespad=0., fontsize=50, edgecolor='none', facecolor='none')
plt.savefig('energies.pdf')
plt.show()




