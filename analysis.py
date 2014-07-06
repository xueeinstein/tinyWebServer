import numpy as np
import matplotlib.pyplot as plt 
import string

n = raw_input("Please input the max server num: ")
n = string.atoi(n, 10)
x = np.linspace(1, n, n)
y = []
arrays = [[] for i in range(n)]
for i in range(n):
	arrays[i] = np.genfromtxt("log-"+str(i+1)+".log",delimiter="\n")
	# print str(i+1)+"-nodes cluster average reponse time: "
	average_time = np.sum(arrays[i]) / len(arrays[i])
	y.append(average_time)
print x
print y
plt.plot(x, y, color="red", linewidth=2)
plt.xlabel("Num of Servers")
plt.ylabel("Average Respon Time(s)")
plt.ylim(3, 7)
plt.xlim(0.5, 6.5)

plt.show()