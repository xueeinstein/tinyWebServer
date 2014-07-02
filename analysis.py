import numpy as np
# analysis the average reponse time of 4-nodes cluster
array1 = [];
array1 = np.genfromtxt("log-4.log",delimiter="\n")
print array1
total1 = np.sum(array1)
print "4-nodes cluster average reponse time: " 
print total1 / len(array1)

# analysis the average reponse time of 2-nodes cluster
array2 = [];
array2 = np.genfromtxt("log-2.log",delimiter="\n")
print array2
total2 = np.sum(array2)
print "2-nodes cluster average reponse time: " 
print total2 / len(array2)

