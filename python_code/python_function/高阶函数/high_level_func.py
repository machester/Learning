# -*- coding: UTF-8 -*-
import sys

f = abs
print("high leve function test, function transfer to parameter")
print(f(-10))

# set function default f value is abs
def add(x, y, f = abs):
    return f(x) + f(y)
    
print("add(x, y, f) = ", add(-10, 10, f))
