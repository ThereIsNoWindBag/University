import math
import numpy as np

data = [[1, 2, 3] , [4, 5, 6], [7, 8, 9]]

arr = np.array(data)

def getDet(v) : 
    if len(v) == 2 :
        return v[0][0]*v[1][1]-v[0][1]*v[1][0]
    else :
        asdf = 0
        asdf += pow(-1, 0)*v[0][0]*getDet(v[1:len(v), 1:len(v)])
        
        for i in range(1,len(v) - 1) :
            asdf += pow(-1, i)*v[0][i]*getDet(np.c_[v[1:len(v), 0:i], v[1:len(v), i+1:len(v)]])
            
        asdf += pow(-1, len(v)-1)*v[0][len(v)-1]*getDet(v[1:len(v), 0:len(v)-1])
        return asdf
    
print getDet(arr)