from myutils import *


# ===================
def suffix_col_ip (col):
    if ((col in dl_mbps) | (col in ul_mbps)) : return " (in Mbps)"
    if ((col in dl_ms) | (col in ul_ms)) : return " (in ms)"    
    return "" # ====================================
# =============




# +++++++ #
#    IP   #
#  STUFF  #
# +++++++ # ---------------------------------------------------------------------------------------------------
dlIpColumns = ["flowId","sourceNode","sourceNodePosition","sourceAddress","sourcePort","destAddress","destPort",
               "protocol","dl_txPackets","dl_txBytes","dl_txOffered","dl_xBytes","dl_rxOfferedthroughput","dl_meanDelay","dl_meanJitter",
               "dl_rxPackets","dl_lostPackets","dl_lostPacketsRatio"]

ulIpColumns = ["flowId","sourceNode","sourceNodePosition","sourceAddress","sourcePort","destAddress","destPort",
               "protocol","ul_txPackets","ul_txBytes","ul_txOffered","ul_xBytes","ul_rxOfferedthroughput","ul_meanDelay","ul_meanJitter",
               "ul_rxPackets","ul_lostPackets","ul_lostPacketsRatio"]
dl_mbps = ["dl_txOffered", "dl_rxOfferedthroughput"] 
dl_ms =   ["dl_meanDelay", "dl_meanJitter"]
ul_mbps = ["ul_txOffered", "ul_rxOfferedthroughput"] 
ul_ms =   ["ul_meanDelay", "ul_meanJitter"]
# ------------------------------------------



