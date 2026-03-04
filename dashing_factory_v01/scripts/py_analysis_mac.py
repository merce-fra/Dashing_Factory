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




# ---------------------------------------------------------------
# Plot multiple Y axes on the same chart with same x axis.
# Args:
#     data: dataframe which contains x and y columns
#     x: column to use as x axis. If None, use index.
#     y: list of columns to use as Y axes. If None, all columns are used
#         except x column.
#     spacing: spacing between the plots
#     **kwargs: keyword arguments to pass to data.plot()
# Returns:
#     a matplotlib.axes.Axes object returned from data.plot()
# Example:
# >>> plot_multi(df, figsize=(22, 10))
# >>> plot_multi(df, x='time', figsize=(22, 10))
# >>> plot_multi(df, y='price qty value'.split(), figsize=(22, 10))
# >>> plot_multi(df, x='time', y='price qty value'.split(), figsize=(22, 10))
# >>> plot_multi(df[['time price qty'.split()]], x='time', figsize=(22, 10))
#     code is mentioned in https://stackoverflow.com/q/11640243/2593810
# ----------------------------------------------------
def plot_multi( data: pd.DataFrame, x: Union[str, None] = None,
                y: Union[List[str], None] = None, spacing: float = 0.1,
                title = "", **kwargs ) :
                # -> matplotlib.axes.Axes:

    # Get default color style from pandas - can be changed to any other color list
    if y is None: y = data.columns

    # remove x_col from y_cols
    if x: y = [col for col in y if col != x]

    if len(y) == 0: return
    colors = get_standard_colors(num_colors=len(y))

    if "legend" not in kwargs: kwargs["legend"] = False  # prevent multiple legends

    # First axis
    ax = data.plot(x=x, y=y[0], color=colors[0], **kwargs)
    ax.set_ylabel(ylabel=y[0]); 
    ax.set_xlabel("Scenario (nb nodes)", fontsize=12); 
    ax.set_title(title, fontsize=15); 
    ax.tick_params(axis='y', colors=colors[0])
    ax.yaxis.label.set_color(colors[0])    
    ax.spines["right"].set_edgecolor(colors[0])    
    lines, labels = ax.get_legend_handles_labels()

    for i in range(1, len(y)):
        # Multiple y-axes
        ax_new = ax.twinx()
        ax_new.spines["right"].set_position(("axes", 1 + spacing * (i - 1)))
        data.plot(ax=ax_new, x=x, y=y[i], color=colors[i % len(colors)], **kwargs)
        ax_new.set_ylabel(ylabel=y[i])
        ax_new.tick_params(axis='y', colors=colors[i % len(colors)])
        ax_new.yaxis.label.set_color(colors[i % len(colors)])
        ax_new.spines["right"].set_edgecolor(colors[i % len(colors)])

        # Proper legend position
        line, label = ax_new.get_legend_handles_labels()
        lines += line; labels += label

    ax.legend(lines, labels, loc=0)
    return ax  # -----------------
# --------------------            



# ---------------------------
# Collect stats for one metric
#---------------------------------------------------
def collect_mac_msg_for (file, columnz, targetz) :
    colz = ["scenario","tag", "cellId"] + targetz
    metaDf = pd.DataFrame (columns = colz); 
    meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
    for current_dir in meta_directories :         
        directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))] 
        for directory in directories :               
            files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r""+file, f)]                            
            if (len(files)) :             
                littleDf = pd.DataFrame ()
                # df = pd.read_csv(current_dir + "/"+ directory + "/" + file, sep="\t", names=columnz, skiprows=1, index_col=False)                  
                df = pd.read_csv(current_dir + "/"+ directory + "/" + file, delimiter=r"\s+", names=columnz, skiprows=1, index_col=False)
                littleDf["cellId"]  = df.cellId.unique()
                littleDf["rnti"]  = df.RNTI.unique()
                for target in targetz :
                    target_count = df[(df.MsgType == target)].groupby(["cellId"])["MsgType"].count().sum()
                    if target_count : targetDf = df[(df.MsgType == target)].groupby("cellId")["MsgType"].count().reset_index(name=target)
                    else : targetDf = pd.DataFrame ({ "cellId" : df.cellId.unique(), target : 0 })
                    littleDf = pd.merge(littleDf, targetDf, on="cellId")
                littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir 
                metaDf = metaDf.append(littleDf)
                if (metaDf.isna().sum().sum() > 0):
                    sys.exit()
            else : print ("        NOT OK --> len(files) = ", len(files))                                                          
    return metaDf   # ---------------------------------------------
