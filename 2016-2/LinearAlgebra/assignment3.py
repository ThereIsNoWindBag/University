import math
import numpy as np
import copy

from sympy import Symbol, solve, symbols

alpha = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z']

data = [[1, 1, 1] , [0, 5, 6], [0, 0, 9]] #여기에 구하고자 하는 행렬을 입력

arr = np.array(data)
 
def getDet(v) : 
    if len(v) == 2 :
        return v[0][0]*v[1][1]-v[0][1]*v[1][0]
    else :
        equation = 0
        nv = []
        
        for i in range(1, len(v)) :
            nv.append(v[i][1:len(v)]) 
        equation += pow(-1, 0)*v[0][0]*getDet(nv)
        
        nv = []
        for i in range(1, len(v) - 1) :
            for j in range(1, len(v)) :
                nv.append(v[j][0:i] + v[j][i+1:len(v)])
            equation += pow(-1, i)*v[0][i]*getDet(nv)
            nv = []
            
        for i in range(1, len(v)) :
            nv.append(v[i][0:len(v)-1]) 
        equation += pow(-1, len(v)-1)*v[0][len(v)-1]*getDet(nv)
        
        return equation
    
def getEigen(v) :
    h = Symbol('h')
    
    vl = v.tolist()
    
    vt = copy.deepcopy(vl)
    
    for i in range(0, len(v)) :
        vt[i][i] = vt[i][i] - h
    
    sol = solve(getDet(vt))
    
    print "EigenValues"
    for i in sol :
         print "⊙", i
    print "\n"
    
    sym = []
    for i in range(0, len(sol)) :
        sym.append(Symbol(str(alpha[i])))
            
    sol3 = []
    for i in range(0, len(sol)) :
        vt = copy.deepcopy(vl)
        for j in range(0, len(v)) :
            vt[j][j] = vt[j][j] - sol[i]
        dot = np.dot(vt, sym)
        sol2 = solve(dot, sym)
        for j in range(0, len(sol)) :
            sol3.append(sol2[sym[j]] if (sym[j] in sol2) else alpha[j])
        print i+1, "th EigenVector = ", str(sol3).replace("'", "")
        sol3 = []
        print "\n"
        
getEigen(arr)