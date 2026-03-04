from myutils import *


# ------------------------------------------
# Draw the mean, Max and Min values together
# in multiple plots
# ------------------
def draw_simple_for (groupedDf, targetz, mean=True, max=False, median=False,
                     label="", title="", ymaxCoeff=1.05, xmaxCoeff=0.3, 
                     text_label=True, show=False) :
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

        if (mean) :     y_values = groupedDf.groupby(["scenario"])[parameter].mean() 
        elif (median) : y_values = groupedDf.groupby(["scenario"])[parameter].median() 
        else :          y_values = groupedDf.groupby(["scenario"])[parameter].max() 
                  
        plt.scatter (scenarioz, y_values, color = "red")
        plt.plot (scenarioz, y_values)
        if (text_label) : 
            for index, value in enumerate(y_values) : 
                value = round (value, 2)
                plt.text(index-xmaxCoeff, value*ymaxCoeff, str(value), color="red", fontsize=8)          
        plt.xticks(list(range(len(scenarioz)))[:], list(scenarioz), rotation=20, fontsize=6)
        plt.ylabel(parameter+suffix_col_ip(parameter))
        plt.xlabel("Scenario (nb nodes)")
        if (title) :  plt.title(title+"\n ("+ parameter+")", fontsize=14)
        else : plt.title(parameter, fontsize=14)
        plt.savefig(label+"_"+parameter+"_simplePlotVersion.jpg")
        if show : plt.show()
# -------------------------------------------------------



# ----------------------------------------------------
# rxPacket Trace : Get  tbSize, mcs, rv, SINR_dB, CQI 
# ----------------------------------------------------
# In "UL" --> TBler is always giving "nan" TBler_UL_mean/TBler_UL_median/TBler_UL_var ??
# --------------------------------------------------------------------------------------


#-------------------
#  rxPacket stats
#------------------------------------------------------------------------------------------------------
rxPacketColumns = ["Time" ,"direction" ,"frame" ,"subF" ,"slot" ,"1stSym" ,"nSymbol" ,"cellId" ,
                   "bwpId" ,"streamId" ,"rnti" ,"tbSize" ,"mcs","rv" ,"SINR_dB" ,"CQI" ,"corrupt" ,"TBler"]
