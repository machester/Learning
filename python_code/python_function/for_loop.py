# -*- coding: UTF-8 -*-
import sys

# for loop rang from 1 to 10, step is 1
for index in range(1, 10, 1):
	print('index = ', + index)
print('----------------------------------')

# for loop rang from 0 to 20, step is 5
for index in range(0, 20, 5):
	print('index step by 5 is: ', + index)
print('----------------------------------')

# for loop print 'end = ''' test
print('index = ', sep=' ', end=' ')
for index in range(0, 20, 5):
	print(index, end=', ')
print('\n')
print('----------------------------------')
