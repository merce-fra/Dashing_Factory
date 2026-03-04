from myutils import *



# +++++++ #
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

# 'RTT', 
# 'RTO', 
# 'congestion_window', 
# 'ecnState', 
# 'RWND', 
# 'highest_sequence', 
#  'pacing_rat', 'inflated_congestion_window', 'AdvWND', 'congestion_state', 
#  'slow_start_threshold', 'next_tx_sequence', 'highest_rx_sequence', 'bytes_inflight']

# -----------------------------------------------------------------------------------------------------------------------


tcpColumns   = [ "timeStamp", "destination", "orignNode", "oldValue", "newValue"]
metrics = ['RTT', 'RTO', 'congestion_window', 'ecnState', 'RWND', 'highest_sequence', 
           'pacing_rat', 'inflated_congestion_window', 'AdvWND', 'congestion_state', 
           'slow_start_threshold', 'next_tx_sequence', 'highest_rx_sequence', 'bytes_inflight']

# ---------------------------
# Collect stats for one metric
#----------------------------
def tcp_stats_for (metric) :
    littleDf = pd.DataFrame ()
    file = "tcp_"+metric+".csv"
    meta_directories = [d for d in os.listdir("./") if os.path.isdir(d)] 
    for current_dir in meta_directories : 
    
        directories=[d for d in os.listdir(current_dir+"/") if os.path.isdir(os.path.join(current_dir, d))] 
        for directory in directories :                                              
                df = pd.read_csv(current_dir + "/"+ directory + "/" + file, names=tcpColumns, skiprows=1, index_col=False)                
                dff = df.groupby(["orignNode"]).size().reset_index(name=metric+"_changes")
                dff[metric] = df.groupby(["orignNode"])["newValue"].mean().reset_index()["newValue"]
                dff ["tag"] = directory; dff ["scenario"] = current_dir 
                dff = dff[["scenario", "tag", "orignNode", metric+"_changes", metric]]  
                littleDf = littleDf.append(dff)        
    return littleDf   # ---------------------------
# ---------------------------



# -------------------------------------
tcpMetaDf =  tcp_stats_for (metrics[0])
for metric in metrics[1:] :
    print (metric," ---> ", len(tcp_stats_for (metric)), end="")
    if (len(tcp_stats_for (metric))) : 
        print ("    included ...")
        tcpMetaDf = tcpMetaDf.merge(tcp_stats_for (metric), on=["scenario", "tag", "orignNode"], how="outer")
    else :
        print ("    not included ...")
reduce_memory_usage (tcpMetaDf)        
tcpMetaDf.reset_index().to_feather("tcpMetaDf.feather")        
#       ------------------------------



# -------------------------------
#   Another form of groupby :  
# -------------------------------
tcpMetaDf = pd.read_feather("tcpMetaDf.feather")
targetz = ['RTT_changes', 'RTT', 'RTO_changes',
           'RTO', 'congestion_window_changes', 'congestion_window', 'RWND_changes',
           'RWND', 'highest_sequence_changes', 'highest_sequence',
           'inflated_congestion_window_changes', 'inflated_congestion_window',
           'AdvWND_changes', 'AdvWND', 'slow_start_threshold_changes',
           'slow_start_threshold', 'next_tx_sequence_changes', 'next_tx_sequence',
           'highest_rx_sequence_changes', 'highest_rx_sequence',
           'bytes_inflight_changes', 'bytes_inflight']

groupedDf = tcpMetaDf.groupby(["scenario","orignNode"])[targetz].mean().reset_index()
reduce_memory_usage (groupedDf)
groupedDf.reset_index().to_feather("groupedDf_tcpDf.feather")
groupedDf_tcpDf = pd.read_feather("groupedDf_tcpDf.feather") 
# ---------------------------------------------------------------