megaRxPacketDf = pd.DataFrame (columns=rxPacketColumns)      
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))]     
    for directory in directories : 
        files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'RxPacketTrace.txt', f)]                            
        if (len(files)) :
            littleDf = pd.read_csv(current_dir +"/"+ directory +"/RxPacketTrace.txt", names=rxPacketColumns, sep="\t", skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            megaRxPacketDf = megaRxPacketDf.append(littleDf) 
        else :
            print ("error somewhere, no UlPathlossTrace.txt file inside ", current_dir + "/"+ directory)
reduce_memory_usage (megaRxPacketDf)
# ----------------------------------


#-------------------------
#  (1) UL rxPacket stats
#------------------------------------------------------------------------------------------------------
ulRxPacketColumns = ["Time" ,"direction" ,"frame" ,"subF" ,"slot" ,"1stSym" ,"nSymbol" ,"cellId" ,
                   "bwpId" ,"streamId" ,"rnti" ,"ul_tbSize" ,"ul_mcs","ul_rv" ,"ul_SINR_dB" ,"ul_CQI" ,"ul_corrupt" ,"ul_TBler"]
dict = {x:y for x,y in zip(rxPacketColumns[11:], ulRxPacketColumns[11:])} 
megaRxPacketUlDf = megaRxPacketDf[megaRxPacketDf.direction == "UL"] 
megaRxPacketUlDf = megaRxPacketUlDf.rename(dict, axis="columns") 
# megaRxPacketUlDf.reset_index().to_feather("megaRxPacketUlDf.feather")   

# --------------------------
#  for the groupby dataset
# -------------------------- 
ul_targetz  = ["ul_tbSize" ,"ul_mcs","ul_rv" ,"ul_SINR_dB" ,"ul_CQI" ,"ul_corrupt"] # ul_Tbler always in nan no se prqui
ul_groupedDf = megaRxPacketUlDf.groupby(["scenario","cellId", "rnti"])[ul_targetz[0]].mean().reset_index()
for metric in ul_targetz[1:] :
    ul_groupedDf[metric] = megaRxPacketUlDf.groupby(["scenario","cellId", "rnti"])[metric].mean().reset_index()[metric]
ul_groupedDf["ul_rv_max"] = megaRxPacketUlDf.groupby(["scenario","cellId", "rnti"])["ul_rv"].max().reset_index()["ul_rv"]

ul_targetz_round_3 = ["ul_rv" ,"ul_rv_max", "ul_corrupt"] 
ul_targetz_round_2 = ["ul_tbSize" ,"ul_mcs" ,"ul_SINR_dB" ,"ul_CQI" ] 

ul_groupedDf[ul_targetz_round_3] = ul_groupedDf[ul_targetz_round_3].apply(lambda x: round(x, 3))
ul_groupedDf[ul_targetz_round_2] = ul_groupedDf[ul_targetz_round_2].apply(lambda x: round(x, 2))

draw_plotbox_for_groupbyV (ul_groupedDf, ul_targetz, "rxPacket_ul", title =  "Phy Traces", text_median=True, show=False) 
draw_plotbox_for_groupbyV ( ul_groupedDf, ["ul_mcs"], "rxPacket_ul", title =  "Phy Traces", text_median=True, show=False, ymaxCoeff=1.01)
draw_plotbox_for_groupbyV (ul_groupedDf, ["ul_SINR_dB"], "rxPacket_ul", title ="Phy Traces", ymaxCoeff=1.03, xmaxCoeff=0.19)                            
# ----------------------------------------------------------------------
#   For RAW dataset version   
# --------------------------
ul_groupedDf = megaRxPacketUlDf.groupby(["scenario","tag", "cellId", "rnti"])[ul_targetz[0]].mean().reset_index()
for metric in ul_targetz[1:] :
    ul_groupedDf[metric] = megaRxPacketUlDf.groupby(["scenario","tag", "cellId", "rnti"])[metric].mean().reset_index()[metric]
ul_groupedDf["ul_rv_max"] = megaRxPacketUlDf.groupby(["scenario","tag", "cellId", "rnti"])["ul_rv"].max().reset_index()["ul_rv"]

ul_groupedDf[ul_targetz_round_3] = ul_groupedDf[ul_targetz_round_3].apply(lambda x: round(x, 3))
ul_groupedDf[ul_targetz_round_2] = ul_groupedDf[ul_targetz_round_2].apply(lambda x: round(x, 2))

reduce_memory_usage (ul_groupedDf)
ul_groupedDf.reset_index().to_feather("rawDf_ulRxPacketDf.feather")
rawDf_ulRxPacketDf = pd.read_feather("rawDf_ulRxPacketDf.feather")
# ----------------------------------------------------------------------

#-------------------------
#  (2) DL rxPacket stats
#------------------------------------------------------------------------------------------------------
dlRxPacketColumns = ["Time" ,"direction" ,"frame" ,"subF" ,"slot" ,"1stSym" ,"nSymbol" ,"cellId" ,
                   "bwpId" ,"streamId" ,"rnti" ,"dl_tbSize" ,"dl_mcs","dl_rv" ,"dl_SINR_dB" ,"dl_CQI" ,"dl_corrupt" ,"dl_TBler"]
dict = {x:y for x,y in zip(rxPacketColumns[11:], dlRxPacketColumns[11:])}                    
megaRxPacketDlDf = megaRxPacketDf[ megaRxPacketDf.direction == "DL"]
megaRxPacketDlDf = megaRxPacketDlDf.rename(dict, axis="columns")                                                              
megaRxPacketDlDf.reset_index().to_feather("megaRxPacketDlDf.feather")  
megaRxPacketDlDf = pd.read_feather("megaRxPacketDlDf.feather")
# --------------------------
#  for the groupby dataset
# -------------------------- 
dl_targetz         = ["dl_tbSize" ,"dl_mcs","dl_rv" ,"dl_SINR_dB" ,"dl_CQI" ,"dl_corrupt"] # ul_Tbler always in nan no se prqui
dl_groupedDf = megaRxPacketDlDf.groupby(["scenario","cellId", "rnti"])[dl_targetz[0]].mean().reset_index()
for metric in dl_targetz[1:] :
    dl_groupedDf[metric] = megaRxPacketDlDf.groupby(["scenario","cellId", "rnti"])[metric].mean().reset_index()[metric]

dl_groupedDf.reset_index().to_feather("dl_groupedDf.feather")
dl_groupedDf = pd.read_feather("dl_groupedDf.feather")

dl_targetz_round_3 = ["dl_rv" ,"dl_corrupt"] 
dl_targetz_round_2 = ["dl_tbSize" ,"dl_mcs" ,"dl_SINR_dB" ,"dl_CQI" ] 
dl_groupedDf[dl_targetz_round_3] = dl_groupedDf[dl_targetz_round_3].apply(lambda x: round(x, 3))
dl_groupedDf[dl_targetz_round_2] = dl_groupedDf[dl_targetz_round_2].apply(lambda x: round(x, 2))

reduce_memory_usage (dl_groupedDf)
dl_groupedDf.reset_index().to_feather("groupedDf_dlRxPacketDf.feather")

draw_plotbox_for_groupbyV (dl_groupedDf, dl_targetz, "rxPacket_dl", title =  "Phy Traces", text_median=True, show=False) 
draw_plotbox_for_groupbyV ( dl_groupedDf, ["dl_mcs"], "rxPacket_dl", title =  "Phy Traces", text_median=True, show=False, ymaxCoeff=1.01)
draw_plotbox_for_groupbyV ( dl_groupedDf, ["dl_CQI"], "rxPacket_dl", title =  "Phy Traces", text_median=True, show=False, ymaxCoeff=1.001)                            
# -------------------------------------------------------------------------------------------------------------
#   For RAW dataset version   
# --------------------------
dl_groupedDf = megaRxPacketDlDf.groupby(["scenario","tag", "cellId", "rnti"])[dl_targetz[0]].mean().reset_index()
for metric in dl_targetz[1:] :
    dl_groupedDf[metric] = megaRxPacketDlDf.groupby(["scenario","tag", "cellId", "rnti"])[metric].mean().reset_index()[metric]

dl_targetz_round_3 = ["dl_rv" ,"dl_corrupt"] 
dl_targetz_round_2 = ["dl_tbSize" ,"dl_mcs" ,"dl_SINR_dB" ,"dl_CQI" ] 
dl_groupedDf[dl_targetz_round_3] = dl_groupedDf[dl_targetz_round_3].apply(lambda x: round(x, 3))
dl_groupedDf[dl_targetz_round_2] = dl_groupedDf[dl_targetz_round_2].apply(lambda x: round(x, 2))

reduce_memory_usage (dl_groupedDf)
dl_groupedDf.reset_index().to_feather("rawDf_dlRxPacketDf.feather")
rawDf_dlRxPacketDf = pd.read_feather("rawDf_dlRxPacketDf.feather")
# ----------------------------------------------------------------






































#-------------------
# (1)  UL pathloss
#------------------------------------------------------------------------------------------------------
ulPathlossColumns = ["Time", "CellId", "BwpId", "txStreamId",	"IMSI",	"rxStreamId", "ul_pathLoss_dB"]   

megaUlPathLossDf = pd.DataFrame (columns=ulPathlossColumns)      
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))]     
    for directory in directories : 
        files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'UlPathlossTrace.txt', f)]                            
        if (len(files)) :
            littleDf = pd.read_csv(current_dir +"/"+ directory +"/UlPathlossTrace.txt", names=ulPathlossColumns, sep="\t", skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            megaUlPathLossDf = megaUlPathLossDf.append(littleDf) 
        else :
            print ("error somewhere, no UlPathlossTrace.txt file inside ", current_dir + "/"+ directory)

reduce_memory_usage (megaUlPathLossDf)
megaUlPathLossDf.reset_index().to_feather("megaUlPathLossDf.feather")

# --------------------------
#  for the groupby dataset
# --------------------------
megaUlPathLossDf = pd.read_feather("megaUlPathLossDf.feather")
targetz         = ["ul_pathLoss_dB"]                     
groupedDf = megaUlPathLossDf.groupby(["scenario","IMSI"])[targetz].mean().reset_index()
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("groupedDf_ulPathLossDf.feather")
draw_plotbox_for_groupbyV (groupedDf, targetz, "ppathloss_ul", title ="Phy Traces", ymaxCoeff=.997, xmaxCoeff=0.25)
# ------------------------------------------------
#   For RAW dataset version   
# --------------------------
megaUlPathLossDf = pd.read_feather("megaUlPathLossDf.feather")
targetz         = ["ul_pathLoss_dB"]                     
groupedDf = megaUlPathLossDf.groupby(["scenario","tag", "IMSI"])[targetz].mean().reset_index()
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("rawDf_ulPathLossDf.feather")
rawDf_ulPathLossDf = pd.read_feather("rawDf_ulPathLossDf.feather")
# ------------------------------------------------


#----------------------
# (2)  DL pathloss
#-------------------------------------------------------------------------------------------------------------
dlPathlossColumns = ["Time", "CellId", "BwpId", "txStreamId",	"IMSI",	"rxStreamId", "dl_pathLoss_dB"]   

megaDlPathLossDf = pd.DataFrame (columns=dlPathlossColumns)      
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))]     
    for directory in directories : 
        files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'DlPathlossTrace.txt', f)]                            
        if (len(files)) :
            littleDf = pd.read_csv(current_dir +"/"+ directory +"/DlPathlossTrace.txt", names=dlPathlossColumns, sep="\t", skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            megaDlPathLossDf = megaDlPathLossDf.append(littleDf) 
        else :
            print ("error somewhere, no DlPathlossTrace.txt file inside ", current_dir + "/"+ directory)

reduce_memory_usage (megaDlPathLossDf)
megaDlPathLossDf.reset_index().to_feather("megaDlPathLossDf.feather")

# --------------------------
#  for the groupby dataset
# --------------------------
megaDlPathLossDf = pd.read_feather("megaDlPathLossDf.feather")
targetz         = ["dl_pathLoss_dB"]                     
groupedDf = megaDlPathLossDf.groupby(["scenario","IMSI"])[targetz].mean().reset_index()
groupedDf.reset_index().to_feather("groupedDf_dlPathLossDf.feather")

draw_plotbox_for_groupbyV (groupedDf, targetz, "ppathloss_dl", title ="Phy Traces", ymaxCoeff=.997, xmaxCoeff=0.25)
# ------------------------------------------------
#   For RAW dataset version   
# --------------------------
megaDlPathLossDf = pd.read_feather("megaDlPathLossDf.feather")
targetz         = ["dl_pathLoss_dB"]                     
groupedDf = megaDlPathLossDf.groupby(["scenario", "tag", "IMSI"])[targetz].mean().reset_index()
groupedDf.reset_index().to_feather("rawDf_dlPathLossDf.feather")
rawDf_dlPathLossDf = pd.read_feather("rawDf_dlPathLossDf.feather")
# ------------------------------------------------

# ************************************************







#-------------------------------------------------------
# (1)  Get SINR Stats Dl : Files : "sinrs.csv"
#-------------------------------------------------------------------------------------------------------------
sinrColumns = ["timestamp", "cellId", "rnti", "sinr"]               

megaSinrDf = pd.DataFrame (columns=sinrColumns)      
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))]     
    for directory in directories : 
        files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'sinrs.csv', f)]                            
        if (len(files)) :
            littleDf = pd.read_csv(current_dir +"/"+ directory +"/sinrs.csv", names=sinrColumns, sep="\t", skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            megaSinrDf = megaSinrDf.append(littleDf) 
        else :
            print ("error somewhere, no sinrs.csv file inside ", current_dir + "/"+ directory)

reduce_memory_usage (megaSinrDf)

### megaSinrDf.reset_index().to_feather("megaSinrDf.feather")

# --------------------
#   groupby version   
# --------------------
### megaSinrDf = pd.read_feather("megaSinrDf.feather")
targetz     = ["sinr"]                 
groupedDf = megaSinrDf.groupby(["scenario","cellId", "rnti",])[targetz].mean().reset_index()
groupedDf.reset_index().to_feather("groupedDf_sinrDf.feather")

draw_plotbox_for_groupbyV (groupedDf, ["sinr"], "sinr_phy_v1", title ="Phy Traces", ymaxCoeff=1.03, xmaxCoeff=0.19)
# ------------------------------------------------



# ---------------------------
#   For RAW dataset version   
# ---------------------------
targetz     = ["sinr"]                 
groupedDf = megaSinrDf.groupby(["scenario", "tag", "cellId", "rnti",])[targetz].mean().reset_index()
groupedDf.reset_index().to_feather("rawDf_sinrDf.feather")
rawDf_sinrDf = pd.read_feather("rawDf_sinrDf.feather")
# ------------------------------------------------