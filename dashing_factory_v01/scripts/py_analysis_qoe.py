from myutils import *

# Yin. X QoE model :
# Assuming :
#     1. Average Video Quality: The average per-chunk quality over all chunks: 
#     2. Average Quality Variations: This tracks the magnitude of the changes in the quality from one chunk to another:
#     3. Rebuffer: For each chunk k rebuffering occurs if the download time dk(Rk)/Ck is higher than the playout buffer
#                  level when the chunk download started (i.e., Bk). 
#     4. Startup Delay Ts, assuming Ts << Bmax
# QoE = Average Video Quality - Average Quality Variations - Rebuffer - Startup Delay
# -----------------------------------------------------------------------------------




# ===================
def suffix_col (col):
    metrics_in_kbps = ["avgRateByFrame", "avgRateBytes", "maxRate", "minminRate", "minRate", "avgRate"]
    metrics_in_s    = ["InterruptionTime", "avgDt", "estAvgBufferTime", "initBuffer"]
    metrics_in_KB   = ["queueLengthInBytes"]    
    if col in metrics_in_s : return " (in sec)"
    if col in metrics_in_KB : return " (in KB)"
    if col in metrics_in_kbps : return " (in kbps)"
    return "" # ====================================
# =============



#
def qoe_YinX_v1 (df) :
    qoe = ((df.newBitRate/1024)).sum()- \
          ( ( (df.newBitRate/1024).diff() ).abs()  [1:]).sum()- \
          (df.interruptionTime).sum() - \
          (df.initBuffer).sum()
    return round(qoe, 2)    


def qoe_YinX_v2 (df, qmax) :
    # qoe = (df.newBitRate/1024).sum()/11-( ( (df.newBitRate/1024).diff() ).abs()  [1:]).sum()/10
    qoe = ((df.newBitRate/1024)).sum() /11- \
          ( ( (df.newBitRate/1024).diff() ).abs()  [1:]).sum() /10- \
          (df.interruptionTime).sum()/11 - \
          (df.initBuffer).sum()/11
    qoe = qoe/qmax * 100
    return round(qoe, 2)

megaDashBySegDf = pd.read_feather("megaDashBySegDf.feather")

qoedf = pd.DataFrame()
df = megaDashBySegDf.groupby(["scenario", "tag", "node"]).size().reset_index()
qmax = megaDashBySegDf.newBitRate.max()/1024
qoe_values_v1 = []; qoe_values_v2 = []
qoedf["tag"] =  df.tag
qoedf["node"] =  df.node
qoedf["scenario"] =  df.scenario
for tag, node in zip(df.tag, df.node) :  
    # print (tag, node)
    test = megaDashBySegDf[((megaDashBySegDf.tag==tag)&(megaDashBySegDf.node==node))]    
    qoe_v1 = qoe_YinX_v1 (test); qoe_values_v1.append(qoe_v1)        
    qoe_v2 = qoe_YinX_v2 (test, qmax); qoe_values_v2.append(qoe_v2)        
    
qoedf["qoe_YinX_v1"] =  qoe_values_v1
qoedf["qoe_YinX_v2"] =  qoe_values_v2
targetz = ["qoe_YinX_v1", "qoe_YinX_v2"]

reduce_memory_usage (qoedf)
qoedf.reset_index().to_feather("megaQoE_Df.feather")
groupedDf = pd.read_feather("megaQoE_Df.feather") 




# ------------------------------------------
# Draw the mean, Max and Min values together
# in multiple plots
# ------------------
fig = plt.figure(dpi=600)
parameterz = targetz
qoedf["scenario_label"] = qoedf["scenario"].apply(lambda x: int(x[:-5])) 
qoedf = qoedf.sort_values(by=["scenario_label"])
scenarioz = qoedf.scenario.unique()    
for parameter in parameterz: 
    plt.clf()
    index = 0        
    y_values = []
    for scenarioId in scenarioz :  
        index+=1                  
        scenarioValues = qoedf[qoedf.scenario==scenarioId][parameter]
        y_values.append(scenarioValues)
        ymax = scenarioValues.median()
        plt.text(index-0.5, ymax*1.1, str(ymax), color="red", fontsize=6)         
    box = plt.boxplot(y_values, sym='.')    
    # change the color of its elements
    # for _, line_list in box.items():
    #     for line in line_list:
    #         line.set_color('gray')
    plt.xticks(list(range(len(scenarioz)+1))[1:], list(scenarioz), fontsize=8, rotation=30)
    plt.ylabel(parameter+suffix_col(parameter))
    plt.title(parameter)
    plt.savefig("plotboxVersion_"+parameter+".jpg")
