import matplotlib.pyplot as plt
import csv

x = []
y = []

with open('lt1054-data.csv', 'r') as csvfile:
    csvreader = csv.reader(csvfile)
    for row in csvreader:
        x.append(float(row[0]))
        y.append(float(row[1]))

plt.plot(x, y)
plt.xlabel('I, mA')
plt.ylabel('V')
plt.title('I-V')
plt.show()