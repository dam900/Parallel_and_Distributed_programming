import matplotlib.pyplot as plt
import os

DIR_NAME = "./data_out/cost"
file_names = os.listdir(DIR_NAME)

data = []
for fn in file_names:
    with open(DIR_NAME + "/" + fn, "r") as f:
        data.append(list(map(float, f.readlines())))

fig = plt.figure()
ax1 = fig.add_subplot(121, projection="3d")
ax2 = fig.add_subplot(122)
t = [j for j in range(len(data[0]))]
for i, line in enumerate(data):
    ax1.plot(t, line, zs=i, zdir="y")
    ax2.plot(t, line)
plt.show()
