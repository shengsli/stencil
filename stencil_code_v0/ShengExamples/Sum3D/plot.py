import sys
import csv
import numpy as np
import matplotlib.pyplot as plt

# input argument
arg_num = sys.argv
if len(arg_num) != 2:
    sys.exit("usage: python3 %s <filename.csv>" % sys.argv[0]);
filename = sys.argv[1]

# read csv
data = np.loadtxt(filename, delimiter=',', skiprows=1, dtype='float', usecols=(0,2,3))
str_data = np.loadtxt(filename, delimiter=',', skiprows=1, dtype='str', usecols=(1))

# prepare data
RUNTIMES=5
reshaped_data = data.reshape(-1,RUNTIMES,data.shape[1])
reshaped_str_data = str_data.reshape(-1,RUNTIMES)

# data for time plot
ind = reshaped_str_data[:,0]
ind[0] = "seq"
times = reshaped_data[:,:,0]
time_avgs = np.average(times,axis=1)
time_stds = np.std(times,axis=1)

row_num = times.shape[0]
seq_time = times[0]
seq_avg = time_avgs[0]
seq_std = time_stds[0]
pthreads_times = times[1:row_num//2+1]
skeleton_times = times[row_num//2+1:]
pthreads_time_avgs = time_avgs[1:row_num//2+1]
skeleton_time_avgs = time_avgs[row_num//2+1:]
pthreads_time_stds = time_stds[1:row_num//2+1]
skeleton_time_stds = time_stds[row_num//2+1:]
seq_ind = ind[0]
threads_ind = ind[row_num//2+1:]

# data for speedup plot
speedups = seq_time/times
speedup_avgs = np.average(speedups,axis=1)
speedup_stds = np.std(speedups,axis=1)
pthreads_speedups = speedups[1:row_num//2+1]
skeleton_speedup = speedups[row_num//2+1:]
pthreads_speedup_avgs = speedup_avgs[1:row_num//2+1]
skeleton_speedup_avgs = speedup_avgs[row_num//2+1:]
pthreads_speedup_stds = speedup_stds[1:row_num//2+1]
skeleton_speedup_stds = speedup_stds[row_num//2+1:]

strs = sys.argv[1].split('.')
time_plot_name = strs[0]+"_time.pdf"
speedup_plot_name = strs[0]+"_speedup.pdf"

plt.errorbar(threads_ind, skeleton_time_avgs, skeleton_time_stds, marker='^', label="skeleton")
plt.errorbar(threads_ind, pthreads_time_avgs, pthreads_time_stds, marker='^', label="pthreads")
plt.xlabel("nthreads")
plt.ylabel("time")
plt.legend()
plt.savefig(time_plot_name)
plt.close()

plt.errorbar(threads_ind, skeleton_speedup_avgs, skeleton_speedup_stds, marker='^', label="skeleton")
plt.errorbar(threads_ind, pthreads_speedup_avgs, pthreads_speedup_stds, marker='^', label="pthreads")
plt.xlabel("nthreads")
plt.ylabel("speedup")
plt.legend()
plt.savefig(speedup_plot_name)
