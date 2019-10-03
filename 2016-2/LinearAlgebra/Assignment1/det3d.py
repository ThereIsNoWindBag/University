
# coding: utf-8

# In[34]:

import math
import numpy as np

data = [[1, 2, 3] , [2, 3, 4], [3, 4, 6]]

arr = np.array(data)

def det2d(v) :
    return v[0][0]*v[1][1]-v[0][1]*v[1][0]

def det3d(v) : 
    
    det3d = v[0][0]*det2d(v[1:3, 1:3]) - v[1][0]*det2d(np.c_[v[1:3, 0:1], v[1:3, 2:3]]) + v[2][0]*det2d(v[0:2, 0:2])
    
    return det3d
    
print (arr)
print "\ndet3d : ", det3d(arr)


# In[ ]:



