from myutils import *
from pandas.plotting._matplotlib.style import get_standard_colors
from typing import List, Union







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
# (1)  Get PDCP Stats Dl : Files : "NrDlPdcpStatsE2E.txt"
#-------------------------------------------------------------------------------------------------------------
dl_pdcpColumns = ["time",	"end_s", "cellId", "IMSI", "RNTI", "LCID"	, "dl_pdcp_nTxPDUs"	, "dl_pdcp_TxBytes"	, 
                 "dl_pdcp_nRxPDUs", "dl_pdcp_RxBytes", "dl_pdcp_delay_s", "dl_pdcp_stdDev_s", "dl_pdcp_min_s", "dl_pdcp_max_s", "dl_pdcp_PduSize",	
                 "dl_pdcp_stdDev",	"dl_pdcp_min",	"dl_pdcp_max"]              

megaDlPdcpDf = pd.DataFrame (columns=dl_pdcpColumns)      
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))]     
    for directory in directories : 
        files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'NrDlPdcpStatsE2E.txt', f)]                            
        if (len(files)) :
            littleDf = pd.read_csv(current_dir +"/"+ directory +"/NrDlPdcpStatsE2E.txt", names=dl_pdcpColumns, sep="\t", skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            megaDlPdcpDf = megaDlPdcpDf.append(littleDf) 
        else :
            print ("error somewhere, no dlFlows.csv file inside ", current_dir + "/"+ directory)

reduce_memory_usage (megaDlPdcpDf)
megaDlPdcpDf.reset_index().to_feather("megaDlPdcpDf.feather")
# ------------------------------------------------------------

# -------------
#   groupby :  
# -------------
megaDlPdcpDf = pd.read_feather("megaDlPdcpDf.feather")
targetz  = ["dl_pdcp_nTxPDUs"	, "dl_pdcp_TxBytes", "dl_pdcp_nRxPDUs", "dl_pdcp_RxBytes", "dl_pdcp_delay_s", 
                 "dl_pdcp_stdDev_s", "dl_pdcp_min_s", "dl_pdcp_max_s", "dl_pdcp_PduSize", "dl_pdcp_stdDev",	"dl_pdcp_min",	"dl_pdcp_max"]                   
groupedDf = megaDlPdcpDf.groupby(["scenario","IMSI"])[targetz].mean().reset_index()

reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("groupedDf_dlPdcpDf.feather")
# --------------------------------------------------------------------

# --------------------------------------------------------------------
draw_plotbox_for (groupedDf, targetz, "pdcp_dl", title =  "PDCP Traces", text_median=False, show=False) 
# ------------------------------------------------


# -------------------------
#   raw dataset version
# ----------------------------------------------------
megaDlPdcpDf = pd.read_feather("megaDlPdcpDf.feather")
targetz  = ["dl_pdcp_nTxPDUs"	, "dl_pdcp_TxBytes", "dl_pdcp_nRxPDUs", "dl_pdcp_RxBytes", "dl_pdcp_delay_s", 
                 "dl_pdcp_stdDev_s", "dl_pdcp_min_s", "dl_pdcp_max_s", "dl_pdcp_PduSize", "dl_pdcp_stdDev",	"dl_pdcp_min",	"dl_pdcp_max"]                   
groupedDf = megaDlPdcpDf.groupby(["scenario","tag", "IMSI"])[targetz].mean().reset_index()

reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("rawDf_dlPdcpDf.feather")
# --------------------------------------------------------------------







#-------------------------------------------------------
# (2)  Get PDCP Stats Ul : Files : "NrUlPdcpStatsE2E.txt"
#-------------------------------------------------------------------------------------------------------------
ul_pdcpColumns = ["time",	"end_s", "cellId", "IMSI", "RNTI", "LCID"	, "ul_pdcp_nTxPDUs"	, "ul_pdcp_TxBytes"	, 
                 "ul_pdcp_nRxPDUs", "ul_pdcp_RxBytes", "ul_pdcp_delay_s", "ul_pdcp_stdDev_s", "ul_pdcp_min_s", "ul_pdcp_max_s", "ul_pdcp_PduSize",	
                 "ul_pdcp_stdDev",	"ul_pdcp_min",	"ul_pdcp_max"]

megaUlPdcpDf = pd.DataFrame (columns=ul_pdcpColumns)      
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))]     
    for directory in directories : 
        files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'NrUlPdcpStatsE2E.txt', f)]                            
        if (len(files)) :
            littleDf = pd.read_csv(current_dir +"/"+ directory +"/NrUlPdcpStatsE2E.txt", names=ul_pdcpColumns, sep="\t", skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            megaUlPdcpDf = megaUlPdcpDf.append(littleDf) 
        else :
            print ("error somewhere, no ulFlows.csv file inside ", current_dir + "/"+ directory)

reduce_memory_usage (megaUlPdcpDf)
megaUlPdcpDf.reset_index().to_feather("megaUlPdcpDf.feather")
# -----------------------------------------------------------


# -------------
#   groupby :  
# -------------
megaUlPdcpDf = pd.read_feather("megaUlPdcpDf.feather")
targetz = ["ul_pdcp_nTxPDUs", "ul_pdcp_TxBytes", "ul_pdcp_nRxPDUs", "ul_pdcp_RxBytes", "ul_pdcp_delay_s", 
           "ul_pdcp_stdDev_s", "ul_pdcp_min_s", "ul_pdcp_max_s", "ul_pdcp_PduSize", "ul_pdcp_stdDev",	"ul_pdcp_min",	"ul_pdcp_max"]                   
groupedDf = megaUlPdcpDf.groupby(["scenario","IMSI"])[targetz].mean().reset_index()
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("groupedDf_ulPdcpDf.feather")
# ----------------------------------------------------------------

# -----------------------------------------------------------------------
draw_plotbox_for (groupedDf, targetz, "pdcp_ul", title =  "PDCP Traces", text_median=False, show=False) 
# ------------------------------------------------------------------------------------------------------


# -------------
#   raw dataset version :  
# -----------------------------------------------------
megaUlPdcpDf = pd.read_feather("megaUlPdcpDf.feather")
targetz = ["ul_pdcp_nTxPDUs", "ul_pdcp_TxBytes", "ul_pdcp_nRxPDUs", "ul_pdcp_RxBytes", "ul_pdcp_delay_s", 
           "ul_pdcp_stdDev_s", "ul_pdcp_min_s", "ul_pdcp_max_s", "ul_pdcp_PduSize", "ul_pdcp_stdDev",	"ul_pdcp_min",	"ul_pdcp_max"]                   
groupedDf = megaUlPdcpDf.groupby(["scenario","tag", "IMSI"])[targetz].mean().reset_index()
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("rawDf_ulPdcpDf.feather")
# ----------------------------------------------------------------







    # -----------------------
    # Get PDCP Stats Ul/Dl : 
    # ----------------------------------------------
    #                       Focus just on e2e delay
    #             cellId :  CellId of the attached Enb
    #               imsi :  IMSI of the UE who received the PDU
    #               rnti :  C-RNTI of the UE who received the PDU
    #               lcid :  LCID through which the PDU has been received
    #            pckSize :  size of the PDU in bytes
    #              delay :  PDCP to PDCP delay in nanoseconds 
    #                       (statistics average, min, max and standard deviation in second)               
    #                        nr-bearer-stats-calculator.cc line 330
    # --------------------------------------------------------------