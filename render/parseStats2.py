import sys
import matplotlib.pyplot as plt

argc = len(sys.argv)
if argc < 1 + 2:
    print("Usage: python3 " + sys.argv[0] + " (fileName label)+")

fileName = sys.argv[1]


xDatas = []
yDatas = []
labels = []

for i in range(1, argc, 2):
    fileName = sys.argv[i]
    f = open(fileName, "r")

    x = []
    y = []
    lab = sys.argv[i + 1]

    for line in f:
        line = line.split(" ")
        x.append(int(line[0]))
        y.append(float(line[2]) / float(line[1]))
    xDatas.append(x);
    yDatas.append(y)
    labels.append(lab)
    f.close()


for i in range(len(xDatas)):
    plt.plot(xDatas[i], yDatas[i], label = labels[i])

plt.legend()

plt.xlabel("frame number")
plt.ylabel("fraction of models drawn")
plt.ylim(0.0, 1.0)
plt.savefig("stats.png")