# ------------------------------------------------




# ------------------------------------------
# Draw the mean, Max and Min values together
# in multiple plots
# ------------------
fig = plt.figure(dpi=600)
parameterz = ["qoe_YinX_v2"]
qoedf["scenario_label"] = qoedf["scenario"].apply(lambda x: int(x[:-5])) 
qoedf = qoedf.sort_values(by=["scenario_label"])
scenarioz = qoedf.scenario.unique()    
for parameter in parameterz: 
    plt.clf()
    index = 0        
    y_values = []
    for scenarioId in scenarioz :   
        index+=1                  
        scenarioValues = qoedf[qoedf.scenario==scenarioId][parameter]
        y_values.append(scenarioValues)
        ymax = scenarioValues.median()
        plt.text(index-0.5, ymax*1.1, str(ymax), color="red", fontsize=6)         
    box = plt.boxplot(y_values, sym='.')    
    plt.xticks(list(range(len(scenarioz)+1))[1:], list(scenarioz), fontsize=8, rotation=30)
    plt.ylabel("Normalized QoE [scale of 0-100]")
    plt.title("Achieved Quality of Experience (QoE)")
    plt.savefig("qoe_normalized"+parameter+".jpg")
# ------------------------------------------------






















# ===================
def suffix_col (col):
    if col in metrics_in_s : return " (in sec)"
    if col in metrics_in_KB : return " (in KB)"
    if col in metrics_in_kbps : return " (in kbps)"
    return "" # ====================================
# =============




# ++++++++ #
#   DASH   #
#   STUFF  #
# ++++++++ #

# ----# 
#  1  # 
# ----# 

# -------------------------------------
# By Station, i.e. dashStats.csv files
# ----------------------------------------
# Stats at the traffic flow / node level 
# This is intended to be a recap outlining 
# the results of one run at the level of each streaming flow 
# --------------------------------------------------------------

dashColumns = ["node","InterruptionTime","interruptions","framesPlayed","avgRateByFrame","avgRateBytes","maxRate",
               "minminRate","minRate", "avgRate","avgDt","estAvgBufferTime","changes","initBuffer","queueLength", 
               "queueLengthInBytes", "tag", "scenario"]   

metrics_in_kbps = ["avgRateByFrame", "avgRateBytes", "maxRate", "minminRate", "minRate", "avgRate"]
metrics_in_s    = ["InterruptionTime", "avgDt", "estAvgBufferTime", "initBuffer"]
metrics_in_KB   = ["queueLengthInBytes"]

megaDashDf = pd.DataFrame (columns=dashColumns)                          
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 

for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))] 
    dashDf = pd.DataFrame (columns=dashColumns)                              
    for directory in directories :                  
        littleDf = pd.read_csv(current_dir + "/" + directory +"/dashStats.csv", names=dashColumns, skiprows=1, index_col=False)
        littleDf ["tag"] = directory; littleDf ["scenario"] = current_dir        
        dashDf = dashDf.append(littleDf) 
    megaDashDf = megaDashDf.append(dashDf)  

# clean the stuff : delete units from the corresponding column (sec, KB, kbps) 
megaDashDf[metrics_in_s]    = megaDashDf[metrics_in_s].apply(lambda x: x.apply(lambda y: float(y[:-1])), axis=1)
megaDashDf[metrics_in_KB]   = megaDashDf[metrics_in_KB].apply(lambda x: x.apply(lambda y: float(y[:-2])), axis=1)
megaDashDf[metrics_in_kbps] = megaDashDf[metrics_in_kbps].apply(lambda x: x.apply(lambda y: float(y[:-4])), axis=1)   

reduce_memory_usage (megaDashDf)
megaDashDf.reset_index().to_feather("megaDashDf.feather")
# -----------------------------------------------------------


