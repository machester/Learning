# -*- coding: UTF-8 -*-
# set function power, default 2
import sys

def power(x, n=2):
    s = 1
    while n > 0:
        n = n - 1
        s = s * x
    return s

value = power(5)
print(value);

value = power(5, 3)
print(value);
