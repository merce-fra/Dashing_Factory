#!/bin/bash

# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#  Performance evaluation framework for dash video streaming
#  over a factory deployement

#  This is a bash script automating the process of running 
#  Merce-Dash-NS3 
#
# ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


declare -r WHITE='\E[37m'
declare -r GRAY='\E[38m'
declare -r BLUE='\E[34m'
declare -r CYAN='\E[36m'
declare -r YELLOW='\E[33m'
declare -r GREEN='\E[32m'
declare -r RED='\E[31m'
declare -r MAGENTA='\E[35m'




# -------------------         
# Parameters in input
# ---------------------------------------
  declare -i users="${1}"   # UE per gNb
  declare -i simTime="${2}"   # UE per gNb
  declare -i randomSeed="${3}"    # random seed
  declare    inf_scenario="${4}"   # inF scenario  
  declare    simTag="${1}nodes"   # tag used to distinct simulations  
# -------------------------------------------------------------------


echo "# --------------------------------------"
echo "# STEP (01) | Inside ns-3 root dir"
echo "# --------------------------------------"

echo "# 8. Run the ns3 simulation"
# declare ns3_dir=/home/user/disk/workspace/ns/ns-3-test/ns-3-dev
declare ns3_dir=.
cd ${ns3_dir}
    # clear
    declare startTime=$(date)
    declare output_dir=/outputs/${inf_scenario}/${randomSeed}/
    declare complete_path=${ns3_dir}${output_dir}
    echo ${complete_path}    
    if [[ ! -e $complete_path ]]; then mkdir -p ${complete_path}; fi  
    echo " -------------------------------------------"
    echo " Simulation started at : $startTime"
    echo " "    
    echo " > Parameters :  output dir : ${output_dir}"
    echo "                 users  : ${users}"
    echo "                 duration : ${simTime}"
    echo "                 inF scenario : ${inf_scenario}"
    echo "                 seed : ${randomSeed}"
    #./ns3 run "dashing-factory    --users=${users} --simulationTime=${simTime}\
    ./waf --run "dashing-factory     --users=${users} --simulationTime=${simTime}\
                                    --outputDir=${complete_path}\
                                    --inFScenario=${inf_scenario}\
                                    --seed=${randomSeed}" 
    # --gdb
    # --command-template="gdb %s"
    # --users=24 --simulationTime=100 \
    # --outputDir=/home/user/disk/workspace/ns/ns-3-dev/outputs/24nodes/4321/ --seed=4321"                                  
    declare endTime=$(date)
    echo " "    
    echo " Simulation ended at : $endTime"    
    echo " -------------------------------------------"

    
    ## mv TxedGnbMacCtrlMsgsTrace.txt ${complete_path}
    ## mv RxedGnbMacCtrlMsgsTrace.txt TxedGnbPhyCtrlMsgsTrace.txt RxedGnbPhyCtrlMsgsTrace.txt ${complete_path}
    ## mv TxedUeMacCtrlMsgsTrace.txt RxedUeMacCtrlMsgsTrace.txt   TxedUePhyCtrlMsgsTrace.txt ${complete_path}
    ## mv RxPacketTrace.txt RxedUePhyCtrlMsgsTrace.txt   RxedUePhyDlDciTrace.txt ${complete_path}
    ## mv NrDlPdcpTxStats.txt NrUlPdcpTxStats.txt ${complete_path}
    ## mv DlCtrlSinr.txt        NrDlRlcStatsE2E.txt   NrUlRlcRxStats.txt ${complete_path}
    ## mv DlDataSinr.txt        NrDlRxRlcStats.txt    NrUlRlcStatsE2E.txt ${complete_path}
    ## mv DlPathlossTrace.txt   NrDlTxRlcStats.txt    NrUlRlcTxStats.txt ${complete_path}
    ## mv NrDlMacStats.txt      NrUlMacStats.txt      UlPathlossTrace.txt ${complete_path}
    ## mv NrDlPdcpRxStats.txt   NrUlPdcpRxStats.txt ${complete_path}
    ## mv NrDlPdcpStatsE2E.txt  NrUlPdcpStatsE2E.txt ${complete_path}
    ## mv DlPdcpStats.txt  UlPdcpStats.txt   ${complete_path}
    ## mv DlRlcStats.txt  UlRlcStats.txt   ${complete_path}
    ## mv SinrTrace.txt    ${complete_path}

    # ue-positions-ao-scInF-SparseLow-sp3-gm gnb-positions-ao-scInF-SparseLow-sp3-gm 
    # distances-ao-scInF-SparseLow-sp3-gm
    # Coupling-ao-scInF-SparseLow-sp3-gm sinrs-ao-scInF-SparseLow-sp3-gm 
    # rssi-ao-scInF-SparseLow-sp3-gm 
    ## mv *-gm ${complete_path}

    ## mv dashStatsByNode_* ${complete_path}
    ## mv dashSegmentStatsByNode_* ${complete_path}
    ## mv tcp_*_trace.csv ${complete_path}
    ## mv tcp_*_window.csv tcp_slow_start_threshold.csv ${complete_path}

    mv *$randomSeed.txt ${complete_path}
    mv *$randomSeed.csv ${complete_path}
