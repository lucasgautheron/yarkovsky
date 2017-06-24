import csv
import sys
from decimal import Decimal

period = Decimal(sys.argv[2])
shift = period/4

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
                print(n)
                cols.append(0)

            cols[n] = Decimal(col)
            n = n + 1
        
        lines.append(cols)

print(len(lines))
max_time = lines[-1][0]

delta = max_time - shift
periods = int(delta/period)

stop = shift + period*periods

count = 0
for l in lines:
    n = 0
    for col in lines[l]:
        if not n in means:
            print(n)
            cols.append(0)
        means[n] = means[n] + l[col]
    count = count + 1

for col in means:
    means[col] = means[col] / count


print(means)
