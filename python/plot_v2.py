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
problem_size = int(data[0,2])
ind = reshaped_str_data[:,0]
ind[0] = 1
times = reshaped_data[:,:,0]
time_avgs = np.average(times,axis=1)
time_stds = np.std(times,axis=1)

row_num = times.shape[0]
seq_time = times[0]
seq_time_avg = time_avgs[0]
seq_time_std = time_stds[0]
pthreads_times = times[1:row_num//2+1]
skeleton_times = times[row_num//2+1:]
pthreads_time_avgs = time_avgs[1:row_num//2+1]
skeleton_time_avgs = time_avgs[row_num//2+1:]
pthreads_time_stds = time_stds[1:row_num//2+1]
skeleton_time_stds = time_stds[row_num//2+1:]
seq_ind = ind[0]
threads_ind = ind[row_num//2+1:]

# data for speedup plot
speedups = seq_time_avg/times
speedup_avgs = np.average(speedups,axis=1)
speedup_stds = np.std(speedups,axis=1)
pthreads_speedups = speedups[1:row_num//2+1]
skeleton_speedups = speedups[row_num//2+1:]
pthreads_speedup_avgs = speedup_avgs[1:row_num//2+1]
skeleton_speedup_avgs = speedup_avgs[row_num//2+1:]
pthreads_speedup_stds = speedup_stds[1:row_num//2+1]
skeleton_speedup_stds = speedup_stds[row_num//2+1:]

# data for efficiency plot
threads_num = np.empty(ind.shape)
threads_num[0] = 1
threads_num[1:row_num//2+1] = ind[row_num//2+1:]
threads_num[row_num//2+1:] = ind[row_num//2+1:]
efficiencies = speedups/threads_num[:,None]
efficiency_avgs = np.average(efficiencies,axis=1)
efficiency_stds = np.std(efficiencies,axis=1)
pthreads_efficiencies = efficiencies[1:row_num//2+1]
skeleton_efficiencies = efficiencies[row_num//2+1:]
pthreads_efficiency_avgs = efficiency_avgs[1:row_num//2+1]
skeleton_efficiency_avgs = efficiency_avgs[row_num//2+1:]
pthreads_efficiency_stds = efficiency_stds[1:row_num//2+1]
skeleton_efficiency_stds = efficiency_stds[row_num//2+1:]

# data for cost plot
costs = times*threads_num[:,None]
cost_avgs = np.average(costs,axis=1)
cost_stds = np.std(costs,axis=1)
pthreads_costs = costs[1:row_num//2+1]
skeleton_costs = costs[row_num//2+1:]
pthreads_cost_avgs = cost_avgs[1:row_num//2+1]
skeleton_cost_avgs = cost_avgs[row_num//2+1:]
pthreads_cost_stds = cost_stds[1:row_num//2+1]
skeleton_cost_stds = cost_stds[row_num//2+1:]

# filename
strs = sys.argv[1].split('.')
time_plot_name = strs[0]+"_time.pdf"
speedup_plot_name = strs[0]+"_speedup.pdf"
efficiency_plot_name = strs[0]+"_efficiency.pdf"
cost_plot_name = strs[0]+"_cost.pdf"
idx = strs[0].find('D')
example_name = strs[0][0].upper() + strs[0][1:idx+1]

# plot time
plt.errorbar([0], seq_time_avg, seq_time_std, marker='.', label="sequential")
plt.errorbar(threads_ind, skeleton_time_avgs, skeleton_time_stds, marker='.', label="skeleton")
plt.errorbar(threads_ind, pthreads_time_avgs, pthreads_time_stds, marker='.', label="pthreads")
plt.xlabel("#threads")
plt.ylabel("AVG execution time (second)")
plt.legend()
plt.title("{0}: AVG execution time - #threads, #items = {1:,}".format(example_name, problem_size))
plt.savefig(time_plot_name)
plt.close()

# plot speedup
plt.errorbar(threads_ind, skeleton_speedup_avgs, skeleton_speedup_stds, marker='.', label="skeleton")
plt.errorbar(threads_ind, pthreads_speedup_avgs, pthreads_speedup_stds, marker='.', label="pthreads")
plt.xlabel("#threads")
plt.ylabel(r"AVG speedup = $T_{s} / T_{p}$")
plt.legend()
plt.title("{0}: AVG speedup - #threads, #items = {1:,}".format(example_name, problem_size))
plt.savefig(speedup_plot_name)
plt.close()

# plot efficiency
plt.errorbar(threads_ind, skeleton_efficiency_avgs, skeleton_efficiency_stds, marker='.', label="skeleton")
plt.errorbar(threads_ind, pthreads_efficiency_avgs, pthreads_efficiency_stds, marker='.', label="pthreads")
plt.xlabel("#threads")
plt.ylabel("AVG efficiency (%) = speedup / #threads")
plt.title("{0}: AVG efficiency - #threads, #items = {1:,}".format(example_name, problem_size))
plt.legend()
plt.savefig(efficiency_plot_name)
plt.close()

# plot cost 
plt.errorbar(threads_ind, skeleton_cost_avgs, skeleton_cost_stds, marker='.', label="skeleton")
plt.errorbar(threads_ind, pthreads_cost_avgs, pthreads_cost_stds, marker='.', label="pthreads")
plt.xlabel("#threads")
plt.ylabel(r"AVG cost (seconds) = execution time $\times$ #threads")
plt.title("{0}: AVG cost - #threads, #items = {1:,}".format(example_name, problem_size))
plt.legend()
plt.savefig(cost_plot_name)
plt.close()
