from myutils import *

# PHY  --------------------------------------------------------
groupedDf_sinrDf = pd.read_feather("groupedDf_sinrDf.feather")
groupedDf_dlPathLossDf = pd.read_feather("groupedDf_dlPathLossDf.feather") #660
groupedDf_ulPathLossDf = pd.read_feather("groupedDf_ulPathLossDf.feather") #660
groupedDf_ulRxPacketDf = pd.read_feather("groupedDf_ulRxPacketDf.feather") #660
groupedDf_dlRxPacketDf = pd.read_feather("groupedDf_dlRxPacketDf.feather") #660
# ------------------------------------------------------------------------

# MAC  --------------------------------------------------------
groupedDf_nrMacDf = pd.read_feather("groupedDf_nrMacDf.feather") #660x7
groupedDf_rxedUeMacDf = pd.read_feather ("groupedDf_rxedUeMacDf.feather")#660x12
groupedDf_txedUeMacDf = pd.read_feather ("groupedDf_txedUeMacDf.feather")#660x12
groupedDf_rxedPhyUeDf = pd.read_feather ("groupedDf_rxedPhyUeDf.feather")#660x12
groupedDf_txedPhyUeDf = pd.read_feather ("groupedDf_txedPhyUeDf.feather")#660x12
# ------------------------------------------------------------------------

# RLC --------------------------------------------------------
groupedDf_dlRlcDf = pd.read_feather ("groupedDf_dlRlcDf.feather")#660
groupedDf_ulRlcDf = pd.read_feather ("groupedDf_ulRlcDf.feather")#660
# ------------------------------------------------------------------------

# PDCP --------------------------------------------------------
groupedDf_dlPdcpDf = pd.read_feather ("groupedDf_dlPdcpDf.feather")#660
groupedDf_ulPdcpDf = pd.read_feather ("groupedDf_ulPdcpDf.feather")#660
# ------------------------------------------------------------------------

# IP   ------------------------------------------------------
groupedDf_ipDlDf = pd.read_feather("groupedDf_ipDlDf.feather")#660 
groupedDf_ipUlDf = pd.read_feather("groupedDf_ipUlDf.feather")#660 
# -----------------------------------------------------------

# TCP  --------------------------------------------------------
groupedDf_tcpDf = pd.read_feather("groupedDf_tcpMetaDf.feather")#21
# ---------------------------------------------------------------

# DASH   ------------------------------------------------------------
groupedDf_dashV1 = pd.read_feather("groupedDf_dashV1.feather")#600
# ---------------------------------------------------------------

# Scenario   ------------------------------------------------------------
groupedDf_scenarioDf = pd.read_feather("groupedDf_scenarioDf.feather")#65
# ---------------------------------------------------------------





###########################################
# Time to collect the allinone grouped df #
###########################################

# DASH 
groupedDf_dashV1["node"] = groupedDf_dashV1["node"] + 1
groupedDf_dashV1 = groupedDf_dashV1.drop("index", axis=1) #(660x17)
grouped = groupedDf_dashV1 #(660x17colz)
# DASH|Scenario 
groupedDf_scenarioDf = groupedDf_scenarioDf.drop("index", axis=1) #(10x3)
grouped = grouped.merge(groupedDf_scenarioDf, on="scenario") #(660x18)
# DASH|Scenario|IP 
groupedDf_ipUlDf = groupedDf_ipUlDf.drop("index", axis=1) #(660x12)
grouped = grouped.merge(groupedDf_ipUlDf, on=["scenario", "node"]) #(660x28)??
# DASH|Scenario|IP|PDCP 
groupedDf_ulPdcpDf = groupedDf_ulPdcpDf.drop("index", axis=1)
groupedDf_ulPdcpDf["node"] = groupedDf_ulPdcpDf.IMSI #(660x15)
grouped = grouped.merge(groupedDf_ulPdcpDf, on=["scenario","node"]) #(660x41)
# DASH|Scenario|IP|PDCP|RLC 
groupedDf_ulRlcDf = groupedDf_ulRlcDf.drop("index", axis=1)
groupedDf_ulRlcDf["node"] = groupedDf_ulRlcDf.IMSI #(660x15)
grouped = grouped.merge(groupedDf_ulRlcDf, on=["scenario","node"]) #(660x54)
# ------------------------------------------------------------------------
# DASH|Scenario|IP|PDCP|RLC|MAC(nrMac)
groupedDf_nrMacDf = groupedDf_nrMacDf.drop("index", axis=1) 
groupedDf_nrMacDf["node"] = groupedDf_nrMacDf.IMSI #(660x7)
grouped = grouped.merge(groupedDf_nrMacDf, on=["scenario","node"]) #(660x59)
# ------------------------------------------------------------------------
# DASH|Scenario|IP|PDCP|RLC|MAC(nrMac)|PHY
grouped = grouped.rename(columns={'RNTI': 'rnti'})
grouped = grouped.merge(groupedDf_sinrDf, on=["scenario","cellId",  "rnti"]) #(660x61)

