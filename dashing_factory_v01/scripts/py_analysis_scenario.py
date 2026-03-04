
from myutils import *


# ++++++++++++ #
#   SCENARIO   #
#    STUFF     #
# ++++++++++++ # -----------------------------------------------------
# distances.csv  gnb-positions.csv  recapStats.csv  ue-positions.csv
# -----------------------------------------------------------------------------------------------------------------------

# ==========================================================
# Recap files
# ==========================================================

recapScenarioColumns = ['start', 'duration', 'ul_meanThroughput', 'ul_meanDelay', 'ul_meanJitter', 'ul_meanLossRatio', 
                        'dl_meanThroughput','dl_meanDelay', 'dl_meanJitter', 'dl_meanLossRatio']
targetScenario = ['duration', 'ul_meanThroughput', 'ul_meanDelay', 'ul_meanJitter', 'ul_meanLossRatio', 
                'dl_meanThroughput','dl_meanDelay', 'dl_meanJitter', 'dl_meanLossRatio']

megaRecapDf = pd.DataFrame (columns=recapScenarioColumns)                          
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 

for current_dir in meta_directories : 
    print ("enter ", current_dir)
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))] 
    recapDf = pd.DataFrame (columns=recapScenarioColumns)                              
    for directory in directories : 
        print ("    enter ", directory, end="")        
        recapFiles  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'recapStats*\.csv', f)]
        if (len(recapFiles)) :
            print ("---> reading recap file done ")
            littleDf = pd.read_csv(current_dir + "/"+ directory + "/recapStats.csv",  names=recapScenarioColumns,  skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            recapDf = recapDf.append(littleDf) 
        else :
            print ("---> no recap file inside ", directory)
        
    megaRecapDf = megaRecapDf.append(recapDf)  

# # clean the stuff : delete units from the corresponding column (sec, KB, kbps) 
# megaRecapDf[metrics_in_s]    = megaRecapDf[metrics_in_s].apply(lambda x: x.apply(lambda y: float(y[:-1])), axis=1)
# megaRecapDf[metrics_in_KB]   = megaRecapDf[metrics_in_KB].apply(lambda x: x.apply(lambda y: float(y[:-2])), axis=1)
# megaRecapDf[metrics_in_kbps] = megaRecapDf[metrics_in_kbps].apply(lambda x: x.apply(lambda y: float(y[:-4])), axis=1)   

reduce_memory_usage (megaRecapDf)
megaRecapDf.reset_index().to_feather("megaRecapDf.feather")
# ===================================================================


# -------------------------------
#   Groupby : per scenario
# -------------------------------
megaRecapDf = pd.read_feather("megaRecapDf.feather") 
megaRecapDf['duration_min'] = megaRecapDf.duration//60
targetz = ['duration_min']

groupedDf = megaRecapDf.groupby(["scenario"])[targetz].mean().reset_index()
groupedDf['scenario_id'] = groupedDf['scenario'].apply(lambda x: int(x[:-5]))
groupedDf = groupedDf.sort_values(by=['scenario_id'])
groupedDf = groupedDf.drop('scenario_id', axis=1)

reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("groupedDf_scenarioDf.feather")
groupedDf_scenarioDf = pd.read_feather("groupedDf_scenarioDf.feather") 
# --------------------------------------------------------------------



# -------------------------------
#   raw daset version
# -------------------------------
megaRecapDf = pd.read_feather("megaRecapDf.feather") 
megaRecapDf['duration_min'] = megaRecapDf.duration//60
targetz = ['duration_min']

groupedDf = megaRecapDf.groupby(["scenario", "tag"])[targetz].mean().reset_index()
groupedDf['scenario_id'] = groupedDf['scenario'].apply(lambda x: int(x[:-5]))
groupedDf = groupedDf.sort_values(by=['scenario_id'])
groupedDf = groupedDf.drop('scenario_id', axis=1)

reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("rawDf_scenarioDf.feather")
rawDf_scenarioDf = pd.read_feather("rawDf_scenarioDf.feather") 
# --------------------------------------------------------------------



# ---------------------------
def convert_min2h (minutes) :
    hours = " " if (minutes//60)==0 else str(int(minutes//60))+"h"
    min_r = " " if (minutes % 60)==0 else str(int(minutes % 60))+"m"
    return hours+min_r
# -----------------------------------------------------------


# ====================================
# About Duration of each experiment : 
# boxPlot over different scenarios 
# ===========================================
plt.clf()
fig = plt.figure(dpi=600)
megaRecapDf["scenario_label"] = megaRecapDf["scenario"].apply(lambda x: int(x[:-5])) 
megaRecapDf = megaRecapDf.sort_values(by=["scenario_label"])
scenarioz = list(megaRecapDf.scenario.unique())
index = 0        
y_values = []
for scenarioId in scenarioz :      
    index+=1                  
    scenarioValues = megaRecapDf[megaRecapDf.scenario==scenarioId]["duration_min"]
    y_values.append(scenarioValues)
    ymedian = scenarioValues.median()
    plt.text(index-0.15, ymedian+20, convert_min2h(ymedian), color="red", fontsize=8)         
plt.boxplot(y_values)
plt.xticks(list(range(len(scenarioz)+1))[1:], list(scenarioz), fontsize=8, rotation=30)
plt.ylabel("Duration (in min)", fontsize=8)
plt.title("Duration of Experiments", fontsize=14, color="red")
plt.savefig("plotboxVersion_duration.jpg")
# plt.show()  # -------------------------------
# ------------








# ==========================================================
#  Distances Files
# ==========================================================

uePositionsColumns   = ["ue", "nodeId",  "imsi", "uex",  "uey", "uez"]
gnbPositionsColumns  = ["gnb", "gnbId", "gnbx", "gnby", "gnbz"]         
ueDistancesColumns   = ["nodeId", "distance", "gnbId"]         
megaDistancesColumns = ['ue', 'nodeId', 'uex', 'uey', 'uez', 'distance', 'gnbId', 'gnb', 'gnbx', 'gnby', 'gnbz', 'tag', 'scenario']

megaDistanceDf = pd.DataFrame (columns=megaDistancesColumns)
for current_dir in meta_directories : 
    print ("enter ", current_dir)
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))] 
    recapDf = pd.DataFrame (columns=recapScenarioColumns)                              
          
    distanceDf = pd.DataFrame (columns=megaDistancesColumns)
    for directory in directories :                            
        uePositionsDf   = pd.read_csv(current_dir + "/"+ directory + "/ue-positions.csv",  sep="\t", names=uePositionsColumns, index_col=False)
        gnbPositionsDf  = pd.read_csv(current_dir + "/"+ directory + "/gnb-positions.csv", sep="\t", names=gnbPositionsColumns,index_col=False)
        ueDistancesDf   = pd.read_csv(current_dir + "/"+ directory + "/distances.csv",  sep="\t", names=ueDistancesColumns, index_col=False)
        littleDf = (uePositionsDf.merge(ueDistancesDf, on="nodeId")).merge (gnbPositionsDf, on="gnbId")
        littleDf ["tag"] = directory
        littleDf ["scenario"] = current_dir
        distanceDf = distanceDf.append (littleDf)
        
        plt.clf()   
        fig = plt.figure(figsize=(8, 6))     
        plt.title("Nodes Distribution (tag="+directory+")", fontsize=12, color="r")
        gnbz =  ["gnb"+str(index) for index in gnbPositionsDf.gnb]
        plt.scatter(gnbPositionsDf.gnbx+10, gnbPositionsDf.gnby+15, marker = "1", c="blue", s=80)
        # uez =  ["ue"+str(index) for index in uePositionsDf.ue]
        # plt.scatter(uePositionsDf.uex+10, uePositionsDf.uey+15, c="red", s=20, label=uePositionsDf.ue)
        plt.scatter(uePositionsDf.uex+10, uePositionsDf.uey+15, marker="+", c="red", s=20)

        imsiz =  uePositionsDf.imsi.values.tolist()
        for imsi in uePositionsDf.imsi :
            plt.text(x=uePositionsDf[uePositionsDf.imsi==imsi].uex+10, 
                     y=uePositionsDf[uePositionsDf.imsi==imsi].uey+15.5, 
                     s="ue"+str(imsi), color='r', fontsize=8)

        for i in range (0, 120, 20) :
            plt.axline((i, 0), (i, 50), linestyle=":", linewidth=0.5, color='b')      

        plt.axline((0, 25), (120, 25), linestyle=":", linewidth=0.5, color='b')      
        plt.xlim (0, 120); plt.ylim (0, 50)
        plt.savefig(current_dir+"/"+directory+"_topology.png")
        # ---------------------------------------------------



    # -----------------------------------------------
    # Here we draw the same for each scenario apart
    # e.g. 12nodes, 24nodes, etc
    # --------------------------------------------------------------------------    
    plt.clf()        
    plt.title("Nodes Distribution (scenario with "+ current_dir[:2]+" nodes)", fontsize = 14, color='r')
    gnbz =  ["gnb"+str(index) for index in distanceDf.gnb]
    plt.scatter(distanceDf.gnbx+10, distanceDf.gnby+15, marker = "1", c="blue", s=80, label=gnbz)
    uez =  ["ue"+str(index) for index in distanceDf.ue]
    plt.scatter(distanceDf.uex+10, distanceDf.uey+15, marker="+", c="red", s=20, label=uez)
    for i in range (0, 120, 20) :
        plt.axline((i, 0), (i, 50), linestyle=":", linewidth=0.5, color='b')      
    plt.axline((0, 25), (120, 25), linestyle=":", linewidth=0.5, color='b')      
    plt.xlim (0, 120); plt.ylim (0, 50)
    plt.savefig(current_dir+"/allinone_topology_"+current_dir[:2]+"nodes.png")  
    # ------------------------------------------------------------------------


    # ----------------------------------------------------
    megaDistanceDf = megaDistanceDf.append (distanceDf)
    # ----------------------------------------------------



# -----------------------------------------------------
# Here we draw the same for all the previous scenarios
# --------------------------------------------------------------------------
plt.clf()        
plt.title("Nodes Distribution Over All Scenarios", fontsize = 14, color='r')
gnbz =  ["gnb"+str(index) for index in megaDistanceDf.gnb]
plt.scatter(megaDistanceDf.gnbx+10, megaDistanceDf.gnby+15, marker = "1", c="blue", s=80, label=gnbz)
uez =  ["ue"+str(index) for index in megaDistanceDf.ue]
plt.scatter(megaDistanceDf.uex+10, megaDistanceDf.uey+15, marker="+", c="red", s=20, label=uez)
for i in range (0, 120, 20) :
    plt.axline((i, 0), (i, 50), linestyle=":", linewidth=0.5, color='b')      
plt.axline((0, 25), (120, 25), linestyle=":", linewidth=0.5, color='b')      
plt.xlim (0, 120); plt.ylim (0, 50)
plt.savefig("allinone_topology.png")  
# -----------------------------------



# ----------------------------------    
reduce_memory_usage (megaDistanceDf)
megaDistanceDf.reset_index().to_feather("megaDistanceDf.feather")
# ---------------------------------------------------------------


