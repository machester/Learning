
#-------- for time function ----------------
import sys
import time

tm = time.time()
format_time = time.ctime(tm)
for index in range(1,5):
	print("current time is: " + format_time)
sys.exit()
#============================================

