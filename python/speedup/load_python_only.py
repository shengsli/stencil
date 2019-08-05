import sys
import csv
import numpy as np
import matplotlib.pyplot as plt

numArgs = sys.argv
if len(numArgs) != 3:
    sys.exit("usage: python3 %s <filename.csv> <filename.pdf>" % sys.argv[0]);
filename = sys.argv[1]

with open(filename, 'r') as f:
    reader = csv.reader(f, delimiter=',')
    headers = next(reader)
    data = list(reader)
    data = np.array(data).astype(str)

print(data.shape)
