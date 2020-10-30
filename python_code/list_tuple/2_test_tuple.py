# -*- coding: utf-8 -*-

# tuple cannot change, list can change. make a list on tuple
TEST_TUPLE = (
    ['Apple', 'Google', 'Microsoft'],
    ['Java', 'Python', 'Ruby', 'PHP'],
    ['Adam', 'Bart', 'Lisa']
)
print(TEST_TUPLE[0][0])
print(TEST_TUPLE[0][1])
print("-------------------------------------\n")
print(TEST_TUPLE[1][0])
print(TEST_TUPLE[1][1])
print("-------------------------------------\n")
print(TEST_TUPLE[2][0])
print(TEST_TUPLE[2][1])

#** Answer --------------------------------------------------------------------------
# Apple
# Google
# -------------------------------------
# Java
# Python
# -------------------------------------
# Adam
# Bart
#** Answer --------------------------------------------------------------------------