# ---------------------------



# ---------------------------
# Collect stats for one metric
#---------------------------------------------------
def collect_mac_msg_for_v2 (file, columnz, targetz) :
    colz = ["scenario","tag", "cellId", "RNTI"] + targetz
    metaDf = pd.DataFrame (columns = colz); 
    meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
    for current_dir in meta_directories :         
        directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))] 
        for directory in directories :               
            files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r""+file, f)]                            
            if (len(files)) :             
                littleDf = pd.DataFrame ()
                df = pd.read_csv(current_dir + "/"+ directory + "/" + file, delimiter=r"\s+", names=columnz, skiprows=1, index_col=False)
                littleDf[["cellId", "RNTI"]] = df.groupby(["cellId", "RNTI"]).size().reset_index()[["cellId", "RNTI"]]
                for target in targetz :
                    targetDf = pd.DataFrame ()
                    target_count = df[(df.MsgType == target)].groupby(["cellId", "RNTI"])["MsgType"].count().sum()
                    if target_count : targetDf = df[(df.MsgType == target)].groupby(["cellId", "RNTI"])["MsgType"].count().reset_index(name=target)
                    else :
                        targetDf = df.groupby(["cellId", "RNTI"]).size().reset_index()[["cellId", "RNTI"]]
                        targetDf[target] = 0
                    littleDf = littleDf.merge(targetDf, how="left", on=["cellId", "RNTI"])
                    littleDf = littleDf.fillna(0)
                littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir 
                metaDf = metaDf.append(littleDf)
                if (metaDf.isna().sum().sum() > 0):
                    sys.exit()
            else : print ("        NOT OK --> len(files) = ", len(files))                                                          
    return metaDf   # ---------------------------------------------
# ---------------------------


#--------------------------------------------------------------------------------------------
# (1) Files such as  : "rxedGnbMacDf", "rxedUeMacDf", "rxedPhyGnbDf", "rxedPhyUeDf", 
#                      "txedGnbMacDf", "txedUeMacDf", "txedPhyGnbDf", "txedPhyUeDf"]
#--------------------------------------------------------------------------------------------
# rxedColumns = ["Time", "Entity", "Frame", "SF", "Slot", "nodeId", "RNTI", "bwpId", "MsgType"] 
rxedColumns = ["Time",    "Entity",  "Layer", "TxMx", "Frame",   "SF",      "Slot",     "cellId",  "RNTI",    "bwpId",   "MsgType"]
targetz     = ["UL_DCI", "DL_DCI", "DL_CQI", "MIB", "SIB1", "RACH_PREAMBLE", "RAR", "BSR", "DL_HARQ", "SR", "SRS", "UL_UCI"]


rxedGnbMacDf = collect_mac_msg_for_v2 ("RxedGnbMacCtrlMsgsTrace.txt", rxedColumns, targetz)
rxedUeMacDf = collect_mac_msg_for_v2 ("RxedUeMacCtrlMsgsTrace.txt", rxedColumns, targetz)
rxedPhyGnbDf = collect_mac_msg_for_v2 ("RxedGnbPhyCtrlMsgsTrace.txt", rxedColumns, targetz)
rxedPhyUeDf = collect_mac_msg_for_v2 ("RxedUePhyCtrlMsgsTrace.txt", rxedColumns, targetz)

txedGnbMacDf = collect_mac_msg_for_v2 ("TxedGnbMacCtrlMsgsTrace.txt", rxedColumns, targetz)
txedUeMacDf = collect_mac_msg_for_v2 ("TxedUeMacCtrlMsgsTrace.txt", rxedColumns, targetz)
txedPhyGnbDf = collect_mac_msg_for_v2 ("TxedGnbPhyCtrlMsgsTrace.txt", rxedColumns, targetz)
txedPhyUeDf = collect_mac_msg_for_v2 ("TxedUePhyCtrlMsgsTrace.txt", rxedColumns, targetz)

