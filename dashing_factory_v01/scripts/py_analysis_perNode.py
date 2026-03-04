
from myutils import *


meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
for current_dir in meta_directories : 
    # directories  = os.listdir(current_dir+"/")  
    directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))] 
    superDf = pd.DataFrame (columns=["scenario", "tag"])
    tagz = []    
    scenarioz = []    




    # ++++++++ #
    #   DASH   #
    #   STUFF  #
    # ++++++++ #

    # -----------
    # DashBySeg
    # -----------

    megaDashBySegColumns = ["timestamp","segmentId","frameId","node","playedFrames","requestTime","segmentFetchTime",
                            "segmentSize","bufferTime","deltaBufferTime","estAvgBufferTime","newBitRate","oldBitRate",
                            "changes","estBitRate","segmentRate","interruptions","interruptionTime","queueLength","queueSize",
                            "initBuffer"]  
    targetzBySeg = ["playedFrames","requestTime","segmentFetchTime", "segmentSize","bufferTime","deltaBufferTime",
                    "estAvgBufferTime","newBitRate","oldBitRate", "changes","estBitRate","segmentRate","interruptions",
                    "interruptionTime","queueLength","queueSize", "initBuffer"]                                     
    # inputzBySeg  =  initialize_dict_from_list (initialize_col (targetzBySeg, functionz))                           
    inputzBySeg  =  initialize_dict_from_list (targetzBySeg)                           
    for directory in directories :                                                              
        bySegFiles  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'dashSegmentStatsByNode_.*\.csv', f)]                          
        if (len(bySegFiles)) : 
            megaDashBySegDf = pd.read_csv(current_dir + "/" + directory +"/"+bySegFiles[0], names=megaDashBySegColumns, skiprows=1, index_col=False)
            for file in bySegFiles [1:] :
                littleDf = pd.read_csv(current_dir + "/" + directory +"/"+file, names=megaDashBySegColumns, skiprows=1, index_col=False)
                megaDashBySegDf = megaDashBySegDf.append(littleDf)   
        # clean the stuff : metrics with sec,             
        targetzBySeg_s = ["requestTime", "segmentFetchTime", "bufferTime", "deltaBufferTime", "estAvgBufferTime", 
                          "interruptionTime", "initBuffer"] 
        megaDashBySegDf[targetzBySeg_s] = megaDashBySegDf[targetzBySeg_s].apply(lambda x: x.apply(lambda y: float(y[:-1])), axis=1)
        targetzBySeg_KB = ["segmentSize", "queueSize"]
        megaDashBySegDf[targetzBySeg_KB] = megaDashBySegDf[targetzBySeg_KB].apply(lambda x: x.apply(lambda y: float(y[:-2])), axis=1)
        targetzBySeg_kbps = ["newBitRate", "oldBitRate", "estBitRate", "segmentRate"]
        megaDashBySegDf[targetzBySeg_kbps] = megaDashBySegDf[targetzBySeg_kbps].apply(lambda x: x.apply(lambda y: float(y[:-4])), axis=1)   
        
        # Cols with means
        inputzBySeg = enrich_dict (targetzBySeg, functionz, megaDashBySegDf, inputzBySeg, use_functionz = False)


        # inputzBySeg  = enrich_dict (targetzBySeg, functionz, megaDashBySegDf, inputzBySeg) 
    super_inputz    = inputzBySeg.copy()        
    super_inputz = dict(super_inputz, **inputzBySeg) 
    superDf = pd.DataFrame(super_inputz)
    superDf["tag"] = tagz
    superDf["scenario"] = scenarioz
    superDf.to_csv("superDf_"+current_dir[:2]+"nodes.csv", index=False)                     

        
    # plt.clf();plt.plot(megaDashBySegDf[megaDashBySegDf.node==1].segmentId, megaDashBySegDf[megaDashBySegDf.node==1].bufferTime); 
    # plt.title("Buffering Time by segment received"); plt.savefig("bufferTime.jpg"); plt.show() 






























































    # +++++++
    #  PHY
    #  STUFF 
    # +++++++

    # ----------------------------------------------------
    # rxPacket Trace : Get  tbSize, mcs, rv, SINR_dB, CQI 
    # ----------------------------------------------------
    # In "UL" --> TBler is always giving "nan" TBler_UL_mean/TBler_UL_median/TBler_UL_var
    rxPacketColumns = ["Time" ,"direction" ,"frame" ,"subF" ,"slot" ,"1stSym" ,"nSymbol" ,"cellId" ,
                       "bwpId" ,"streamId" ,"rnti" ,"tbSize" ,"mcs","rv" ,"SINR_dB" ,"CQI" ,"corrupt" ,"TBler"]
    targets    =      ["tbSize",  "mcs",  "rv",   "SINR_dB",  "CQI",  "corrupt",  "TBler"]
    directionz =      ["UL", "DL"]
    functionz  =      ["mean", "median", "var"]
    inputz     = initialize_dict_from_list (initialize_col_with_dir (targets, directionz, functionz))
    for directory in directories :
        tagz.append(directory)
        scenarioz.append(current_dir)
        rxPacketDf = pd.read_csv(current_dir+"/"+directory+"/RxPacketTrace.txt", sep="\t", names=rxPacketColumns, skiprows=1)
        inputz = enrich_dict_with_dir (targets, directionz, functionz, rxPacketDf, inputz)
    # ------------------------------------------------------------------------------------


    # ---------------------------------------------------------------------------------------------------------
    # PathLoss Ul/Dl
    # ----------------------------------------------------------------------------------------
    super_inputz    = inputz.copy()
    ulPathlossColumns = ["Time", "CellId", "BwpId", "txStreamId",	"IMSI",	"rxStreamId", "ul_pathLoss_dB"]   
    dlPathlossColumns = ["Time", "CellId", "BwpId", "txStreamId",	"IMSI",	"rxStreamId", "dl_pathLoss_dB"]   
    targets         = ["ul_pathLoss_dB"]                     
    inputz          = initialize_dict_from_list (initialize_col (targets, functionz))
    for directory in directories :
        pathlossUlDf = pd.read_csv(current_dir +"/"+ directory +"/UlPathlossTrace.txt", sep="\t", names=ulPathlossColumns, skiprows=1)
        inputz = enrich_dict (targets, functionz, pathlossUlDf, inputz)

    targets         = ["dl_pathLoss_dB"]                     
    inputz          = initialize_dict_from_list (initialize_col (targets, functionz))
    for directory in directories :
        pathlossDlDf = pd.read_csv(current_dir +"/"+ directory +"/"+ "/DlPathlossTrace.txt", sep="\t", names=dlPathlossColumns, skiprows=1)
        inputz = enrich_dict (targets, functionz, pathlossDlDf, inputz)  
    super_inputz = dict(super_inputz, **inputz)      
    # -----------------------------------------------------------------


    # -----------------------------------------------------
    # Get MAC Stats Ul/Dl : harqId  ndi  rv  mcs  tbSize
    # -----------------------------------------------------------------------------------------------
    dlMacColumns = ["time",	"cellId", "bwpId", "IMSI", "RNTI", "frame", "sframe", "slot", "symStart", 
                    "numSym", "stream", "harqId", "ndi", "rv", "dl_mac_mcs", "dl_mac_TbSize"]
    targets      = ["dl_mac_mcs", "dl_mac_TbSize"]                     
    inputz       = initialize_dict_from_list (initialize_col (targets, functionz))
    for directory in directories :
        macDlDf = pd.read_csv(current_dir +"/"+ directory +"/"+ "/NrDlMacStats.txt", sep="\t", names=dlMacColumns, skiprows=1)
        inputz  = enrich_dict (targets, functionz, macDlDf, inputz)    
    ulMacColumns = ["time",	"cellId", "bwpId", "IMSI", "RNTI", "frame", "sframe", "slot", "symStart", 
                    "numSym", "stream", "harqId", "ndi", "rv", "ul_mac_mcs", "ul_mac_TbSize"]
    targets      = ["ul_mac_mcs", "ul_mac_TbSize"]      
    inputz       = initialize_dict_from_list (initialize_col (targets, functionz))                   
    for directory in directories :
        macUlDf = pd.read_csv(current_dir +"/"+ directory + "/NrUlMacStats.txt", sep="\t", names=ulMacColumns, skiprows=1)
        inputz  = enrich_dict (targets, functionz, macUlDf, inputz)    
    super_inputz = dict(super_inputz, **inputz)    
    # --------------------------------------------------------------

    # -----------------------
    # Get PDCP Stats Ul/Dl : 
    # ----------------------------------------------
    #                       Focus just on e2e delay
    #             cellId :  CellId of the attached Enb
    #               imsi :  IMSI of the UE who received the PDU
    #               rnti :  C-RNTI of the UE who received the PDU
    #               lcid :  LCID through which the PDU has been received
    #            pckSize :  size of the PDU in bytes
    #              delay :  RLC to RLC delay in nanoseconds 
    #                       (statistics average, min, max and standard deviation in second)               
    #                        nr-bearer-stats-calculator.cc line 330
    # --------------------------------------------------------------
    dl_pdcpColumns = ["time",	"end_s", "cellId", "IMSI", "RNTI", "LCID"	, "nTxPDUs"	, "dl_pdcp_TxBytes"	, 
                    "dl_pdcp_nRxPDUs", "dl_pdcp_RxBytes", "dl_pdcp_delay_s", "dl_pdcp_stdDev_s", "dl_pdcp_min_s", 
                    "dl_pdcp_max_s", "dl_pdcp_PduSize", "dl_pdcp_stdDev",	"dl_pdcp_min",	"dl_pdcp_max"]
    targets        = ["dl_pdcp_TxBytes", "dl_pdcp_nRxPDUs","dl_pdcp_RxBytes","dl_pdcp_delay_s","dl_pdcp_stdDev_s","dl_pdcp_min_s","dl_pdcp_max_s",
                    "dl_pdcp_PduSize","dl_pdcp_stdDev","dl_pdcp_min","dl_pdcp_max"]              
    inputz         = initialize_dict_from_list (initialize_col (targets, functionz))
    for directory in directories :
        pdcpDlDf = pd.read_csv(current_dir +"/"+ directory + "/NrDlPdcpStatsE2E.txt", sep="\t", names=dl_pdcpColumns, skiprows=1)
        inputz  = enrich_dict (targets, functionz, pdcpDlDf, inputz)    
    ul_pdcpColumns = ["time",	"end_s", "cellId", "IMSI", "RNTI", "LCID"	, "nTxPDUs"	, "ul_pdcp_TxBytes"	, 
                    "ul_pdcp_nRxPDUs", "ul_pdcp_RxBytes", "ul_pdcp_delay_s", "ul_pdcp_stdDev_s", "ul_pdcp_min_s", 
                    "ul_pdcp_max_s", "ul_pdcp_PduSize", "ul_pdcp_stdDev",	"ul_pdcp_min",	"ul_pdcp_max"]
    targets        = ["ul_pdcp_TxBytes", "ul_pdcp_nRxPDUs","ul_pdcp_RxBytes","ul_pdcp_delay_s","ul_pdcp_stdDev_s","ul_pdcp_min_s","ul_pdcp_max_s",
                    "ul_pdcp_PduSize","ul_pdcp_stdDev","ul_pdcp_min","ul_pdcp_max"]              
    inputz         = initialize_dict_from_list (initialize_col (targets, functionz))
    for directory in directories :
        pdcpUlDf = pd.read_csv(current_dir +"/"+ directory +"/NrUlPdcpStatsE2E.txt", sep="\t", names=ul_pdcpColumns, skiprows=1)
        inputz  = enrich_dict (targets, functionz, pdcpUlDf, inputz)  
    super_inputz = dict(super_inputz, **inputz)      
    # ------------------------------------------------------------------------------------------

    # -------------------
    # Get RLC Stats Ul/Dl : 
    # ------------------------------------------------------------------------------------------
    dl_rlcColumns = ["time",	"end_s", "cellId", "IMSI", "RNTI", "LCID"	, "dl_rlc_nTxPDUs"	, "dl_rlc_TxBytes"	, 
                    "dl_rlc_nRxPDUs", "dl_rlc_RxBytes", "dl_rlc_delay_s", "dl_rlc_stdDev_s", "dl_rlc_min_s", "dl_rlc_max_s", "dl_rlc_PduSize",	
                    "dl_rlc_stdDev",	"dl_rlc_min",	"dl_rlc_max"]
    targets       = ["dl_rlc_nTxPDUs"	, "dl_rlc_TxBytes", "dl_rlc_nRxPDUs", "dl_rlc_RxBytes", "dl_rlc_delay_s", 
                    "dl_rlc_stdDev_s", "dl_rlc_min_s", "dl_rlc_max_s", "dl_rlc_PduSize", "dl_rlc_stdDev",	"dl_rlc_min",	"dl_rlc_max"]                   
    inputz        =  initialize_dict_from_list (initialize_col (targets, functionz))

    for directory in directories :
        rlcDlDf = pd.read_csv(current_dir +"/"+ directory +"/"+ "NrDlRlcStatsE2E.txt", sep="\t", names=dl_rlcColumns, skiprows=1)
        inputz  = enrich_dict (targets, functionz, rlcDlDf, inputz) 

    ul_rlcColumns = ["time",	"end_s", "cellId", "IMSI", "RNTI", "LCID"	, "ul_rlc_nTxPDUs"	, "ul_rlc_TxBytes"	, 
                    "ul_rlc_nRxPDUs", "ul_rlc_RxBytes", "ul_rlc_delay_s", "ul_rlc_stdDev_s", "ul_rlc_min_s", "ul_rlc_max_s", "ul_rlc_PduSize",	
                    "ul_rlc_stdDev",	"ul_rlc_min",	"ul_rlc_max"]
    targets       = ["ul_rlc_nTxPDUs"	, "ul_rlc_TxBytes", "ul_rlc_nRxPDUs", "ul_rlc_RxBytes", "ul_rlc_delay_s", 
                    "ul_rlc_stdDev_s", "ul_rlc_min_s", "ul_rlc_max_s", "ul_rlc_PduSize", "ul_rlc_stdDev",	"ul_rlc_min",	"ul_rlc_max"]                   
    inputz        =  initialize_dict_from_list (initialize_col (targets, functionz))

    for directory in directories :
        rlcUlDf = pd.read_csv(current_dir +"/"+ directory +"/NrUlRlcStatsE2E.txt", sep="\t", names=ul_rlcColumns, skiprows=1)
        inputz  = enrich_dict (targets, functionz, rlcUlDf, inputz)   
    super_inputz = dict(super_inputz, **inputz)            
    # ----------------------------------------------------------------

    # -------------------
    # Get SINR Stats Ul/Dl : 
    # ----------------------------------------
    sinrColumns = ["cellId", "rnti", "sinr"]
    targets     = ["sinr"]                 
    inputz      =  initialize_dict_from_list (initialize_col (targets, functionz))
    for directory in directories :
        sinrDf = pd.read_csv(current_dir +"/"+ directory +"/sinrs.csv", sep="\t", names=sinrColumns, skiprows=1)
        inputz  = enrich_dict (targets, functionz, sinrDf, inputz) 
    super_inputz = dict(super_inputz, **inputz)    
    # --------------------------------------------------------------

    # -------------------
    # Get Rxed stuffs
    # -----------------------------------------------------------------------------------------------------
    # don't know important here as an info ?
    # rxedDciDlColumns = ["Time", "Entity", "Frame", "SF", "Slot", "nodeId", "RNTI", "bwpId", "HarqID", "K1"]
    # rxedDciDlDf = pd.read_csv(current_dir +"/"+ directory +"/"+ "RxedUePhyDlDciTrace.txt", sep="\t", names=rxedDciDlColumns, skiprows=1, index_col=False)
    # Ue Phy Dl Dci Trace.txt

    # 2) Gnb/Ue Mac Ctrl Msgs Trace ("VarTTI" no think is there)
    # 2-a)   rxedGnbMacDf.MsgType.value_counts ().index -->
    #    --> ['BSR', 'DL_HARQ', 'DL_CQI', 'SR', 'RACH_PREAMBLE']
    rxedColumns = ["Time", "Entity", "Frame", "SF", "Slot", "nodeId", "RNTI", "bwpId", "MsgType"] 
    targets     = ['BSR', 'DL_HARQ', 'DL_CQI', 'SR', 'RACH_PREAMBLE']
    col_list    = ["rxed_gnb_mac" + "_nb_"+target for target in targets]
    inputz      =  initialize_dict_from_list (col_list)
    for directory in directories :
        rxedGnbMacDf = pd.read_csv(current_dir +"/"+ directory +"/RxedGnbMacCtrlMsgsTrace.txt", sep="\t", names=rxedColumns, skiprows=1, index_col=False)
        inputz  = enrich_dict_with_val ("MsgType", rxedGnbMacDf, inputz, "rxed_gnb_mac") 

    # 2-b)  rxedUeMacDf.MsgType.value_counts ().index --> ['UL_DCI', 'RAR']
    targets     = ['UL_DCI', 'RAR']
    col_list    = ["rxed_ue_mac" + "_nb_"+target for target in targets]
    inputz      =  initialize_dict_from_list (col_list)
    for directory in directories :
        rxedUeMacDf = pd.read_csv(current_dir +"/"+ directory +"/RxedUeMacCtrlMsgsTrace.txt", sep="\t", names=rxedColumns, skiprows=1, index_col=False)
        inputz  = enrich_dict_with_val ("MsgType", rxedUeMacDf, inputz, "rxed_ue_mac") 

    # 3) Ue/Gnb Phy Ctrl Msgs 
    #    rxedPhyGnbDf = pd.read_csv(current_dir +"/"+ directory +"/"+ "RxedGnbPhyCtrlMsgsTrace.txt", sep="\t", names=rxedColumns, skiprows=1, index_col=False)
    #    rxedPhyUeDf =  pd.read_csv(current_dir +"/"+ directory +"/"+ "RxedUePhyCtrlMsgsTrace.txt", sep="\t", names=rxedColumns, skiprows=1, index_col=False)

    # 3-a)   rxedPhyGnbDf.MsgType.value_counts ().index -->
    #        --> ['DL_HARQ', 'DL_CQI', 'SRS', 'SR', 'RACH_PREAMBLE']
    rxedColumns = ["Time", "Entity", "Frame", "SF", "Slot", "nodeId", "RNTI", "bwpId", "MsgType"] 
    targets     = ['DL_HARQ', 'DL_CQI', 'SRS', 'SR', 'RACH_PREAMBLE']
    col_list    = ["rxed_gnb_phy" + "_nb_"+target for target in targets]
    inputz      =  initialize_dict_from_list (col_list)
    for directory in directories :
        rxedPhyGnbDf = pd.read_csv(current_dir +"/"+ directory +"/RxedGnbPhyCtrlMsgsTrace.txt", sep="\t", names=rxedColumns, skiprows=1, index_col=False)
        inputz  = enrich_dict_with_val ("MsgType", rxedPhyGnbDf, inputz, "rxed_gnb_phy") 

    # 3-b)   rxedPhyUeDf.MsgType.value_counts ().index -->
    #        --> ['UL_DCI', 'DL_DCI', 'MIB', 'SIB1', 'RAR']
    rxedColumns = ["Time", "Entity", "Frame", "SF", "Slot", "nodeId", "RNTI", "bwpId", "MsgType"] 
    targets     = ['UL_DCI', 'DL_DCI', 'MIB', 'SIB1', 'RAR']
    col_list    = ["rxed_ue_phy" + "_nb_"+target for target in targets]
    inputz      =  initialize_dict_from_list (col_list)
    for directory in directories :
        rxedPhyUeDf = pd.read_csv(current_dir +"/"+ directory +"/RxedUePhyCtrlMsgsTrace.txt", sep="\t", names=rxedColumns, skiprows=1, index_col=False)
        inputz  = enrich_dict_with_val ("MsgType", rxedPhyUeDf, inputz, "rxed_ue_phy") 
    super_inputz = dict(super_inputz, **inputz)    
    # ---------------------------------------------------------------------------------



    # -------------------
    # Get Txed stuffs
    # ----------------------------------------------------------------------------------------------------

    txedColumns = ["Time",	"Entity", "Frame", "SF", "Slot", "nodeId", "RNTI", "bwpId", "MsgType"]

    # 1)   Txed Gnb/Ue Mac Ctrl Msgs 
    # 1-a) txedGnbMacDf.MsgType.value_counts ().index --> ['RAR']
    targets     = ["RAR"]
    col_list    = ["txed_gnb_mac" + "_nb_"+target for target in targets]
    inputz      =  initialize_dict_from_list (col_list)
    for directory in directories :
        txedGnbMacDf = pd.read_csv(current_dir +"/"+ directory +"/TxedGnbMacCtrlMsgsTrace.txt", sep="\t", names=txedColumns, skiprows=1, index_col=False)
        inputz  = enrich_dict_with_val ("MsgType", txedGnbMacDf, inputz, "txed_gnb_mac") 
    # 1-b) txedUeMacDf.MsgType.value_counts ().index --> ['BSR', 'SR', 'RACH_PREAMBLE']
    targets     = ['BSR', 'SR', 'RACH_PREAMBLE']
    col_list    = ["txed_ue_mac" + "_nb_"+target for target in targets]
    inputz      =  initialize_dict_from_list (col_list)
    for directory in directories :
        txedUeMacDf = pd.read_csv(current_dir +"/"+ directory +"/TxedUeMacCtrlMsgsTrace.txt", sep="\t", names=txedColumns, skiprows=1, index_col=False)
        inputz  = enrich_dict_with_val ("MsgType", txedUeMacDf, inputz, "txed_ue_mac")

    # 2-a) txedGnbPhyDf.MsgType.value_counts ().index --> ['UL_UCI', 'DL_DCI', 'MIB', 'SIB1', 'RAR']
    targets     = ['UL_UCI', 'DL_DCI', 'MIB', 'SIB1', 'RAR']
    col_list    = ["txed_gnb_phy" + "_nb_"+target for target in targets]
    inputz      =  initialize_dict_from_list (col_list)
    for directory in directories :
        txedGnbPhyDf = pd.read_csv(current_dir +"/"+ directory +"/"+ "TxedGnbPhyCtrlMsgsTrace.txt", sep="\t", names=txedColumns, skiprows=1, index_col=False)
        inputz  = enrich_dict_with_val ("MsgType", txedGnbPhyDf, inputz, "txed_gnb_phy") 
    # 2-b) txedUePhyDf.MsgType.value_counts ().index --> ['DL_HARQ', 'DL_CQI', 'SRS', 'SR', 'RACH_PREAMBLE']
    targets     = ['DL_HARQ', 'DL_CQI', 'SRS', 'SR', 'RACH_PREAMBLE']
    col_list    = ["txed_ue_phy" + "_nb_"+target for target in targets]
    inputz      =  initialize_dict_from_list (col_list)
    for directory in directories :
        txedUePhyDf = pd.read_csv(current_dir +"/"+ directory +"/TxedUePhyCtrlMsgsTrace.txt", sep="\t", names=txedColumns, skiprows=1, index_col=False)
        inputz  = enrich_dict_with_val ("MsgType", txedUePhyDf, inputz, "txed_ue_phy")
    super_inputz = dict(super_inputz, **inputz)    


    # +++++++ #
    #    IP   #
    #  STUFF  #
    # +++++++ # --------------------------------------------------------------------------------------------------------------
    # directory   = "./ip/" 
    ipColumns = ["flowId","sourceNode","sourceNodePosition","sourceAddress","sourcePort","destAddress","destPort",
                "protocol","dl_txPackets","dl_txBytes","dl_txOffered","dl_xBytes","dl_rxOfferedthroughput","dl_meanDelay","dl_meanJitter",
                "dl_rxPackets","dl_lostPackets","dl_lostPacketsRatio"]
    targets   = ["dl_txPackets","dl_txBytes","dl_txOffered","dl_xBytes","dl_rxOfferedthroughput","dl_meanDelay","dl_meanJitter",
                "dl_rxPackets","dl_lostPackets","dl_lostPacketsRatio"]
    inputz    =  initialize_dict_from_list (initialize_col (targets, functionz))

    for directory in directories :
        dlDf = pd.read_csv(current_dir +"/"+ directory +"/dlFlows.csv", names=ipColumns, skiprows=1, index_col=False)
        # Unable to calculate mean/median/var if we conserve the unites, i.e. Mbps, ms
        # concerned columns : Mbps : "dl_txOffered", "dl_rxOfferedthroughput", 
        #                     ms   ; "dl_meanDelay", "dl_meanJitter"
        dlDf ["dl_txOffered"]           = dlDf ["dl_txOffered"].apply(lambda x: float(x[:-4]))
        dlDf ["dl_rxOfferedthroughput"] = dlDf ["dl_rxOfferedthroughput"].apply(lambda x: float(x[:-4]))
        dlDf ["dl_meanDelay"]  =  dlDf ["dl_meanDelay"].apply(lambda x: float(x[:-2]))
        dlDf ["dl_meanJitter"] = dlDf ["dl_meanJitter"].apply(lambda x: float(x[:-2]))
        inputz  = enrich_dict (targets, functionz, dlDf, inputz)     

    ipColumns = ["flowId","sourceNode","sourceNodePosition","sourceAddress","sourcePort","destAddress","destPort",
                "protocol","ul_txPackets","ul_txBytes","ul_txOffered","ul_xBytes","ul_rxOfferedthroughput","ul_meanDelay","ul_meanJitter",
                "ul_rxPackets","ul_lostPackets","ul_lostPacketsRatio"]
    targets   = ["ul_txPackets","ul_txBytes","ul_txOffered","ul_xBytes","ul_rxOfferedthroughput","ul_meanDelay","ul_meanJitter",
                "ul_rxPackets","ul_lostPackets","ul_lostPacketsRatio"]
    inputz    =  initialize_dict_from_list (initialize_col (targets, functionz))
    for directory in directories :
        ulDf = pd.read_csv(current_dir +"/"+ directory +"/ulFlows.csv", names=ipColumns, skiprows=1, index_col=False)
        ulDf ["ul_txOffered"]           = ulDf ["ul_txOffered"].apply(lambda x: float(x[:-4]))
        ulDf ["ul_rxOfferedthroughput"] = ulDf ["ul_rxOfferedthroughput"].apply(lambda x: float(x[:-4]))
        ulDf ["ul_meanDelay"] =  ulDf ["ul_meanDelay"].apply(lambda x: float(x[:-2]))
        ulDf ["ul_meanJitter"] = ulDf ["ul_meanJitter"].apply(lambda x: float(x[:-2]))    
        inputz  = enrich_dict (targets, functionz, ulDf, inputz)   
    super_inputz = dict(super_inputz, **inputz)      
    # -----------------------------------------------------------------------------------------


    #   TCP   #
    #  STUFF  #
    # +++++++ # -----------------------------------------------------
    # tcp_AdvWND.csv             tcp_inflated_congestion_window.csv
    # tcp_bytes_inflight.csv     tCP_next_tx_sequence.csv
    # tcp_congestion_state.csv   tcp_pacing_rat.csv
    # tcp_congestion_window.csv  tcp_RTO.csv
    # tcp_ecnState.csv           tcp_RTT.csv
    # tcp_highest_sequence.csv   tcp_RWND.csv
    # tcp_high_tx_mark.csv       tcp_slow_start_threshold.csv
    # -----------------------------------------------------------------------------------------------------------------------
    # directory   = "./tcp/" 
    tcpColumns   = [ "timeStamp", "destination", "orignNode", "oldValue", "newValue"]
    # targetz      = [ "tcpBytes", "tcpCongWin", "tcpHighestSeq", "tcpHighTx", "tcpInflatedCong", "tcpNextTx", 
    #                 "tcpRto", "tcpRtt", "tcpRwnd", "tcpSlowStart" ]
    targetz = [ "PacingRate", "CongestionWindow", "CongestionWindowInflated", "SlowStartThreshold", 
                "CongState", "EcnState", "NextTxSequence", "HighestRxSequence", "HighestSequence", 
                "BytesInFlight", "RWND", "AdvWND", "RTT", "RTO"]
    inputz       = initialize_dict_from_list (initialize_col (targetz, functionz))

    for directory in directories :   

        # tcpBytesDf         = pd.read_csv(current_dir +"/"+ directory + "/tcp_inflated_congestion_window.csv", names=tcpColumns, skiprows=1, index_col=False)
        # tcpCongWinDf       = pd.read_csv(current_dir +"/"+ directory + "/tcp_bytes_inflight.csv", names=tcpColumns, skiprows=1, index_col=False)
        # tcpHighestSeqDf    = pd.read_csv(current_dir +"/"+ directory + "/tcp_next_tx_sequence.csv", names=tcpColumns, skiprows=1, index_col=False)
        # tcpHighTxDf        = pd.read_csv(current_dir +"/"+ directory + "/tcp_congestion_window.csv", names=tcpColumns, skiprows=1, index_col=False)
        # tcpInflatedCongDf  = pd.read_csv(current_dir +"/"+ directory + "/tcp_RTO.csv", names=tcpColumns, skiprows=1, index_col=False)
        # tcpNextTxDf        = pd.read_csv(current_dir +"/"+ directory + "/tcp_RTT.csv", names=tcpColumns, skiprows=1, index_col=False)
        # tcpRtoDf           = pd.read_csv(current_dir +"/"+ directory + "/tcp_highest_sequence.csv", names=tcpColumns, skiprows=1, index_col=False)
        # tcpRttDf           = pd.read_csv(current_dir +"/"+ directory + "/tcp_RWND.csv", names=tcpColumns, skiprows=1, index_col=False)
        # tcpRwndDf          = pd.read_csv(current_dir +"/"+ directory + "/tcp_high_tx_mark.csv", names=tcpColumns, skiprows=1, index_col=False)
        # tcpSlowStartDf     = pd.read_csv(current_dir +"/"+ directory + "/tcp_slow_start_threshold.csv", names=tcpColumns, skiprows=1, index_col=False/)".csvt, names=tcpColumnscp_pacing_rat"

        tcpPacingRateDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_pacing_rat.csv", names=tcpColumns, skiprows=1, index_col=False)
        tcpCongestionWindowDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_congestion_window.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpCongestionWindowInflatedDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_inflated_congestion_window.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpSlowStartThresholdDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_slow_start_threshold.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpCongStateDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_congestion_state.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpEcnStateDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_ecnState.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpNextTxSequenceDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_next_tx_sequence.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpHighestRxSequenceDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_highest_rx_sequence.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpHighestSequenceDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_highest_sequence.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpBytesInFlightDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_bytes_inflight.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpRWNDDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_RWND.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpAdvWNDDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_AdvWND.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpRTTDf = pd.read_csv(current_dir + "/"+ directory + "/tcp_RTT.csv",   names=tcpColumns, skiprows=1, index_col=False)
        tcpRTODf = pd.read_csv(current_dir + "/"+ directory + "/tcp_RTO.csv", names=tcpColumns, skiprows=1, index_col=False)

        dfz = [ tcpPacingRateDf, tcpCongestionWindowDf, tcpCongestionWindowInflatedDf, tcpSlowStartThresholdDf, tcpCongStateDf, 
                tcpEcnStateDf, tcpNextTxSequenceDf, tcpHighestRxSequenceDf, tcpHighestSequenceDf, tcpBytesInFlightDf, tcpRWNDDf, 
                tcpAdvWNDDf, tcpRTTDf, tcpRTODf ]
                
        for i in range (len(dfz)) :
            dfz[i][targetz[i]] = dfz[i].newValue
            inputz  = enrich_dict ([targetz[i]], functionz, dfz[i], inputz) 
    super_inputz = dict(super_inputz, **inputz)                
    # -------------------------------------------------------------------













    # -----------
    # DashByNod
    # -----------
    megaDashByNodeColumns = ["timeStamp","Node","newBitRate","oldBitRate","estBitRate","interruptions","InterruptionTime",
                             "bufferTime","deltaBufferTime","estAvgBufferTime","delay_segmentTime","segmentRate","initBuffer"]      
    targetzByNode = ["newBitRate","oldBitRate","estBitRate","interruptions","InterruptionTime", "bufferTime","deltaBufferTime",
                    "estAvgBufferTime","delay_segmentTime","segmentRate","initBuffer"]                                      
    inputzByNode  =  initialize_dict_from_list (initialize_col (targetzByNode, functionz))                           
    for directory in directories :                                                              
        byNodeFiles  = [f for f in os.listdir(current_dir + "/"+ directory) if re.match(r'dashStatsByNode_.*\.csv', f)]                          
        if (len(byNodeFiles)) : 
            megaDashByNodeDf = pd.read_csv(current_dir + "/"+ directory+"/"+byNodeFiles[0], names=megaDashByNodeColumns, skiprows=1, index_col=False)
            for file in byNodeFiles [1:] :
                littleDf = pd.read_csv(current_dir + "/"+ directory+"/"+file, names=megaDashByNodeColumns, skiprows=1, index_col=False)
                megaDashByNodeDf = megaDashByNodeDf.append(littleDf)   
        
        # clean the stuff : metrics with sec,             
        targetzByNode_s = ["InterruptionTime", "bufferTime", "deltaBufferTime", "estAvgBufferTime", "delay_segmentTime", "initBuffer"]
        megaDashByNodeDf[targetzByNode_s] = megaDashByNodeDf[targetzByNode_s].apply(lambda x: x.apply(lambda y: float(y[:-1])), axis=1)
                                
        inputzByNode  = enrich_dict (targetzByNode, functionz, megaDashByNodeDf, inputzByNode) 
    super_inputz = dict(super_inputz, **inputzByNode)                
    # -----------------------------------------------


    # -----------
    # DashDf
    # -----------
    dashColumns = ["node","InterruptionTime","interruptions","framesPlayed","avgRateByFrame","avgRateBytes","maxRate",
                "minminRate","minRate", "avgRate","avgDt","estAvgBufferTime","changes","initBuffer","queueLength", 
                "queueLengthInBytes"]   
    targetzDash = ["InterruptionTime", "interruptions","framesPlayed","avgRateByFrame","avgRateBytes","maxRate", "minminRate","minRate", 
                "avgRate","avgDt","estAvgBufferTime","changes","initBuffer","queueLength", "queueLengthInBytes"]               
    inputzDash  =  initialize_dict_from_list (initialize_col (targetzDash, functionz))                           
    for directory in directories :  
        dashDf   = pd.read_csv(current_dir + "/" + directory + "/dashStats.csv", names=dashColumns, skiprows=1, index_col=False)
        targetzDf_s = ["InterruptionTime", "avgDt", "estAvgBufferTime", "initBuffer"] 
        dashDf[targetzDf_s] = dashDf[targetzDf_s].apply(lambda x: x.apply(lambda y: float(y[:-1])), axis=1)
        targetzDf_KB = ["queueLengthInBytes"]
        dashDf[targetzDf_KB] = dashDf[targetzDf_KB].apply(lambda x: x.apply(lambda y: float(y[:-2])), axis=1)
        targetzDf_kbps = ["avgRateBytes", "minminRate", "minRate", "avgRate", "avgRateByFrame", "maxRate"]
        dashDf[targetzDf_kbps] = dashDf[targetzDf_kbps].apply(lambda x: x.apply(lambda y: float(y[:-4])), axis=1)   
        inputzDash  = enrich_dict (targetzDash, functionz, dashDf, inputzDash)                                                             
    super_inputz = dict(super_inputz, **inputzDash)                
    # ====================================================================





































































    # ++++++++++++ #
    #   SCENARIO   #
    #    STUFF     #
    # ++++++++++++ # -----------------------------------------------------
    # distances.csv  gnb-positions.csv  recapStats.csv  ue-positions.csv
    # -----------------------------------------------------------------------------------------------------------------------
    recapScenarioColumns = ['start', 'duration', 'ul_meanThroughput', 'ul_meanDelay', 'ul_meanJitter', 'ul_meanLossRatio', 
                            'dl_meanThroughput','dl_meanDelay', 'dl_meanJitter', 'dl_meanLossRatio']
    targetScenario = ['duration', 'ul_meanThroughput', 'ul_meanDelay', 'ul_meanJitter', 'ul_meanLossRatio', 
                    'dl_meanThroughput','dl_meanDelay', 'dl_meanJitter', 'dl_meanLossRatio']
                
    # inputzScenario  =  initialize_dict_from_list (initialize_col (targetScenario, functionz))                           
    inputzScenario  =  initialize_dict_from_list (targetScenario)                          
    for directory in directories :  
        recapScenarioDf = pd.read_csv(current_dir + "/"+ directory + "/recapStats.csv",  names=recapScenarioColumns,  skiprows=1, index_col=False)                            
        # targetScenario_ms = ["ul_meanDelay", "ul_meanJitter", "dl_meanDelay", "dl_meanJitter"] 
        # recapScenarioDf[targetScenario_ms] = recapScenarioDf[targetScenario_ms].apply(lambda x: x.apply(lambda y: float(y[:-2])), axis=1)        
        # targetScenario_Mbps = ["ul_meanThroughput", "dl_meanThroughput"]
        # recapScenarioDf[targetScenario_Mbps] = recapScenarioDf[targetScenario_Mbps].apply(lambda x: x.apply(lambda y: float(y[:-4])), axis=1)   
        inputzScenario  = enrich_dict (targetScenario, functionz, recapScenarioDf, inputzScenario, False) 
        # inputzScenario  = enrich_dict_with_val ("MsgType", recapScenarioDf, inputzScenario, "txed_ue_mac")                                                                
    super_inputz = dict(super_inputz, **inputzScenario)                
    #--------------------------------------------------

    # ++++++++++++ #
    #   SCENARIO   #
    #    STUFF     #
    # ++++++++++++ # -----------------------------------------------------
    # distances.csv  gnb-positions.csv  recapStats.csv  ue-positions.csv
    # -----------------------------------------------------------------------------------------------------------------------
    # directory   = "./scenario/" 
    uePositionsColumns   = ["ue", "nodeId",  "imsi", "uex",  "uey", "uez"]
    gnbPositionsColumns  = ["gnb", "gnbId", "gnbx", "gnby", "gnbz"]         
    ueDistancesColumns   = ["nodeId", "distance", "gnbId"]         
    megaDistancesColumns = ['ue', 'nodeId', 'uex', 'uey', 'uez', 'distance', 'gnbId', 'gnb', 'gnbx', 'gnby', 'gnbz', 'tag', 'scenario']
    megaDistancesDf = pd.DataFrame (columns=megaDistancesColumns)
    for directory in directories :                            
        uePositionsDf   = pd.read_csv(current_dir + "/"+ directory + "/ue-positions.csv",  sep="\t", names=uePositionsColumns, index_col=False)
        gnbPositionsDf  = pd.read_csv(current_dir + "/"+ directory + "/gnb-positions.csv", sep="\t", names=gnbPositionsColumns,index_col=False)
        ueDistancesDf   = pd.read_csv(current_dir + "/"+ directory + "/distances.csv",  sep="\t", names=ueDistancesColumns, index_col=False)
        distancesDf = (uePositionsDf.merge(ueDistancesDf, on="nodeId")).merge (gnbPositionsDf, on="gnbId")
        distancesDf ["tag"] = current_dir + "/"+ directory
        distancesDf ["scenario"] = current_dir
        megaDistancesDf = megaDistancesDf.append (distancesDf)
        
        plt.clf()        
        plt.title("Topology")
        gnbz =  ["gnb"+str(index) for index in gnbPositionsDf.gnb]
        plt.scatter(gnbPositionsDf.gnbx+10, gnbPositionsDf.gnby+15, marker = "1", c="blue", s=80, label=gnbz)
        uez =  ["ue"+str(index) for index in uePositionsDf.ue]
        plt.scatter(uePositionsDf.uex+10, uePositionsDf.uey+15, marker="+", c="red", s=20, label=uez)
        for i in range (0, 120, 20) :
            plt.axline((i, 0), (i, 50), linestyle=":", linewidth=0.5, color='b')      
        plt.axline((0, 25), (120, 25), linestyle=":", linewidth=0.5, color='b')      
        plt.xlim (0, 120); plt.ylim (0, 50)
        plt.savefig(current_dir+"/topology"+directory+".png")

    plt.clf()        
    plt.title("Topology")
    gnbz =  ["gnb"+str(index) for index in megaDistancesDf.gnb]
    plt.scatter(megaDistancesDf.gnbx+10, megaDistancesDf.gnby+15, marker = "1", c="blue", s=80, label=gnbz)
    uez =  ["ue"+str(index) for index in megaDistancesDf.ue]
    plt.scatter(megaDistancesDf.uex+10, megaDistancesDf.uey+15, marker="+", c="red", s=20, label=uez)
    for i in range (0, 120, 20) :
        plt.axline((i, 0), (i, 50), linestyle=":", linewidth=0.5, color='b')      
    plt.axline((0, 25), (120, 25), linestyle=":", linewidth=0.5, color='b')      
    plt.xlim (0, 120); plt.ylim (0, 50)
    plt.savefig("allinone_topology_"+current_dir[:2]+"nodes.png")  

    superDf = pd.DataFrame(super_inputz)
    superDf["tag"] = tagz
    superDf["scenario"] = scenarioz
    superDf.to_csv("superDf_"+current_dir[:2]+"nodes.csv", index=False)  


