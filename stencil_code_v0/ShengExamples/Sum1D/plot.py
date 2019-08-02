import sys
import csv
import numpy as np
import matplotlib.pyplot as plt

list = sys.argv
if len(list) != 3:
    sys.exit("usage: python3 %s <filename.csv> <filename.pdf>" % sys.argv[0]);
filename = sys.argv[1]
# print("read file %s" % filename)

skeletonTimesStds = []
pthreadsTimesStds = []

with open(filename) as csvfile:
    reader = csv.reader(csvfile, delimiter=',', skipinitialspace=True)
    fstrow = next(reader)
    # print("x-axis is %s" % fstrow[1]) # expect nthreads
    # print("y-axis is %s" % fstrow[0]) # expect time

    skeletonTimes = []
    pthreadsTimes = []
    nthreads = []
    NUMRUNS=5
    sum=0.
    tempStds = []
    for count, row in enumerate(reader, start=0):
        tempStds.append(row[0])
        sum += float(row[0])
        if count % NUMRUNS == 4:
            avg = sum/NUMRUNS
            tempStdsNdarray = np.array(tempStds, dtype='f')
            tempStd = np.std(tempStdsNdarray)
            if row[1]=="0":
                skeletonTimes.append(avg)
                nthreads.append("seq")
                skeletonTimesStds.append(tempStd)
            elif row[1][0]=='p':
                pthreadsTimes.append(avg)
                pthreadsTimesStds.append(tempStd)
            else:
                nthreads.append(row[1])
                skeletonTimes.append(avg)
                skeletonTimesStds.append(tempStd)
            sum=0
            tempStds = []

# print(*skeletonTimes)
# print(*nthreads)
# print(*pthreadsTimes)

ind = range(len(nthreads))
# plt.figure()
plt.bar(nthreads[0], skeletonTimes[0], yerr=skeletonTimesStds[0], label="sequential")
plt.bar(nthreads[1:], skeletonTimes[1:], yerr=skeletonTimesStds[1:], label="skeleton")
plt.bar(nthreads[1:], pthreadsTimes, yerr=pthreadsTimesStds, label="pthread")
plt.xlabel("nthreads")
plt.ylabel("time")
plt.legend()
# plt.show()
plt.savefig(sys.argv[2])
