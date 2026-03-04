#!/bin/bash

declare -i maxNodes="${1}"        # max of UE nodes
declare -i maxRandom="${2}"       # number of experiment to repeat
                                  # to take randomness into account
declare -i simTime="${3}"


for ((i=${maxNodes};i<${maxNodes}+1;i+=1)); do
    for ((indexRand=0;indexRand<${maxRandom};indexRand+=1)); do  
        let random_seed="`echo $[ $[ RANDOM % 10000 ]]`"            
        for inFScenario in InF-SparseLow  InF-SparseHigh  InF-DenseHigh  InF-DenseLow; do 
             # Parameters to pass to dashFact_oneSimul.sh :
             #     ueNum="${1}"
             #     simTime="${2}"
             #     randomSeed="${3}"        
                 echo "bash_cmdz/dashFact_oneSimul.sh " ${i}  ${simTime}  ${random_seed} ${inFScenario}
                 # sbatch --job-name="${i}n-${random_seed}" \
                 bash_cmdz/dashFact_oneSimul_v2.sh ${i}  ${simTime}  ${random_seed} ${inFScenario}
        done
    done
done 



