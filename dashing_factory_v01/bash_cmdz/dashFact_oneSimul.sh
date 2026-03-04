#!/bin/bash
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=1

#  "${i}node" ${gNbNum}  ${i} ${xCoord} ${yCoord} ${random_seed} 

# declare simTag="${1}"           # simulation tag
# declare -i gNbNum="${2}"        # Number gNbs
# declare -i ueNumPergNb="${3}"   # UE per gNb
# declare -i uePos="${4}"   # random seed
# declare -i randomSeed="${5}"   # random seed
declare -i ueNum="${1}"   # nb of UEs in simulation
declare -i simTime="${2}" # simulation time
declare -i randomSeed="${3}"   # random seed


echo "streamFact_startSimul.sh "${simTag} ${gNbNum} ${ueNumPergNb} ${randomSeed} 
## srun bash_cmdz/streamFact_startSimul.sh  ${simTag} ${gNbNum} ${ueNumPergNb} ${uePos} ${randomSeed} 
## bash_cmdz/dashFact_startSimul.sh  12  5  9 (example)
bash_cmdz/dashFact_startSimul.sh    ${ueNum} ${simTime} ${randomSeed} 

