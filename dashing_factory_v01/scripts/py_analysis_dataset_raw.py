from myutils import *

# PHY  --------------------------------------------------------
rawDf_sinrDf = pd.read_feather("rawDf_sinrDf.feather") #4728x6
# rawDf_sinrDf = rawDf_sinrDf[rawDf_sinrDf.tag!="6225"] #4644x6
rawDf_dlPathLossDf = pd.read_feather("rawDf_dlPathLossDf.feather") #4644x5
rawDf_ulPathLossDf = pd.read_feather("rawDf_ulPathLossDf.feather") #4644x5
rawDf_dlRxPacketDf = pd.read_feather("rawDf_dlRxPacketDf.feather")
# rawDf_dlRxPacketDf = rawDf_dlRxPacketDf[rawDf_dlRxPacketDf.tag!="6225"] #4644x11
rawDf_ulRxPacketDf = pd.read_feather("rawDf_ulRxPacketDf.feather")
# rawDf_ulRxPacketDf = rawDf_ulRxPacketDf[rawDf_ulRxPacketDf.tag!="6225"] #4644x11
# ------------------------------------------------------------------------
 
# MAC  --------------------------------------------------------
rawDf_nrMacDf = pd.read_feather("rawDf_nrMacDf.feather") 
# rawDf_nrMacDf = rawDf_nrMacDf[~rawDf_nrMacDf.tag.isin (['1372', '3658', '6225'])] 

macDfz_label =  ["rxedUeMacDf", "rxedPhyUeDf", "txedUeMacDf", "txedPhyUeDf"]
# col_2del = ['1372', '3658', '5565', '6225']

for df in macDfz_label :
    rawDf_xDf = pd.read_feather ("rawDf_"+df+".feather")
    # rawDf_xDf = rawDf_xDf[~rawDf_xDf.tag.isin(col_2del)]
    # rawDf_xDf = rawDf_xDf.drop("level_0", axis=1) 
    rawDf_xDf.reset_index().to_feather("rawDf_"+df+".feather")

rawDf_rxedUeMacDf = pd.read_feather ("rawDf_rxedUeMacDf.feather")#4644x18
rawDf_txedUeMacDf = pd.read_feather ("rawDf_txedUeMacDf.feather")#4644x18
rawDf_rxedPhyUeDf = pd.read_feather ("rawDf_rxedPhyUeDf.feather")#4644x18
rawDf_txedPhyUeDf = pd.read_feather ("rawDf_txedPhyUeDf.feather")#4644x18    
# ------------------------------------------------------------------------

# RLC --------------------------------------------------------
rawDf_dlRlcDf = pd.read_feather ("rawDf_dlRlcDf.feather")#4980x16    
# rawDf_dlRlcDf = rawDf_dlRlcDf[~rawDf_dlRlcDf.tag.isin (col_2del)] #4644x16    
rawDf_ulRlcDf = pd.read_feather ("rawDf_ulRlcDf.feather")#4980x16    
# rawDf_ulRlcDf = rawDf_ulRlcDf[~rawDf_ulRlcDf.tag.isin (col_2del)] #4644x16    
# ------------------------------------------------------------------------

# PDCP --------------------------------------------------------
rawDf_dlPdcpDf = pd.read_feather ("rawDf_dlPdcpDf.feather")#4980x16    
# rawDf_dlPdcpDf = rawDf_dlPdcpDf[~rawDf_dlPdcpDf.tag.isin (col_2del)] #4644x16    
rawDf_ulPdcpDf = pd.read_feather ("rawDf_ulPdcpDf.feather")#4980x16    
# rawDf_ulPdcpDf = rawDf_ulPdcpDf[~rawDf_ulPdcpDf.tag.isin (col_2del)] #4644x16    
# ----------------------------------------------------------------------------

# IP   --------------------------------------------------------
rawDf_ipDlDf = pd.read_feather ("rawDf_ipDlDf.feather")#4980x14    
#rawDf_ipDlDf = rawDf_ipDlDf[~rawDf_ipDlDf.tag.isin (col_2del)] #4644x14 
rawDf_ipUlDf = pd.read_feather ("rawDf_ipUlDf.feather")#4980x14    
#rawDf_ipUlDf = rawDf_ipUlDf[~rawDf_ipUlDf.tag.isin (col_2del)] #4644x14       
# ---------------------------------------------------------------------