# ---------------------------------------------------
#        groupby "scenario", "segmentId" and "nodes"
#   i.e. nodes are all merged (averaged) makach manha ...
# ---------------------------------------------------
megaDashDf = pd.read_feather("megaDashDf.feather")
reduce_memory_usage (megaDashDf)
target_colz = [col for col in megaDashDf.columns if col not in ["tag", "scenario", "node", "index"]]
# aggregate cols per node
groupedDf = megaDashDf.groupby(["scenario", "node",])[target_colz].mean()
groupedDf = groupedDf.reset_index()
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("groupedDf_dashV1.feather")
groupedDf_dashV1 = pd.read_feather("groupedDf_dashV1.feather") 
# --------------------------------



# ------------------------------------------
# Draw the mean, Max and Min values together
# in multiple plots
# ------------------
fig = plt.figure(dpi=600)
parameterz = target_colz
groupedDf["scenario_label"] = groupedDf["scenario"].apply(lambda x: int(x[:-5])) 
groupedDf = groupedDf.sort_values(by=["scenario_label"])

scenarioz = groupedDf.scenario.unique()    
for parameter in parameterz: 
    plt.clf()
    index = 0        
    y_values = []
    for scenarioId in scenarioz :  
        index+=1                  
        scenarioValues = groupedDf[groupedDf.scenario==scenarioId][parameter]
        y_values.append(scenarioValues)
        ymax = scenarioValues.median()
        plt.text(index-0.5, ymax, str(ymax), color="red", fontsize=8)         

    plt.boxplot(y_values)
    plt.xticks(list(range(len(scenarioz)+1))[1:], list(scenarioz), fontsize=8, rotation=30)
    plt.ylabel(parameter+suffix_col(parameter))
    plt.title(parameter)
    plt.savefig("plotboxVersion_"+parameter+".jpg")
# ------------------------------------------------


# ==================================================







# ----# 
#  2  # 
# ----# 

# ------------
# By Segment :
# ------------
# These file include details of the streaming flow at the segment level !
# i.e. each line (entry in dataset) is logged once a segment is received successfully, 
# -------------------------------------------------------------------------------------------------------------------

megaDashBySegColumns = ["timestamp","segmentId","frameId","node","playedFrames","requestTime","segmentFetchTime",
                   "segmentSize","bufferTime","deltaBufferTime","estAvgBufferTime","newBitRate","oldBitRate",
                   "changes","estBitRate","segmentRate","interruptions","interruptionTime","queueLength","queueSize",
                   "initBuffer"]  
targetzBySeg    = ["playedFrames","requestTime","segmentFetchTime", "segmentSize","bufferTime","deltaBufferTime",
                  "estAvgBufferTime","newBitRate","oldBitRate", "changes","estBitRate","segmentRate","interruptions",
                  "interruptionTime","queueLength","queueSize", "initBuffer"] 
metrics_in_s    = ["requestTime", "segmentFetchTime", "bufferTime", "deltaBufferTime", "estAvgBufferTime", 
                  "interruptionTime", "initBuffer"] 
metrics_in_KB   = ["segmentSize", "queueSize"]
metrics_in_kbps = ["newBitRate", "oldBitRate", "estBitRate", "segmentRate"]                                    
functionz       = ["mean", "median", "var"]                                                

megaDashBySegDf = pd.DataFrame (columns=targetzBySeg)                          
meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 

for current_dir in meta_directories : 
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))] 
    for directory in directories :                  
        print ("we are in --> ", current_dir + "/"+ directory)                                                                     
        bySegFiles  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'dashSegmentStatsByNode_.*\.csv', f)]                          
        if (len(bySegFiles)) : 
            dashBySegDf = pd.read_csv(current_dir + "/" + directory +"/"+bySegFiles[0], names=megaDashBySegColumns, skiprows=1, index_col=False)
            for file in bySegFiles [1:] :
                littleDf = pd.read_csv(current_dir + "/" + directory +"/"+file, names=megaDashBySegColumns, skiprows=1, index_col=False)
                dashBySegDf = dashBySegDf.append(littleDf)   

            # clean the stuff : delete units from the corresponding column (sec, KB, kbps) 
            dashBySegDf[metrics_in_s]    = dashBySegDf[metrics_in_s].apply(lambda x: x.apply(lambda y: float(y[:-1])), axis=1)
            dashBySegDf[metrics_in_KB]   = dashBySegDf[metrics_in_KB].apply(lambda x: x.apply(lambda y: float(y[:-2])), axis=1)
            dashBySegDf[metrics_in_kbps] = dashBySegDf[metrics_in_kbps].apply(lambda x: x.apply(lambda y: float(y[:-4])), axis=1)   
            dashBySegDf["tag"] = directory
            dashBySegDf["scenario"] = current_dir 

            megaDashBySegDf = megaDashBySegDf.append(dashBySegDf)  

