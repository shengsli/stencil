import sys
import csv
import matplotlib.pyplot as plt

list = sys.argv
if len(list) != 2:
    sys.exit("usage: python3 %s <filename>" % sys.argv[0]);
filename = sys.argv[1]
# print("read file %s" % filename)

with open(filename) as csvfile:
    reader = csv.reader(csvfile, delimiter=',', skipinitialspace=True)
    fstrow = next(reader)
    # print("x-axis is %s" % fstrow[1]) # expect nthreads
    # print("y-axis is %s" % fstrow[0]) # expect time

    times = []
    nthreads = []
    NUMRUNS=5
    sum=0.
    for count, row in enumerate(reader, start=0):
        sum += float(row[0])
        if count % NUMRUNS == 4:
            # nthreads=row[1]
            # time=row[0]
            # print(nthreads, time)
            if row[1]=="0":
                nthreads.append("seq")
            else:
                nthreads.append(row[1])
            avg = sum/NUMRUNS
            times.append(avg)
            sum=0
# print(*times)
# print(*nthreads)

plt.figure()
plt.bar(nthreads, times);
plt.show()