groupedDf_dlPathLossDf = groupedDf_dlPathLossDf.drop("index", axis=1) #(660x3)
grouped = grouped.merge(groupedDf_dlPathLossDf, on=["scenario","IMSI"]) #(660x62)

groupedDf_ulPathLossDf = groupedDf_ulPathLossDf.drop("index", axis=1) #(660x3)
grouped = grouped.merge(groupedDf_ulPathLossDf, on=["scenario","IMSI"]) #(660x63)

groupedDf_ulRxPacketDf = groupedDf_ulRxPacketDf.drop(["level_0", "index"], axis=1) #(660x10)
grouped = grouped.merge(groupedDf_ulRxPacketDf, on=["scenario","cellId",  "rnti"]) #(660x70)

groupedDf_dlRxPacketDf = groupedDf_dlRxPacketDf.drop(["level_0", "index"], axis=1) #(660x9)
grouped = grouped.merge(groupedDf_dlRxPacketDf, on=["scenario","cellId",  "rnti"]) #(660x76)
# ------------------------------------------------------------------------
# DASH|Scenario|IP|PDCP|RLC|MAC(nrMac)|PHY|MAC(remaining)
groupedDf_rxedUeMacDf = pd.read_feather ("groupedDf_rxedUeMacDf.feather")#prob 120x15
newcolz = {col: col+"_rxMac" for col in groupedDf_rxedUeMacDf.columns[4:]}                                                        
groupedDf_rxedUeMacDf = groupedDf_rxedUeMacDf.rename(columns=newcolz)
groupedDf_rxedUeMacDf = groupedDf_rxedUeMacDf.rename(columns={'RNTI': 'rnti'}).drop("index", axis=1)
grouped = grouped.merge(groupedDf_rxedUeMacDf, on=["scenario","cellId",  "rnti"]) #(660x88)

newcolz = {col: col+"_txMac" for col in groupedDf_txedUeMacDf.columns[4:]}                                                        
groupedDf_txedUeMacDf = groupedDf_txedUeMacDf.rename(columns=newcolz)
groupedDf_txedUeMacDf = groupedDf_txedUeMacDf.rename(columns={'RNTI': 'rnti'}).drop("index", axis=1)
grouped = grouped.merge(groupedDf_txedUeMacDf, on=["scenario","cellId",  "rnti"]) #(660x100)


newcolz = {col: col+"_rxPhy" for col in groupedDf_rxedPhyUeDf.columns[4:]}                                                        
groupedDf_rxedPhyUeDf = groupedDf_rxedPhyUeDf.rename(columns=newcolz)
groupedDf_rxedPhyUeDf = groupedDf_rxedPhyUeDf.rename(columns={'RNTI': 'rnti'}).drop("index", axis=1)
grouped = grouped.merge(groupedDf_rxedPhyUeDf, on=["scenario","cellId",  "rnti"]) #(660x112)


newcolz = {col: col+"_txPhy" for col in groupedDf_txedPhyUeDf.columns[4:]}                                                        
groupedDf_txedPhyUeDf = groupedDf_txedPhyUeDf.rename(columns=newcolz)
groupedDf_txedPhyUeDf = groupedDf_txedPhyUeDf.rename(columns={'RNTI': 'rnti'}).drop("index", axis=1)
grouped = grouped.merge(groupedDf_txedPhyUeDf, on=["scenario","cellId",  "rnti"]) #(660x124)


grouped.reset_index().to_feather("groupedDf_dashing_dataset.feather")
groupedDf_dashing_dataset = pd.read_feather ("groupedDf_dashing_dataset.feather")#660x125
groupedDf_dashing_dataset = groupedDf_dashing_dataset.drop("level_0", axis=1) #(660x124)