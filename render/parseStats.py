import sys
import matplotlib.pyplot as plt

argc = len(sys.argv)
if argc != 2:
    print("Usage: python3 " + sys.argv[0] + " fileName")

fileName = sys.argv[1]

f = open(fileName, "r")

frames = []
totals = []
drawns = []

for line in f:
    line = line.split(" ")
    frames.append(int(line[0]))
    totals.append(int(line[1]))
    drawns.append(int(line[2]))

fracs = [drawns[i] / float(totals[i]) for i in range(0, len(drawns))]
f.close()

plt.plot(frames, fracs)
plt.xlabel("frame number")
plt.ylabel("fraction of models drawn")
plt.ylim(0.0, 1.0)
plt.savefig("stats.png")