# TCP  --------------------------------------------------------------
# groupedDf_tcpDf = pd.read_feather("groupedDf_tcpMetaDf.feather")#21
# -------------------------------------------------------------------

# DASH   ------------------------------------------------------
rawDf_dashV1 = pd.read_feather ("rawDf_dashV1.feather")#4980x19  
# rawDf_dashV1 = rawDf_dashV1[~rawDf_dashV1.tag.isin (col_2del)] #4644x19       
# ----------------------------------------------------------------------

# Scenario   -------------------------------------------------------
rawDf_scenarioDf = pd.read_feather ("rawDf_scenarioDf.feather")#85x4  
# rawDf_scenarioDf = rawDf_scenarioDf[~rawDf_scenarioDf.tag.isin (col_2del)] #81x4
# -------------------------------------------------------------------------------





###########################################
# Time to collect the allinone grouped df #
###########################################

# DASH 
rawDf_dashV1["node"] = rawDf_dashV1["node"] + 1
rawDf_dashV1 = rawDf_dashV1.drop("index", axis=1) #(4644x18colz)
grouped = rawDf_dashV1 #(4644x18colz)
# DASH|Scenario 
rawDf_scenarioDf = rawDf_scenarioDf.drop("index", axis=1) #(81x3)
grouped = grouped.merge(rawDf_scenarioDf, on=["scenario", "tag"]) #(4644x19)

# DASH|Scenario|IP 
rawDf_ipUlDf = rawDf_ipUlDf.drop("index", axis=1) #(4644x13)
grouped = grouped.merge(rawDf_ipUlDf, on=["scenario", "tag", "node"]) #(4644x29)

# DASH|Scenario|IP|PDCP(ul) 
rawDf_ulPdcpDf = rawDf_ulPdcpDf.drop("index", axis=1) #(4644x15)
rawDf_ulPdcpDf["node"] = rawDf_ulPdcpDf.IMSI #(4644x15)
grouped = grouped.merge(rawDf_ulPdcpDf, on=["scenario","tag", "node"]) #(4644x42)

# DASH|Scenario|IP|PDCP(ul)|RLC(ul) 
rawDf_ulRlcDf = rawDf_ulRlcDf.drop("index", axis=1)
rawDf_ulRlcDf["node"] = rawDf_ulRlcDf.IMSI #(46440x15)
grouped = grouped.merge(rawDf_ulRlcDf, on=["scenario","tag", "node"]) #(4644x54)

# DASH|Scenario|IP|PDCP(ul)|RLC(ul)|MAC(nrMac)
rawDf_nrMacDf = rawDf_nrMacDf.drop("index", axis=1) 
rawDf_nrMacDf["node"] = rawDf_nrMacDf.IMSI #(4644x8)
grouped = grouped.merge(rawDf_nrMacDf, on=["scenario","tag", "node"]) #(4644x60)

# DASH|Scenario|IP|PDCP(ul)|RLC(ul)|MAC(nrMac)|PHY
grouped = grouped.rename(columns={'RNTI': 'rnti'})
grouped = grouped.merge(rawDf_sinrDf, on=["scenario","tag","cellId",  "rnti"]) #(4644x62) 

rawDf_dlPathLossDf = rawDf_dlPathLossDf.drop("index", axis=1) #(4644x4)
grouped = grouped.merge(rawDf_dlPathLossDf, on=["scenario","tag", "IMSI"]) #(4644x63)

rawDf_ulPathLossDf = rawDf_ulPathLossDf.drop("index", axis=1) #(4644x3)
grouped = grouped.merge(rawDf_ulPathLossDf, on=["scenario","tag", "IMSI"]) #(4644x64)

rawDf_ulRxPacketDf = rawDf_ulRxPacketDf.drop(["index"], axis=1) #(4644x11)
grouped = grouped.merge(rawDf_ulRxPacketDf, on=["scenario", "tag", "cellId",  "rnti"]) #(4644x71)

rawDf_dlRxPacketDf = rawDf_dlRxPacketDf.drop(["index"], axis=1) #(4644x11)
grouped = grouped.merge(rawDf_dlRxPacketDf, on=["scenario","tag", "cellId",  "rnti"]) #(4644x78)