# ---#
# DL #
# -------------------------------------------
megaIpDlDf = pd.DataFrame (columns=dlIpColumns)      
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))]     
    for directory in directories : 
        files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'dlFlows.csv', f)]                            
        if (len(files)) :
            littleDf = pd.read_csv(current_dir +"/"+ directory +"/dlFlows.csv", names=dlIpColumns, skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            megaIpDlDf = megaIpDlDf.append(littleDf) 
        else :
            print ("error somewhere, no dlFlows.csv file inside ", current_dir + "/"+ directory)
# -----------------------------------------------------------------------------------------------



#-----------------------------------------------------------------------------
# clean the stuff : delete units from the corresponding column (sec, KB, kbps) 
megaIpDlDf[dl_mbps]    = megaIpDlDf[dl_mbps].apply(lambda x: x.apply(lambda y: float(y[:-4])), axis=1)
megaIpDlDf[dl_ms]   = megaIpDlDf[dl_ms].apply(lambda x: x.apply(lambda y: float(y[:-2])), axis=1) 
# reduce la taille and save 3lia lkissamoh
reduce_memory_usage (megaIpDlDf)
megaIpDlDf.reset_index().to_feather("megaIpDlDf.feather")
#--------------------------------------------------------



# ---------------------------------------------------
#        groupby "scenario", "segmentId" and "nodes"
#   i.e. nodes are all merged (averaged)
# ---------------------------------------------------
# ['flowId', 'sourceNode', 'tag', 'scenario',
target_dl = ["dl_txPackets","dl_txBytes","dl_txOffered","dl_xBytes","dl_rxOfferedthroughput","dl_meanDelay","dl_meanJitter",
             "dl_rxPackets","dl_lostPackets","dl_lostPacketsRatio"]            

megaIpDlDf = pd.read_feather("megaIpDlDf.feather")
reduce_memory_usage (megaIpDlDf)
# aggregate cols per node
dl_groupedDf = megaIpDlDf.groupby(["scenario", "tag"])[target_dl].mean()
dl_groupedDf = dl_groupedDf.reset_index()
reduce_memory_usage (dl_groupedDf)
# -----------------------------


# ------------------------------------------
# Draw the mean, Max and Min values together
# in multiple plots
# ------------------
parameterz = target_dl
scenarioz = list(dl_groupedDf.scenario.unique())
scenarioz = scenarioz[2:] + scenarioz[:2]
for parameter in parameterz: 
    plt.clf()
    fig = plt.figure(dpi=600, figsize=(6, 5))
    index = 0        
    y_values = []
    for scenarioId in scenarioz :  
        index+=1                  
        scenarioValues = dl_groupedDf[dl_groupedDf.scenario==scenarioId][parameter]
        y_values.append(scenarioValues)
        ymax = scenarioValues.median()
        plt.text(index-0.3, ymax*1.05, str(ymax), color="red", fontsize=8)         

    plt.boxplot(y_values)
    plt.xticks(list(range(len(scenarioz)+1))[1:], list(scenarioz), rotation=20, fontsize=6)
    plt.ylabel(parameter+suffix_col_ip(parameter))
    plt.xlabel("Scenario (nb nodes)")
    plt.title(parameter)
    plt.savefig("ip_dl_"+parameter+"_plotboxVersion.jpg")
# -------------------------------------------------------



# -------------------------------
#   Another form of (dl) groupby :  
# -------------------------------
megaIpDlDf = pd.read_feather("megaIpDlDf.feather")
targetz = ['dl_txPackets', 'dl_txBytes', 'dl_txOffered', 'dl_xBytes', 'dl_rxOfferedthroughput', 'dl_meanDelay', 'dl_meanJitter', 
           'dl_rxPackets', 'dl_lostPackets', 'dl_lostPacketsRatio']
groupedDf = megaIpDlDf.groupby(["scenario","destAddress"])[targetz].mean().reset_index()
groupedDf["node"] = megaIpDlDf.destAddress.apply(lambda x: int(x[6:])-1)
groupedDf = groupedDf.drop('destAddress', axis=1)
groupedDf.reset_index().to_feather("groupedDf_ipDlDf.feather")
groupedDf_ipDlDf = pd.read_feather("groupedDf_ipDlDf.feather") 
# --------------------------------------------------------------------



# -------------------------------
#   raw daset version  
# -------------------------------
megaIpDlDf = pd.read_feather("megaIpDlDf.feather")
targetz = ['dl_txPackets', 'dl_txBytes', 'dl_txOffered', 'dl_xBytes', 'dl_rxOfferedthroughput', 'dl_meanDelay', 'dl_meanJitter', 
           'dl_rxPackets', 'dl_lostPackets', 'dl_lostPacketsRatio']
groupedDf = megaIpDlDf.groupby(["scenario","tag", "destAddress"])[targetz].mean().reset_index()
groupedDf["node"] = megaIpDlDf.destAddress.apply(lambda x: int(x[6:])-1)
groupedDf = groupedDf.drop('destAddress', axis=1)
groupedDf.reset_index().to_feather("rawDf_ipDlDf.feather")
rawDf_ipDlDf = pd.read_feather("rawDf_ipDlDf.feather") 
# --------------------------------------------------------------------



# -------
#   UL
# -------

# -------------------------------------------
megaIpUlDf = pd.DataFrame (columns=ulIpColumns)      
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))]     
    for directory in directories : 
        files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'ulFlows.csv', f)]                            
        if (len(files)) :
            littleDf = pd.read_csv(current_dir +"/"+ directory +"/ulFlows.csv", names=ulIpColumns, skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            megaIpUlDf = megaIpUlDf.append(littleDf) 
        else :
            print ("error somewhere, no ulFlows.csv file inside ", current_dir + "/"+ directory)
# -----------------------------------------------------------------------------------------------



#-----------------------------------------------------------------------------
# clean the stuff : delete units from the corresponding column (sec, KB, kbps) 
megaIpUlDf[ul_mbps]    = megaIpUlDf[ul_mbps].apply(lambda x: x.apply(lambda y: float(y[:-4])), axis=1)
megaIpUlDf[ul_ms]   = megaIpUlDf[ul_ms].apply(lambda x: x.apply(lambda y: float(y[:-2])), axis=1) 
# reduce la taille and save 3lia lkissamoh
reduce_memory_usage (megaIpUlDf)
megaIpUlDf.reset_index().to_feather("megaIpUlDf.feather")
#--------------------------------------------------------



# ---------------------------------------------------
#        groupby "scenario", "segmentId" and "nodes"
#   i.e. nodes are all merged (averaged)
# ---------------------------------------------------
# ['flowId', 'sourceNode', 'tag', 'scenario',
target_ul = ["ul_txPackets","ul_txBytes","ul_txOffered","ul_xBytes","ul_rxOfferedthroughput","ul_meanDelay","ul_meanJitter",
             "ul_rxPackets","ul_lostPackets","ul_lostPacketsRatio"]            

megaIpUlDf = pd.read_feather("megaIpUlDf.feather")
reduce_memory_usage (megaIpUlDf)
# aggregate cols per node
ul_groupedDf = megaIpUlDf.groupby(["scenario", "tag"])[target_ul].mean()
ul_groupedDf = ul_groupedDf.reset_index()
reduce_memory_usage (ul_groupedDf)
# -----------------------------


# ------------------------------------------
# Draw the mean, Max and Min values together
# in multiple plots
# ------------------
parameterz = target_ul
scenarioz = list(ul_groupedDf.scenario.unique())
scenarioz = scenarioz[2:] + scenarioz[:2]
for parameter in parameterz: 
    plt.clf()
    fig = plt.figure(dpi=600, figsize=(6, 5))
    index = 0        
    y_values = []
    for scenarioId in scenarioz :  
        index+=1                  
        scenarioValues = ul_groupedDf[ul_groupedDf.scenario==scenarioId][parameter]
        y_values.append(scenarioValues)
        ymax = scenarioValues.median()
        plt.text(index-0.3, ymax*1.08, str(ymax), color="red", fontsize=8)         

    plt.boxplot(y_values)
    plt.xticks(list(range(len(scenarioz)+1))[1:], list(scenarioz), rotation=20, fontsize=6)
    plt.ylabel(parameter+suffix_col_ip(parameter))
    plt.xlabel("Scenario (nb nodes)")
    plt.title(parameter)
    plt.savefig("ip_"+parameter+"_plotboxVersion.jpg")
# -------------------------------------------------------



# -------------------------------
#   Another form of (ul) groupby :  
# -------------------------------
megaIpUlDf = pd.read_feather("megaIpUlDf.feather")
targetz = ['ul_txPackets', 'ul_txBytes', 'ul_txOffered', 'ul_xBytes', 'ul_rxOfferedthroughput', 'ul_meanDelay', 'ul_meanJitter', 
           'ul_rxPackets', 'ul_lostPackets', 'ul_lostPacketsRatio']
groupedDf = megaIpUlDf.groupby(["scenario","sourceNode"])[targetz].mean().reset_index()
groupedDf["node"] = groupedDf["sourceNode"].apply(lambda x: int(x[2:])+1)
groupedDf = groupedDf.drop('sourceNode', axis=1)
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("groupedDf_ipUlDf.feather")
groupedDf_ipUlDf = pd.read_feather("groupedDf_ipUlDf.feather") 
# --------------------------------------------------------------------




# -------------------------------
#   Raw version dataset
# -------------------------------
megaIpUlDf = pd.read_feather("megaIpUlDf.feather")
targetz = ['ul_txPackets', 'ul_txBytes', 'ul_txOffered', 'ul_xBytes', 'ul_rxOfferedthroughput', 'ul_meanDelay', 'ul_meanJitter', 
           'ul_rxPackets', 'ul_lostPackets', 'ul_lostPacketsRatio']
groupedDf = megaIpUlDf.groupby(["scenario","tag", "sourceNode"])[targetz].mean().reset_index()
groupedDf["node"] = groupedDf["sourceNode"].apply(lambda x: int(x[2:])+1)
groupedDf = groupedDf.drop('sourceNode', axis=1)
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("rawDf_ipUlDf.feather")
rawDf_ipUlDf = pd.read_feather("rawDf_ipUlDf.feather") 
# --------------------------------------------------------------------
