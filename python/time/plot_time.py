import sys
import csv
import numpy as np
import matplotlib.pyplot as plt

arg_num = sys.argv
if len(arg_num) != 3:
    sys.exit("usage: python3 %s <filename.csv> <filename.pdf>" % sys.argv[0]);
filename = sys.argv[1]

data = np.loadtxt(filename, delimiter=',', skiprows=1, dtype='float', usecols=(0,2,3))
str_data = np.loadtxt(filename, delimiter=',', skiprows=1, dtype='str', usecols=(1))

print(data.shape)
print(str_data.shape)

RUNTIMES=5
reshaped_data = data.reshape(-1,RUNTIMES,data.shape[1])
reshaped_str_data = str_data.reshape(-1,RUNTIMES)

ind = reshaped_str_data[:,0]
ind[0] = "seq"
times = reshaped_data[:,:,0]
avgs = np.average(times,axis=1)
stds = np.std(times,axis=1)

row_num = times.shape[0]
seq_time = times[0]
seq_avg = avgs[0]
seq_std = stds[0]
pthreads_times = times[1:row_num//2+1]
skeleton_times = times[row_num//2+1:]
pthreads_avgs = avgs[1:row_num//2+1]
skeleton_avgs = avgs[row_num//2+1:]
pthreads_stds = stds[1:row_num//2+1]
skeleton_stds = stds[row_num//2+1:]
seq_ind = ind[0]
threads_ind = ind[row_num//2+1:]

print(threads_ind.shape)
print(skeleton_times.shape)

plt.figure()
plt.errorbar(threads_ind, skeleton_avgs, skeleton_stds, marker='^', label="skeleton")
plt.errorbar(threads_ind, pthreads_avgs, pthreads_stds, marker='^', label="pthreads")
plt.xlabel("nthreads")
plt.ylabel("time")
plt.legend()
plt.show()
# plt.savefig(sys.argv[2])