reduce_memory_usage (megaDashBySegDf)
megaDashBySegDf.reset_index().to_feather("megaDashBySegDf.feather")



# ============================================================
#  VERSION 2 of groupby : groupby "scenario" and "segmentId"
#                         i.e. nodes are all merged (averaged)
# ==========================================================
megaDashBySegDf = pd.read_feather("megaDashBySegDf.feather")

# Essentially, in this version, we aggregate cols per node
to_use_mean = ["segmentSize","bufferTime","deltaBufferTime", "estAvgBufferTime","newBitRate",
               "oldBitRate", "changes","estBitRate","segmentRate","interruptions","interruptionTime"]
to_use_last = ["playedFrames", "queueLength", "queueSize", "initBuffer"]                                      

to_use_var = [col+"_var" for col in to_use_mean + to_use_last]
to_use_max = [col+"_max" for col in to_use_mean + to_use_last]
to_use_min = [col+"_min" for col in to_use_mean + to_use_last]
groupedDf2 = megaDashBySegDf.groupby(["scenario","segmentId"])[to_use_mean+to_use_last].mean()
groupedDf2 [to_use_var] = megaDashBySegDf.groupby(["scenario", "segmentId"])[to_use_mean+to_use_last].var() 
groupedDf2 [to_use_max] = megaDashBySegDf.groupby(["scenario", "segmentId"])[to_use_mean+to_use_last].max() 
groupedDf2 [to_use_min] = megaDashBySegDf.groupby(["scenario", "segmentId"])[to_use_mean+to_use_last].min() 
groupedDf2 = groupedDf2.reset_index()
reduce_memory_usage (groupedDf2)
# ==============================


# -------------------------------------
# Start with some illustrative graphs   # -----------------------------------------
# just for 12nodes scenario -->  
#                  newbitrate : 1- Mean of all and all, i.e. nodes and tags together (one fig)
#                               2- A raw alltogether version  # -------------------------------
# -----------------------------------------------------------

# -----------------------
# 1-mean of alltogether 
# ---------------------------
parameterz = ["newBitRate",]

megaDashBySegDf["scenario_label"] = megaDashBySegDf["scenario"].apply(lambda x: int(x[:-5])) 
megaDashBySegDf = megaDashBySegDf.sort_values(by=["scenario_label"])

scenarioz = [megaDashBySegDf.scenario.unique()[0],]    
for parameter in parameterz: 
    plt.clf()
    index = 0        
    y_values = []        
    for scenarioId in scenarioz :  
        index+=1 
        ydf = megaDashBySegDf[megaDashBySegDf.scenario==scenarioId]
        yydf = ydf.groupby(["segmentId"])[parameter].mean().reset_index()
        X = yydf.segmentId.unique ()
        Y = yydf [parameter] 
        ymax = Y.max(); xmax = X[np.where(Y == ymax)]
        plt.plot(X, Y) 
        plt.xlim(0, X.max())       
        plt.ylabel(parameter+suffix_col(parameter))        
        plt.xlabel("Received Segments")        
        plt.axline((0, ymax), (X[-1], ymax), linestyle="--", color="red", linewidth=1)
        plt.text(0.1, ymax*1.01, ymax, color="red", fontsize=8)                 
        # plt.text(xmax.mean(), ymax*1.01, ymax, color="red", fontsize=8)                 
        plt.title(parameter+suffix_col(parameter), fontsize=12)
        plt.savefig("illustrative_01_"+parameter+"_mean_"+scenarioId+".jpg")        
        # plt.show()  # ------------------------------------------------------      
#      ------------


