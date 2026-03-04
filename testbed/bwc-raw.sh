#!/bin/bash

###          ul_txOffered(MB/s)   ul_txOffered_max(MB/s)   ul_meanDelay(ms)  ul_meanJitter(ms)
### scenario                                                            
### 12nodes      29.671875               31.609375            18.28125         0.132446
### 24nodes      29.437500               31.906250            20.12500         0.160400
### 36nodes      24.906250               30.156250            23.84375         0.217651
### 48nodes      21.171875               25.250000            26.84375         0.280273
### 60nodes      17.875000               23.531250            30.37500         0.327148
### 72nodes      15.757812               18.500000            33.53125         0.393066
### 84nodes      13.804688               19.375000            37.06250         0.445557
### 96nodes      12.328125               14.679688            40.90625         0.499512
### 108nodes     11.125000               13.992188            43.96875         0.554688
### 120nodes        


### 12nodes    29672.875    31609.375    18.28125    0.132446
### 24nodes    29437.500    31906.250    20.12500    0.160400
### 36nodes    24906.250    30156.250    23.84375    0.217651
### 48nodes    21171.875    25250.000    26.84375    0.280273
### 60nodes    17875.000    23531.250    30.37500    0.327148
### 72nodes    15757.812    18500.000    33.53125    0.393066
### 84nodes    13804.688    19375.000    37.06250    0.445557
### 96nodes    12328.125    14679.688    40.90625    0.499512
### 108nodes    11125.000    13992.188    43.96875    0.554688
### 120nodes    10101.562    14445.312    47.12500    0.615723




num=$1
a=1
v=1
echo "We start bwcRAW ..."    
echo "(a=$a, v=$v, num=$num, i=$i, n=$n)"    
while [ $a -lt 2 ]; do
     echo "    Inside while a<=2 ..."    
     echo "    reminder --> (a=$a, v=$v, num=$num, i=$i, n=$n)"    
     rates_array=()
     ceils_array=()
     delays_array=()
     jitters_array=()
     i=1
     cut -d, -f 1 --output-delimiter=' ' /home/dash/Demo/testbed/5g_traces/$num.csv |\
          while read rate ceil delay jitter ; do    
               echo "        Inside while read rate ... etc"    
               echo "        reminder --> (a=$a, v=$v, num=$num, i=$i, n=$n)"    
               echo "        sudo tc disk ... etc"    
               rates_array[$i]=$rate
               ceils_array[$i]=$ceil
               delays_array[$i]=$delay
               jitters_array[$i]=$jitter
               # duplicate  --> tc qdisc add dev s2-eth1 parent 1:1 handle 10: netem delay 100ms 50ms duplicate 3% 10%// 30%
               # reordoring --> tc qdisc add dev s2-eth1 parent 1:1 handle 10: netem delay 100ms 50ms reorder 10% 
               # corruption --> tc qdisc add dev s2-eth1 parent 1:1 handle 10: netem delay 100ms 50ms corrupt 0.5%
               sudo  tc qdisc add dev s2-eth1 handle 1:0 root htb default 1 &&\
                     tc class add dev s2-eth1 parent 1:0 classid 1:1 htb rate "${rates_array[i]}"kbit ceil "${ceils_array[i]}"kbit &&\
                     tc qdisc add dev s2-eth1 parent 1:1 handle 10: netem delay "${delays_array[i]}ms" "${jitters_array[i]}ms corrupt 60%" &&\
                     
                     echo  "second" $SECONDS  ": rate ->" "${rates_array[i]}"kbit "ceil -> " "${ceils_array[i]}"kbit
                     echo  "           delay ->" "${delays_array[i]}"ms "jitter -> " "${jitters_array[i]}ms"
               echo "        done ... sleep 121"
               sleep 121 
               v=0          
               i=$((i + 1))
               #if [ $SECONDS -ge 300 ]; then
                    #break
               #fi
               n=$(ps -ef| grep  goDASH| wc -l)  
               echo "        if n==1??"    
               echo "        reminder now --> (a=$a, v=$v, num=$num, i=$i, n=$n)"                   
               if [ $n -eq 1 ]; then
                         echo "        Yes it is --> sleep1 --> kill caddy/tcpdump --> break"
                         sleep 1
                         sudo chmod 777 -R /home/dash/Demo/Data/Raw
                         echo "caddy server off....."
                         sudo pkill -9 caddy                         
                         echo "stop pcap capturing....."
                         sudo pkill -9 tcpdum
                         a=$((a + 1))
                         echo "        reminder now --> (a=$a, v=$v, num=$num, i=$i, n=$n)"                   
                         break
               fi
          done
     echo "    a=$a is it eq 2 ????"    
     echo "    reminder now --> (a=$a, v=$v, num=$num, i=$i, n=$n)"                        
     if [ $a -eq "2" ]; then
          echo "        yes a=2 --> break"         
          break
     else
          echo "        no, a!=2 --> "              
          echo "        reminder now --> (a=$a, v=$v, num=$num, i=$i, n=$n)"                                  
          echo "        n==1??"              
          n=$(ps -ef| grep  goDASH| wc -l)
          if [ $n -eq 1 ]; then
               echo "            Yes it is --> sleep1 --> kill caddy/tcpdump --> break"          
               sleep 1
               sudo chmod 777 -R /home/dash/Demo/Data/Raw
               #echo "caddy server of....."
               #echo "stop pcap capturing....."
               sudo pkill -9 tcpdum
               sudo pkill -9 caddy
               a=$((a + 1))
               break
          fi
     fi  
done
