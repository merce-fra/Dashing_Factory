from myutils import *
from pandas.plotting._matplotlib.style import get_standard_colors
from typing import List, Union


# ===================
def suffix_col_ip (col):
    if ((col in dl_mbps) | (col in ul_mbps)) : return " (in Mbps)"
    if ((col in dl_ms) | (col in ul_ms)) : return " (in ms)"    
    return "" # ====================================
# =============



# ------------------------------------------
# Draw the mean, Max and Min values together
# in multiple plots
# ------------------
def draw_plotbox_for (groupedDf, targetz, 
                      label="", title="", 
                      text_median=True, show=False) :
    """
    draw_plotbox_for (groupedDf, targetz, "mac_nr", title = "MAC/NR Traces",
                  text_median=False, show=False) 
    """                      
    parameterz = targetz
    scenarioz = list(groupedDf.scenario.unique())
    scenarioz = scenarioz[2:] + scenarioz[:2]
    for parameter in parameterz: 
        plt.clf()
        fig = plt.figure(dpi=600, figsize=(6, 5))
        index = 0        
        y_values = []
        for scenarioId in scenarioz :  
            index+=1                  
            scenarioValues = groupedDf[groupedDf.scenario==scenarioId][parameter]
            y_values.append(scenarioValues)
            if (text_median):
                ymax = scenarioValues.median()
                plt.text(index-0.3, ymax*1.05, str(ymax), color="red", fontsize=8)         
    
        plt.boxplot(y_values)
        plt.xticks(list(range(len(scenarioz)+1))[1:], list(scenarioz), rotation=20, fontsize=6)
        plt.ylabel(parameter+suffix_col_ip(parameter))
        plt.xlabel("Scenario (nb nodes)")
        if (title) :  plt.title(title+"\n ("+ parameter+")", fontsize=14)
        else : plt.title(parameter, fontsize=14)
        plt.savefig(label+"_"+parameter+"_plotboxVersion.jpg")
        if show : plt.show()
# -------------------------------------------------------



# ---------------------------
# Collect stats for one metric
#---------------------------------------------------
def collect_mac_msg_for (file, columnz, targetz) :
    colz = ["scenario","tag", "nodeId"] + targetz
    metaDf = pd.DataFrame (columns = colz); 
    meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
    for current_dir in meta_directories :         
        directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))] 
        for directory in directories :               
            files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r""+file, f)]                            
            if (len(files)) :             
                littleDf = pd.DataFrame ()
                df = pd.read_csv(current_dir + "/"+ directory + "/" + file, sep="\t", names=columnz, skiprows=1, index_col=False)                  
                littleDf["nodeId"]  = df.nodeId.unique()
                for target in targetz :
                    target_count = df[(df.MsgType == target)].groupby("nodeId")["MsgType"].count().sum()
                    if target_count : targetDf = df[(df.MsgType == target)].groupby("nodeId")["MsgType"].count().reset_index(name=target)
                    else : targetDf = pd.DataFrame ({ "nodeId" : df.nodeId.unique(),
                                                       target : 0 })
                    littleDf = pd.merge(littleDf, targetDf, on="nodeId")
                littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir 
                metaDf = metaDf.append(littleDf)
                if (metaDf.isna().sum().sum() > 0):
                    sys.exit()
            else : print ("        NOT OK --> len(files) = ", len(files))                                                          
    return metaDf   # ---------------------------------------------
# ---------------------------






#-------------------------------------------------------
# (1)  Get RLC Stats Dl : Files : "NrDlRlcStatsE2E.txt"
#-------------------------------------------------------------------------------------------------------------
dl_rlcColumns = ["time",	"end_s", "cellId", "IMSI", "RNTI", "LCID"	, "dl_rlc_nTxPDUs"	, "dl_rlc_TxBytes"	, 
                 "dl_rlc_nRxPDUs", "dl_rlc_RxBytes", "dl_rlc_delay_s", "dl_rlc_stdDev_s", "dl_rlc_min_s", "dl_rlc_max_s", "dl_rlc_PduSize",	
                 "dl_rlc_stdDev",	"dl_rlc_min",	"dl_rlc_max"]