txedPhyGnbDf[((txedPhyGnbDf.scenario=="12nodes")&(txedPhyGnbDf.tag=="1251"))]

# macDfz_label =  ["rxedGnbMacDf", "rxedUeMacDf", "rxedPhyGnbDf", "rxedPhyUeDf", "txedGnbMacDf", "txedUeMacDf", "txedPhyGnbDf", "txedPhyUeDf"]
# macDfz =  [rxedGnbMacDf, rxedUeMacDf, rxedPhyGnbDf, rxedPhyUeDf, txedGnbMacDf, txedUeMacDf, txedPhyGnbDf, txedPhyUeDf]
macDfz_label =  ["rxedUeMacDf", "rxedPhyUeDf", "txedUeMacDf", "txedPhyUeDf"]
macDfz =  [rxedUeMacDf , rxedPhyUeDf , txedUeMacDf, txedPhyUeDf]

for index, df in enumerate (macDfz) :
    if (macDfz_label[index][:4] == "txed") : df["cellId"] = df["cellId"] - 1 
    df["scenario_label"] = df["scenario"].apply(lambda x: int(x[:-5])) 
    reduce_memory_usage (df)
    feather_file = macDfz_label[index]+".feather"
    df.reset_index().to_feather(feather_file)
#   ----------------------------------------------------


rxedUeMacDf = pd.read_feather ("rxedUeMacDf.feather")#660x12
txedUeMacDf = pd.read_feather ("txedUeMacDf.feather")#660x12
rxedPhyUeDf = pd.read_feather ("rxedPhyUeDf.feather")#660x12
txedPhyUeDf = pd.read_feather ("txedPhyUeDf.feather")#660x12    

# ------------------------------------------------------------
# one possibility : draw the average BSR/SR/RACHPREAMBE mesg
# ----------------- in the case of each scenario (nb nodes)
#                   --------------------------------------------
coucou = txedUeMacDf.groupby("scenario").mean().reset_index()
coucou["scenario_label"] = coucou["scenario"].apply(lambda x: int(x[:-5])) 
sorted_coucou = coucou.sort_values(by=['scenario_label'])  
plot_multi(sorted_coucou, "scenario_label", ["BSR", "SR", "RACH_PREAMBLE"], spacing=.08, figsize=(14, 6)); 
plt.title ("Transmitted (MAC) control messages by UEs")
# plt.show()              
# -------------------------------------------------------------------------------------------------------------------




# --------------------- 
#   groupby  dataset
# ----------------------------------------------------------------------------------------------------------------------
targetz = ["UL_DCI", "DL_DCI", "DL_CQI", "MIB", "SIB1", "RACH_PREAMBLE", "RAR", "BSR", "DL_HARQ", "SR", "SRS", "UL_UCI",]
grouped_label =  ["rxedUeMacDf", "rxedPhyUeDf", "txedUeMacDf", "txedPhyUeDf"]
macDfz = [rxedUeMacDf, rxedPhyUeDf, txedUeMacDf, txedPhyUeDf]

for label, df in zip(grouped_label, macDfz) :
    megaDf = pd.read_feather(label+".feather")                   
    groupedDf = megaDf.groupby(["scenario","cellId", "RNTI"])[targetz].mean().reset_index()
    groupedDf = groupedDf[groupedDf.RNTI!=0]

    reduce_memory_usage (groupedDf)
    groupedDf.reset_index().to_feather("groupedDf_"+label+".feather")
# --------------------------------------------------------------------

targetz = ["UL_DCI","DL_DCI","DL_CQI","MIB","SIB1","RACH_PREAMBLE","RAR","BSR","DL_HARQ","SR","SRS","UL_UCI"]

groupedDf_rxedUeMacDf = pd.read_feather ("groupedDf_rxedUeMacDf.feather")#660x12
groupedDf_txedUeMacDf = pd.read_feather ("groupedDf_txedUeMacDf.feather")#660x12
groupedDf_rxedPhyUeDf = pd.read_feather ("groupedDf_rxedPhyUeDf.feather")#660x12
groupedDf_txedPhyUeDf = pd.read_feather ("groupedDf_txedPhyUeDf.feather")#660x12