# -------------------------------
# 2- A raw alltogether version
# --------------------------
parameterz = ["newBitRate",]
scenarioz = [megaDashBySegDf.scenario.unique()[0],]    
for parameter in parameterz: 
    plt.clf()
    index = 0        
    y_values = []        
    for scenarioId in scenarioz :  
        index+=1 
        ydf = megaDashBySegDf[megaDashBySegDf.scenario==scenarioId]
        yydf = ydf[["tag", "node", "segmentId", "newBitRate"]] 
        yyydf = yydf.groupby(["tag", "node"])[parameter].apply(list).reset_index()       
        for raw in yyydf[parameter] : 
            plt.plot(raw)
        plt.legend()
        plt.xlim(0, X.max())       
        plt.ylabel(parameter+suffix_col(parameter))        
        plt.xlabel("Received Segments")                                
        plt.title(parameter+suffix_col(parameter), fontsize=12)
        plt.savefig("illustrative_01_"+parameter+"_rawtogether_"+scenarioId+".jpg")        
        # plt.show()        
# ------------------------------------------------



# -------------------------
# 3- A raw version by tag 
#    (of corse by scenario also)
# -----------------------------
parameterz = ["newBitRate",] # ---------------------
scenarioz = [megaDashBySegDf.scenario.unique()[0],]    
for parameter in parameterz: 
    index = 0        
    y_values = []        
    for scenarioId in scenarioz :  
        index+=1 
        ydf = megaDashBySegDf[megaDashBySegDf.scenario==scenarioId]
        yydf = ydf[["tag", "node", "segmentId", "newBitRate"]] 
        for tag in yydf.tag.unique() :
            plt.clf()
            fig = plt.figure(figsize=(15, 10))
            yyydf = yydf [yydf.tag == tag]
            yyyydf = yyydf.groupby(["node"])[parameter].apply(list).reset_index()       
            index = 1
            for raw in yyyydf[parameter] : 
                plt.plot(raw, label= "ue"+str(index), marker=styles[index], linestyle="--")
                index +=1
            plt.legend(loc='center left',  bbox_to_anchor=(1, 0.5))
            plt.xlim(0, X.max())       
            plt.ylabel(parameter+suffix_col(parameter))        
            plt.xlabel("Received Segments")                                
            plt.title(parameter+suffix_col(parameter)+ "  (tag="+tag+")", fontsize=12)
            plt.savefig(scenarioId+"/"+tag + "_"+parameter+"_rawSingle_"+scenarioId+".jpg")                
# ------------------




# ------------------------------------------------------
# 4- A simpler version. I want the performance of one 
#    node in one tag, one scenario. For illustrative aims
# ----------------------------------------------------------

# One node apart
def single_graph_for (tag, parameter, node, show=False) :
    node =node-1
    ydf = megaDashBySegDf[["tag", "node", "segmentId", parameter]] 
    ylim = ydf[parameter].max()*1.1; # xlim = ydf["segmentId"].max()*1.1
    yydf = ydf [((ydf.tag == tag) & (ydf.node == node))]
    Y = yydf[parameter]; ymax = Y.max(); ymin = Y.min()
    X = yydf["segmentId"].unique(); xmax = X.max()

    plt.clf()
    fig = plt.figure(figsize=(15, 10), dpi=800)
    plt.plot(X, Y, label= "ue"+str(int(node+1)), linestyle="--")

    plt.axline((0, ymax), (xmax, ymax), linestyle=":", color="red", linewidth=1)
    plt.text(0.1, ymax*1.01, "max="+str(ymax), color="red", fontsize=8)                 

    plt.axline((0, ymin), (0.01, ymin), linestyle=":", color="red", linewidth=1)
    plt.text(0.1, ymin*1.01, "min="+str(ymin), color="red", fontsize=8)                 

    plt.legend(loc='center left',  bbox_to_anchor=(1, 0.5))
    plt.xlim(0, xmax); plt.ylim(0, ylim)              

    plt.ylabel(parameter+suffix_col(parameter), fontsize=16)        
    plt.xlabel("Received Segments", fontsize=16)                                

    plt.title(parameter+suffix_col(parameter)+" for node "+ str(int(node+1)) +" (tag="+tag+")",fontsize=18, color="r")

    plt.savefig(scenarioId+"/"+tag+"_node"+str(int(node))+"_"+parameter+".jpg") 
    if (show) : plt.show()               

