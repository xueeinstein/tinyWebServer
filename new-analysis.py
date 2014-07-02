import numpy as np
# filename = "log.log"
filename = raw_input("please input the log file name: ")
array = [];
array = np.genfromtxt(filename,delimiter="\n")
print array
total = np.sum(array)
print "cluster average reponse time: " 
print total / len(array)