megaDlRlcDf = pd.DataFrame (columns=dl_rlcColumns)      
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))]     
    for directory in directories : 
        files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'NrDlRlcStatsE2E.txt', f)]                            
        if (len(files)) :
            littleDf = pd.read_csv(current_dir +"/"+ directory +"/NrDlRlcStatsE2E.txt", names=dl_rlcColumns, sep="\t", skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            megaDlRlcDf = megaDlRlcDf.append(littleDf) 
        else :
            print ("error somewhere, no dlFlows.csv file inside ", current_dir + "/"+ directory)

reduce_memory_usage (megaDlRlcDf)
megaDlRlcDf.reset_index().to_feather("megaDlRlcDf.feather")

# -------------------
#   groupby dataset  
# -------------------
megaDlRlcDf = pd.read_feather("megaDlRlcDf.feather")
targetz       = ["dl_rlc_nTxPDUs"	, "dl_rlc_TxBytes", "dl_rlc_nRxPDUs", "dl_rlc_RxBytes", "dl_rlc_delay_s", 
                 "dl_rlc_stdDev_s", "dl_rlc_min_s", "dl_rlc_max_s", "dl_rlc_PduSize", "dl_rlc_stdDev",	"dl_rlc_min",	"dl_rlc_max"]                   
groupedDf = megaDlRlcDf.groupby(["scenario","IMSI"])[targetz].mean().reset_index()
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("groupedDf_dlRlcDf.feather")
# --------------------------------------------------------------------

# --------------------------------------------------------------------
draw_plotbox_for (groupedDf, targetz, "rlc_dl", title = "RLC Traces", text_median=False, show=False) 
# ------------------------------------------------

# -------------------
#   raw dataset  
# -------------------
targetz       = ["dl_rlc_nTxPDUs"	, "dl_rlc_TxBytes", "dl_rlc_nRxPDUs", "dl_rlc_RxBytes", "dl_rlc_delay_s", 
                 "dl_rlc_stdDev_s", "dl_rlc_min_s", "dl_rlc_max_s", "dl_rlc_PduSize", "dl_rlc_stdDev",	"dl_rlc_min",	"dl_rlc_max"]
megaDlRlcDf = pd.read_feather("megaDlRlcDf.feather")
groupedDf = megaDlRlcDf.groupby(["scenario", "tag", "IMSI"])[targetz].mean().reset_index()
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("rawDf_dlRlcDf.feather")
# --------------------------------------------------------------------







#-------------------------------------------------------
# (2)  Get RLC Stats Ul : Files : "NrUlRlcStatsE2E.txt"
#-------------------------------------------------------------------------------------------------------------
ul_rlcColumns = ["time",	"end_s", "cellId", "IMSI", "RNTI", "LCID"	, "ul_rlc_nTxPDUs"	, "ul_rlc_TxBytes"	, 
                 "ul_rlc_nRxPDUs", "ul_rlc_RxBytes", "ul_rlc_delay_s", "ul_rlc_stdDev_s", "ul_rlc_min_s", "ul_rlc_max_s", "ul_rlc_PduSize",	
                 "ul_rlc_stdDev",	"ul_rlc_min",	"ul_rlc_max"]

megaUlRlcDf = pd.DataFrame (columns=ul_rlcColumns)      
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))]     
    for directory in directories : 
        files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'NrUlRlcStatsE2E.txt', f)]                            
        if (len(files)) :
            littleDf = pd.read_csv(current_dir +"/"+ directory +"/NrUlRlcStatsE2E.txt", names=ul_rlcColumns, sep="\t", skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            megaUlRlcDf = megaUlRlcDf.append(littleDf) 
        else :
            print ("error somewhere, no ulFlows.csv file inside ", current_dir + "/"+ directory)

reduce_memory_usage (megaUlRlcDf)
megaUlRlcDf.reset_index().to_feather("megaUlRlcDf.feather")


# -------------
#   groupby :  
# -------------
megaUlRlcDf = pd.read_feather("megaUlRlcDf.feather")
targetz       = ["ul_rlc_nTxPDUs"	, "ul_rlc_TxBytes", "ul_rlc_nRxPDUs", "ul_rlc_RxBytes", "ul_rlc_delay_s", 
                 "ul_rlc_stdDev_s", "ul_rlc_min_s", "ul_rlc_max_s", "ul_rlc_PduSize", "ul_rlc_stdDev",	"ul_rlc_min",	"ul_rlc_max"]                   
groupedDf = megaUlRlcDf.groupby(["scenario","IMSI"])[targetz].mean().reset_index()

reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("groupedDf_ulRlcDf.feather")
# --------------------------------------------------------------------

# -----------------------------------------------------------------------------------------------------
draw_plotbox_for (groupedDf, targetz, "rlc_ul", title = "RLC Traces", text_median=False, show=False) 
# -------------------------------------------------------------------------------------------------


# -------------------
#   raw dataset  
# -------------------
megaUlRlcDf = pd.read_feather("megaUlRlcDf.feather")
targetz       = ["ul_rlc_nTxPDUs"	, "ul_rlc_TxBytes", "ul_rlc_nRxPDUs", "ul_rlc_RxBytes", "ul_rlc_delay_s", 
                 "ul_rlc_stdDev_s", "ul_rlc_min_s", "ul_rlc_max_s", "ul_rlc_PduSize", "ul_rlc_stdDev",	"ul_rlc_min",	"ul_rlc_max"]                   
groupedDf = megaUlRlcDf.groupby(["scenario","tag", "IMSI"])[targetz].mean().reset_index()

reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("rawDf_ulRlcDf.feather")
# -------------------------------------------------------------------------------------------------