draw_plotbox_for (groupedDf_rxedUeMacDf, ["UL_DCI"], "mac_cntr", title = "Recv MAC Control msg", text_median=False, show=False) 
draw_plotbox_for (groupedDf_txedUeMacDf, ["SR", "BSR"], "mac_cntr", title = "Trans MAC Control msg", text_median=False, show=False) 
draw_plotbox_for (groupedDf_rxedPhyUeDf, ["DL_DCI", "DL_DCI"], "phy_cntr", title = "Recv Phy Control msg", text_median=False, show=False) 
draw_plotbox_for (groupedDf_txedPhyUeDf, ["SRS"], "phy_cntr", title = "Trans Phy Control msg", text_median=False, show=False) 
# ----------------------------------------------------------------------------------------------------------------------

# ---------------------- #
#  raw dataset version   #
# ----------------------------------------------------------------------------------------------------------------------
for label, df in zip(grouped_label, macDfz) :
    megaDf = pd.read_feather(label+".feather")                   
    groupedDf = megaDf.groupby(["scenario","tag", "cellId", "RNTI"])[targetz].mean().reset_index()
    groupedDf = groupedDf[groupedDf.RNTI!=0]

    reduce_memory_usage (groupedDf)
    groupedDf.reset_index().to_feather("rawDf_"+label+".feather")

rawDf_rxedUeMacDf = pd.read_feather ("rawDf_rxedUeMacDf.feather")#660x12
rawDf_txedUeMacDf = pd.read_feather ("rawDf_txedUeMacDf.feather")#660x12
rawDf_rxedPhyUeDf = pd.read_feather ("rawDf_rxedPhyUeDf.feather")#660x12
rawDf_txedPhyUeDf = pd.read_feather ("rawDf_txedPhyUeDf.feather")#660x12    
# --------------------------------------------------------------------

rawzz = [rawDf_rxedUeMacDf, rawDf_txedUeMacDf, rawDf_rxedPhyUeDf, rawDf_txedPhyUeDf]
for index, df in enumerate(rawzz) : 
    print (grouped_label[index],"-->", df.cellId.unique())















#-----------------------------------------------------------
# (2)  Get MAC Stats Ul/Dl : harqId  ndi  rv  mcs  tbSize
#      Files such as  : "NrDlMacStats.txt"
#------------------------------------------------------------------------------------------------
dlMacColumns = ["time",	"cellId", "bwpId", "IMSI", "RNTI", "frame", "sframe", "slot", "symStart", 
                "numSym", "stream", "harqId", "ndi", "rv", "dl_mac_mcs", "dl_mac_TbSize"]
megaNrMacDf = pd.DataFrame (columns=dlMacColumns)      
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))]     
    for directory in directories : 
        files  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'NrDlMacStats.txt', f)]                            
        if (len(files)) :
            littleDf = pd.read_csv(current_dir +"/"+ directory +"/NrDlMacStats.txt", names=dlMacColumns, sep="\t", skiprows=1, index_col=False)
            littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
            megaNrMacDf = megaNrMacDf.append(littleDf) 
        else :
            print ("error somewhere, no dlFlows.csv file inside ", current_dir + "/"+ directory)

reduce_memory_usage (megaNrMacDf)
megaNrMacDf.reset_index().to_feather("megaNrMacDf.feather")
# ---------------------------------------------------------


# -------------------------
#   for groupby dataset
# --------------------------------------------------
megaNrMacDf = pd.read_feather("megaNrMacDf.feather")
targetz      = ["dl_mac_mcs", "dl_mac_TbSize"]                     
groupedDf = megaNrMacDf.groupby(["scenario","IMSI", "cellId", "RNTI"])[targetz].mean().reset_index()
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("groupedDf_nrMacDf.feather")
groupedDf_nrMacDf = pd.read_feather("groupedDf_nrMacDf.feather") 
draw_plotbox_for (groupedDf, targetz, "mac_nr", title = "MAC/NR Traces", text_median=False, show=False) 
# -----------------------------------------------------------------------------------------------------


# -------------------------
#   for raw dataset
# --------------------------------------------------
groupedDf = megaNrMacDf.groupby(["scenario","tag", "IMSI", "cellId", "RNTI"])[targetz].mean().reset_index()
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("rawDf_nrMacDf.feather")
rawDf_nrMacDf = pd.read_feather("rawDf_nrMacDf.feather") 









