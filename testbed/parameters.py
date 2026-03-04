import os
import sys
import subprocess

#host=[2,3] # total number of host
host = [1]

algorithm =['arbiter','exponential','bba','logistic','conventional','elastic'] ## adaptive algorithm

trace =[12, 24, 36, 48, 60, 72, 84, 96, 108, 120]

count = 1

for curr in range(count):
    for h in host:
        for a in algorithm:
            for c in trace:
                clear = 'sudo mn -c'
                run_top_script = 'sudo python topo.py '+ str(h)+ ' ' + str(a)+ ' ' + str(c)       
                subprocess.run(clear.split(' '))
                print(run_top_script)
                subprocess.run(run_top_script.split(' '))
