# -*- coding: UTF-8 -*-
import os
import random

fp = open("D:\\random_data.txt", "w+")
fp.write("-------------- start: random data -------------\n")
for index in range(0, 40):
    data_fp = random.uniform(35.5, 36.7)
    data = round(data_fp, 1)
    print(data)
    fp.write("%2.2f\n"% data)

fp.write("-------------- end: random data -------------\n")
fp.close()