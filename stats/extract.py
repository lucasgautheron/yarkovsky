import csv
import sys
from decimal import Decimal

period = Decimal(sys.argv[2])
shift = period/5

cols = []
lines = []
means = []
max_time = 0

with open(sys.argv[1], 'r') as csvfile:
    reader = csv.reader(csvfile, delimiter=' ', quotechar='"')
    for row in reader:
        if Decimal(row[0]) <= shift:
            continue

        n = 0
        for col in row:
            if not n in cols:
                #print(col)
                cols.append(0)

            cols[n] = Decimal(col)
            n = n + 1
        
        lines.append(cols)
        if (len(lines) % 1000 == 0):
            print(len(lines))

print(len(lines))
max_time = lines[-1][0]

delta = max_time - shift
periods = int(delta/period)

stop = shift + period*periods

print("From ", shift, " to ", max_time, " (", periods, " periods).")

count = 0
means = [0] * len(lines[0])
for l in lines:
    if l[0] > stop:
        break

    means = [x + y for x, y in zip(means, l)]
    count = count + 1

means = [x/count for x in means]


print(means)
