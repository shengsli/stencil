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
data = np.loadtxt(filename, delimiter=',', skiprows=1, dtype='float', usecols=(0,1,2,3))

# prepare data
RUNTIMES=5
PROBLEMSIZE_NUM=5
THREAD_LINE_NUM=int(data.shape[0]/RUNTIMES/PROBLEMSIZE_NUM)
reshaped_data = data.reshape(-1,RUNTIMES,data.shape[1])
threads_num = reshaped_data[:,:,1][:,0].astype(int)
times = reshaped_data[:,:,0]
time_avgs = np.average(times,axis=1)
time_stds = np.std(times,axis=1)
problem_sizes = reshaped_data[:,:,3][:,0].astype(int).astype(str)

strs = sys.argv[1].split('.')
time_plot_name = strs[0]+".pdf"
idx = strs[0].find('D')
example_name = strs[0][0].upper() + strs[0][1:idx+1]

# run 5 times with certain number of threads, plot one line
for idx in range(THREAD_LINE_NUM):
    plt.errorbar(problem_sizes[idx*PROBLEMSIZE_NUM:(idx+1)*PROBLEMSIZE_NUM], 
                 time_avgs[idx*PROBLEMSIZE_NUM:(idx+1)*PROBLEMSIZE_NUM],
                 time_stds[idx*PROBLEMSIZE_NUM:(idx+1)*PROBLEMSIZE_NUM],
                 marker='.', label="#threads={0}".format(threads_num[idx*PROBLEMSIZE_NUM]))

# plt.axis()
plt.xlabel("#items")
plt.ylabel("AVG execution time (second)")
plt.legend()
plt.title("{0}: AVG execution time - #items".format(example_name))
plt.savefig(time_plot_name)
plt.close()
