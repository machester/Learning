# -*- coding: utf-8 -*-

import  os
# create a dict / key-value / map
usr_dict = {'Michael': 95, 'Bob': 75, 'Tracy': 85}
# print map dict
print("Michael:", usr_dict["Michael"], ", Bob:", usr_dict["Bob"], ", Tracy:", usr_dict["Tracy"])

# change dict value
usr_dict["Bob"] = 60
print("Michael:", usr_dict["Michael"], ", Bob:", usr_dict["Bob"], ", Tracy:", usr_dict["Tracy"])

# Answer ---------------------------------------------------------------
# Michael: 95 , Bob: 75 , Tracy: 85
# Michael: 95 , Bob: 60 , Tracy: 85
# Answer ---------------------------------------------------------------


usr_dict.pop("Tracy")