# Multiple node apart
def together_graph_for (tag, parameter, nodes, show=False) :
    ydf = megaDashBySegDf[["tag", "node", "segmentId", parameter]] 
    ylim = ydf[parameter].max()*1.1; # xlim = ydf["segmentId"].max()*1.1
    fig = plt.figure(figsize=(10, 7), dpi=800)    
    plt.clf()    
    for node in nodes :
        node =node-1
        yydf = ydf [((ydf.tag == tag) & (ydf.node == node))]
        Y = yydf[parameter]; ymax = Y.max(); ymin = Y.min()
        X = yydf["segmentId"].unique(); xmax = X.max()    
        plt.plot(X, Y, label= "ue"+str(int(node+1)), linestyle="--", color = colors[node+20])    
        plt.axline((0, ymax), (xmax, ymax), linestyle=":", color = colors[node+20], linewidth=1)
        plt.text(0.1, ymax*1.01, "max="+str(ymax), color = colors[node+20], fontsize=10)                         

    plt.legend(loc='center left',  bbox_to_anchor=(1, 0.5))
    plt.ylabel(parameter+suffix_col(parameter), fontsize=12)        
    plt.xlabel("Received Segments", fontsize=12)                                

    plt.title(parameter+suffix_col(parameter)+" for selected nodes  (tag="+tag+")",fontsize=14, color="r")

    plt.savefig(scenarioId+"/"+tag+"_selectedNode_"+parameter+".jpg") 
    print (parameter," for selected nodes  (tag=",tag,") saved on : [", scenarioId,"/",tag,"_selectedNode_",parameter,".jpg") 
    if (show) : plt.show() 


# Multiple node apart
def mean_graph_for_tag (tag, parameter, nodes, show=False) :
    ydf = megaDashBySegDf[["tag", "node", "segmentId", parameter]] 
    ylim = ydf[parameter].max()*1.1; # xlim = ydf["segmentId"].max()*1.1
    fig = plt.figure(figsize=(10, 7), dpi=800)    
    plt.clf()    
    for node in nodes :
        node =node-1
        yydf = ydf [((ydf.tag == tag) & (ydf.node == node))]
        Y = yydf[parameter]; ymax = Y.max(); ymin = Y.min()
        X = yydf["segmentId"].unique(); xmax = X.max()    
        plt.plot(X, Y, label= "ue"+str(int(node+1)), linestyle="--", color = colors[node+20])    
        plt.axline((0, ymax), (xmax, ymax), linestyle=":", color = colors[node+20], linewidth=1)
        plt.text(0.1, ymax*1.01, "max="+str(ymax), color = colors[node+20], fontsize=10)                         

    plt.legend(loc='center left',  bbox_to_anchor=(1, 0.5))
    plt.ylabel(parameter+suffix_col(parameter), fontsize=12)        
    plt.xlabel("Received Segments", fontsize=12)                                

    plt.title(parameter+suffix_col(parameter)+" for selected nodes  (tag="+tag+")",fontsize=14, color="r")

    plt.savefig(scenarioId+"/"+tag+"_selectedNode_"+parameter+".jpg") 
    print (parameter," for selected nodes  (tag=",tag,") saved on : [", scenarioId,"/",tag,"_selectedNode_",parameter,".jpg") 
    if (show) : plt.show() 
# ---------------------------------------------------------------------
together_graph_for (tag="5391", nodes=[1,6], parameter="newBitRate")
single_graph_for (tag="5391", node=6, parameter="newBitRate")
single_graph_for (tag="5391", node=1, parameter="newBitRate")










# ===========================================
# Draw the mean, Max and Min values together
# in multiple plots
# ===========================================
groupedDf2["scenario_label"] = groupedDf2["scenario"].apply(lambda x: int(x[:-5])) 
# groupedDf2 = groupedDf2.sort_values(by=["scenario_label"])

