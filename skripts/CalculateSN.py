import numpy as np
import matplotlib
matplotlib.rcParams['text.usetex'] = True
import matplotlib.pyplot as plt
from math import sqrt, log
import argparse
from statistics import fmean, stdev
from uncertainties import ufloat
from matplotlib import rc

parser = argparse.ArgumentParser(
    description='Compute S/N and other properties for a Micromegas detector',
    formatter_class=argparse.ArgumentDefaultsHelpFormatter
)

parser.add_argument("-m", "--multiplexing", type=int,   default=5,                         help='multiplexing factor of the micromegas')
parser.add_argument("-p", "--pitch",        type=float, default=0.250,                     help='pitch size of the micromegas [mm]')
parser.add_argument("-l", "--length",       type=float, default=250,                        help='total length of the micromegas [mm]')
parser.add_argument("-r", "--reference",    type=float, default=[80, 0.250, 5],              help='reference point of S/N following the form (length, pitch, multiplexing)', nargs=3)
parser.add_argument("-sn", "--snratio",     type=float, default=[164, 151, 191, 180],      help='S/N measured in 2018, default is Y plane in run 4038, assuming 10 adc noise', nargs=4)

args = parser.parse_args()


#first compute mean S/N and std
SN = ufloat( fmean(args.snratio), stdev(args.snratio) )
print(SN)


pitch = np.linspace(0.05, 1, 200) #x plane to identify

#compute S/N starting from reference values
SN_extrapolated = SN.nominal_value * (args.reference[1] / pitch) * (args.reference[2] / args.multiplexing) * (args.reference[0] / args.length)


#plotting
fig = plt.figure()
ax  = fig.add_subplot(1, 1, 1)
#plot data
data = ax.errorbar(args.reference[1], SN.nominal_value, yerr=SN.std_dev, fmt='o')
#plot extrapolation
extrapolation = ax.plot(pitch, SN_extrapolated, 'r')

ax.set_title('S/N vs pitch size for a length of {:0.1f} mm and a multiplexing factor of {:d}'.format(args.length, args.multiplexing), fontsize=16)
#axis labels
ax.yaxis.label.set_size(16)
ax.xaxis.label.set_size(16)
ax.set_xlabel(r'pitch size [mm]')
ax.set_ylabel(r'Signal to Noise')
#axis limit
ax.set_xlim([0.2, 1.])
ax.set_ylim([0, 210])
#second axis with channel numbers
nchan    = lambda x: (args.length / x)/args.multiplexing
nchaninv = lambda x: (args.length / x)/args.multiplexing
ax2 = ax.secondary_xaxis("top", functions=(nchan, nchaninv))
ax2.set_xlabel('Number of channels nedded')
ax2.xaxis.label.set_size(16)
ax2.spines['top'].set_color('red')
ax2.tick_params(axis='x', colors='red')
ax2.xaxis.label.set_color('red')

#other axis to show resolution of detector
res    = lambda x: (x/sqrt(12))
resinv = lambda x: (x*sqrt(12))
ax3 = ax.secondary_xaxis("top", functions=(res, resinv))
ax3.set_xlabel('resolution of the plane [mm]')
ax3.xaxis.labelpad = 20
ax3.xaxis.label.set_size(16)
ax3.spines['bottom'].set_color('blue')
ax3.tick_params(axis='x', colors='blue')
ax3.xaxis.label.set_color('blue')

#draw a line for a reference point where S/N ratio start to be dangerously low
SN_acceptable = 20 #why 20? good question
limit = ax.axhline(y=SN_acceptable, xmin=0, xmax=1.2, linewidth=2, linestyle='--', color='g')

#draw a line to show previous chip channel number
nchan_reference = 64 #64 channel per plane were previously achieved
print(nchaninv(nchan_reference))
oldchan = ax.axvline(x=nchaninv(nchan_reference),   ymin=0, ymax=210, linewidth=2, linestyle='--', color='r')
oldchip = ax.axvline(x=nchaninv(2*nchan_reference), ymin=0, ymax=210, linewidth=2, linestyle='--', color='orange')
estimate = ax.axvline(x=0.25*sqrt(12), ymin=0, ymax=210, linewidth=2, linestyle='--', color='b')

#legend
ax.legend((data, extrapolation, limit, oldchan, oldchip, estimate), ('data measured in 2018', 'extrapolation of data to larger pitch', 'S/N limit', '64 channel as old plane', 'full APV chip', 'estimate from Henri'))

plt.show()
