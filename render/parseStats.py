"""
    python parseStats.py yAxisLabel (fileName statIdentifier label)+

    stat identifiers are currently:
    f -- frame number (this is automatically put on the x axis
    df -- "drawn fraction", fraction of objects drawn in a frame
    ct -- "culling time", milliseconds that culling logic took this frame

"""


import sys
import matplotlib.pyplot as plt

argc = len(sys.argv)
if argc < 2 + 3 or (argc - 2) % 3 != 0:
    print("Usage: python3 " + sys.argv[0] + " yLabel" + " (fileName statIdentifier label)+")

xDatas = []
yDatas = []
labels = []

def getStat(identifier, line):
    i = line.index(identifier)
    if i == -1:
        raise "Stat identifier not found"
    val = line[i + 1]
    return val


for i in range(2, argc, 3):
    fileName = sys.argv[i]
    f = open(fileName, "r")

    x = []
    y = []
    statId = sys.argv[i + 1]
    label = sys.argv[i + 2]

    for line in f:
        line = line.split(" ")
        frame = int(getStat("f", line))
        x.append(frame)
        data = float(getStat(statId, line))
        y.append(data)

    xDatas.append(x);
    yDatas.append(y)
    labels.append(label)
    f.close()


for i in range(len(xDatas)):
    plt.plot(xDatas[i], yDatas[i], label = labels[i])

plt.legend()

plt.xlabel("frame number")
plt.ylabel(sys.argv[1])
plt.savefig("stats.png")