parameterz = to_use_mean + to_use_last #["segmentSize",]
for parameter in parameterz : 
    plt.clf()
    fig = plt.figure(figsize=(17, 11), dpi=300)
    gs = fig.add_gridspec(3, 4, hspace=0.5, wspace=0.2)
    (ax1, ax2, ax3, ax4), (ax5, ax6, ax7, ax8), (ax9, ax10, _, ax12) = gs.subplots()
    axz = [ax1, ax2, ax3, ax4, ax5, ax6, ax7, ax8, ax9, ax10, ax12]
    # scenarioz = groupedDf2.scenario.unique()
    scenarioz = [str(x)+"nodes" for x in sorted(groupedDf2.scenario_label.unique())]
    index = 0
    ymax_values = []
    xmax_values = []
    for scenarioId in scenarioz :    
        print ("parameter : ",parameter)
        print ("    ---> scenarioId : ",scenarioId)
        tempDff = groupedDf2[groupedDf2.scenario==scenarioId]    
        Ymean = tempDff[parameter].values
        Ymax = tempDff[parameter+"_max"].values
        ymax_value = Ymax.max(); ymax_values.append(ymax_value)
        Ymin = tempDff[parameter+"_min"].values    
        X = tempDff.segmentId.values 
        xmax_value = X.max(); xmax_values.append(xmax_value)       
        axz[index].plot(X, Ymean, label=scenarioId, c=colors[index+20])
        axz[index].plot(X, Ymax, ":", c=colors[index+20])
        axz[index].plot(X, Ymin, ":", c=colors[index+20])
        axz[index].fill_between(X, Ymax, Ymin, alpha=0.2)
        axz[index].axline((0, ymax_value), (X[-1], ymax_value), linestyle="--", 
                           color="red", linewidth=1)
        axz[index].text(0.1, ymax_value*1.01, ymax_value, color="red", fontsize=8) 
        axz[index].set_title(scenarioId)
        axz[index].set_xlim(0, xmax_value)
        index+=1; 
    
    lastx = index; index = 0
    for scenarioId in scenarioz :
        tempDff = groupedDf2[groupedDf2.scenario==scenarioId]    
        Ymean = tempDff[parameter].values
        X = tempDff.segmentId.values 
        axz[-1].plot(X, Ymean, label=scenarioId, c=colors[index+20])
        axz[-1].set_title("A summary")
        axz[-1].legend(loc='center left',  bbox_to_anchor=(1, 0.5))
        index+=1; 
    
    fig.suptitle(parameter +" by segment received", fontsize=18, color="red")  
    fig.text(0.4,0.92, " averaged over all UEs", fontsize=14, color="red")  
    fig.supxlabel("Received Segment")  
    fig.supylabel(parameter+suffix_col(parameter)) 
    for ax in fig.get_axes():
        ax.set_ylim((0, max(ymax_values)*1.15))
        
    #fig.show()
    plt.savefig ("smoothVersion_"+parameter+".jpg")    
# ------------------------------------------------







# ============================================================
#  VERSION 3 of groupby : groupby "scenario" and "segmentId"
#                         i.e. nodes are all merged (averaged)
# ==========================================================
megaDashBySegDf = pd.read_feather("megaDashBySegDf.feather")

megaDashBySegColumns = ["timestamp","segmentId","frameId","node","playedFrames","requestTime","segmentFetchTime",
                        "segmentSize","bufferTime","deltaBufferTime","estAvgBufferTime","newBitRate","oldBitRate",
                        "changes","estBitRate","segmentRate","interruptions","interruptionTime","queueLength","queueSize",
                        "initBuffer"]  
targetzBySeg    = ["playedFrames","requestTime","segmentFetchTime", "segmentSize","bufferTime","deltaBufferTime",
                   "estAvgBufferTime","newBitRate","oldBitRate", "changes","estBitRate","segmentRate","interruptions",
                   "interruptionTime","queueLength","queueSize", "initBuffer"] 


metrics_in_s    = ["requestTime", "segmentFetchTime", "bufferTime", "deltaBufferTime", "estAvgBufferTime", 
                   "interruptionTime", "initBuffer"] 
metrics_in_KB   = ["segmentSize", "queueSize"]
metrics_in_kbps = ["newBitRate", "oldBitRate", "estBitRate", "segmentRate"]                                    

functionz       = ["mean", "median", "var"]                                                

# Essentially, in this version, we aggregate cols per node
to_use_mean = ["segmentSize","bufferTime","deltaBufferTime", "estAvgBufferTime","newBitRate",
               "oldBitRate", "estBitRate","segmentRate","interruptions","interruptionTime"]
to_use_last = ["playedFrames", "queueLength", "queueSize", "initBuffer", "changes"]                                      
prev_cols =   ["scenario", "tag","node"]

finalDashDf = pd.DataFrame ()
finalDashDf[prev_cols+to_use_last] = megaDashBySegDf.groupby(["scenario", "tag","node"])[to_use_last].nth(-1).reset_index()
finalDashDf[prev_cols+to_use_mean] = megaDashBySegDf.groupby(["scenario", "tag","node"])[to_use_mean].mean().reset_index()