# ------------------------------------------------------------------------
# DASH|Scenario|IP|PDCP(ul)|RLC(ul)|MAC(nrMac)|PHY|MAC(remaining)
# rawDf_rxedUeMacDf = pd.read_feather ("rawDf_rxedUeMacDf.feather").drop(["index"], axis=1) #(4644x16)
rawDf_rxedUeMacDf = pd.read_feather ("rawDf_rxedUeMacDf.feather").drop(["level_0", "index"], axis=1) #(4644x16)
newcolz = {col: col+"_rxMac" for col in rawDf_rxedUeMacDf.columns[4:]}
rawDf_rxedUeMacDf = rawDf_rxedUeMacDf.rename(columns=newcolz)
rawDf_rxedUeMacDf = rawDf_rxedUeMacDf.rename(columns={'RNTI': 'rnti'}) #(4644x16)
rawDf_rxedUeMacDf["cellId"] = rawDf_rxedUeMacDf["cellId"] - 1
grouped = grouped.merge(rawDf_rxedUeMacDf, on=["scenario","tag", "cellId",  "rnti"]) #(4644x90)


# rawDf_txedUeMacDf = pd.read_feather ("rawDf_txedUeMacDf.feather").drop(["level_0", "index"], axis=1) #(4644x16)
rawDf_txedUeMacDf = pd.read_feather ("rawDf_txedUeMacDf.feather").drop(["level_0", "index"], axis=1) #(4644x16)
newcolz = {col: col+"_txMac" for col in rawDf_txedUeMacDf.columns[4:]}                                                        
rawDf_txedUeMacDf = rawDf_txedUeMacDf.rename(columns=newcolz)
rawDf_txedUeMacDf = rawDf_txedUeMacDf.rename(columns={'RNTI': 'rnti'}) #(4644x17)
grouped = grouped.merge(rawDf_txedUeMacDf, on=["scenario","tag", "cellId",  "rnti"]) #(4644x102)

rawDf_rxedPhyUeDf = pd.read_feather ("rawDf_rxedPhyUeDf.feather").drop(["level_0", "index"], axis=1) #(4644x16)
newcolz = {col: col+"_rxPhy" for col in rawDf_rxedPhyUeDf.columns[4:]}                                                        
rawDf_rxedPhyUeDf = rawDf_rxedPhyUeDf.rename(columns=newcolz)
rawDf_rxedPhyUeDf = rawDf_rxedPhyUeDf.rename(columns={'RNTI': 'rnti'})
rawDf_rxedPhyUeDf["cellId"] = rawDf_rxedPhyUeDf["cellId"] - 1
grouped = grouped.merge(rawDf_rxedPhyUeDf, on=["scenario","tag", "cellId",  "rnti"]) #(4644x114)

rawDf_txedPhyUeDf = pd.read_feather ("rawDf_txedPhyUeDf.feather").drop(["level_0", "index"], axis=1) #(4644x16)
newcolz = {col: col+"_rxPhy" for col in rawDf_txedPhyUeDf.columns[4:]}                                                        
rawDf_txedPhyUeDf = rawDf_txedPhyUeDf.rename(columns=newcolz)
rawDf_txedPhyUeDf = rawDf_txedPhyUeDf.rename(columns={'RNTI': 'rnti'})
grouped = grouped.merge(rawDf_txedPhyUeDf, on=["scenario","tag", "cellId",  "rnti"]) #(4644x126)

# <-------------------

col_2del = ['1372', '3658', '5565', '6225', '1821', '2586', '2988', '8890', '9806']
rawDf_qoeDf = pd.read_feather("megaQoE_Df.feather") #(4644x6)
rawDf_qoeDf = rawDf_qoeDf[~rawDf_qoeDf.tag.isin(col_2del)]
rawDf_qoeDf["node"] = rawDf_qoeDf["node"]+1

lacked = grouped[grouped.merge(rawDf_qoeDf, 
                               on=["scenario","tag", "node"], 
                               how='left', indicator=True)._merge=="left_only"]
lacked = lacked[["scenario","tag", "node"]]                               
new_rawDf_qoeDf = pd.concat([rawDf_qoeDf, lacked], ignore_index=True).fillna(0)

grouped = grouped.merge(new_rawDf_qoeDf, on=["scenario","tag", "node"]) #(4641x129)


grouped.reset_index().to_feather("rawDf_dashing_dataset.feather")
grouped.reset_index().to_csv("rawDf_dashing_dataset.csv")
rawDf_dashing_dataset = pd.read_feather ("rawDf_dashing_dataset.feather")#4644x129
# rawDf_dashing_dataset = rawDf_dashing_dataset.drop("level_0", axis=1) #(4644x126)


