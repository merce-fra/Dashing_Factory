import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os, re, glob, math, sys
import numpy as np
from scipy.interpolate import interp1d
from scipy.interpolate import make_interp_spline
import matplotlib.dates as dates
from datetime import datetime

from mpl_toolkits.mplot3d import Axes3D
import matplotlib as mpl
from mpl_toolkits.axes_grid.inset_locator import inset_axes
from pandas.plotting._matplotlib.style import get_standard_colors
from typing import List, Union

from scipy.interpolate import griddata

# outdir = './dir'
# if not os.path.exists(outdir):
#     os.mkdir(outdir)




#------------------------------------
# UEs positions (all in one variant)
#------------------------------------
# read SINR data
# directories = [x[0] for x in os.walk(".")]
# sinrDf = pd.DataFrame(columns=["tag", "sinr"])
# for directory in directories :
#     tag = directory[8:]
#     files = [f for f in os.listdir(directory) if f=="sinrs.csv"]
#     if (len(files) == 1) : 
#         smallSinrDf = pd.read_csv(directory+"/"+"sinrs.csv", sep="\t", \
#                       names=['cellId', 'sinr'])
#         newrow = {"tag":tag, "sinr":smallSinrDf.sinr.mean()}
#         sinrDf = sinrDf.append(newrow, ignore_index=True)
# sinrDf = sinrDf.astype({'tag':'int'})  

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
                plt.text(index-0.3, ymax*1.05, str(ymax), color="red", fontsize=6)         
    
        plt.boxplot(y_values)
        plt.xticks(list(range(len(scenarioz)+1))[1:], list(scenarioz), rotation=20, fontsize=6)
        plt.ylabel(parameter+suffix_col_ip(parameter))
        plt.xlabel("Scenario (nb nodes)")
        if (title) :  plt.title(title+"\n ("+ parameter+")", fontsize=14)
        else : plt.title(parameter, fontsize=14)
        plt.savefig(label+"_"+parameter+"_plotboxVersion.jpg")
        if show : plt.show()
# -------------------------------------------------------



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


# ------------------------------------------
# Draw the mean, Max and Min values together
# in multiple plots
# ------------------
def see_df (df, scenario = "12nodes", tag="1251") :
    return df[(df.scenario==scenario) & (df.tag==tag)]
# -----------------------------------------------------    



# ------------------------------------------
# Draw the mean, Max and Min values together
# in multiple plots
# ------------------
def draw_plotbox_for_groupbyV (groupedDf, targetz, label="", title="", ymaxCoeff=1.05, xmaxCoeff=0.3, 
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
                #ymax = round(scenarioValues.median(), 3)
                plt.text(index-xmaxCoeff, ymax*ymaxCoeff, str(ymax), color="red", fontsize=6) 
    
        plt.boxplot(y_values)                                
        plt.xticks(list(range(len(scenarioz)+1))[1:], list(scenarioz), rotation=20, fontsize=6)
        plt.ylabel(parameter+suffix_col_ip(parameter))
        plt.xlabel("Scenario (nb nodes)")
        if (title) :  plt.title(title+"\n ("+ parameter+")", fontsize=14)
        else : plt.title(parameter, fontsize=14)
        plt.savefig(label+"_"+parameter+"_plotboxVersion.jpg")
        if show : plt.show()
# -------------------------------------------------------



dl_mbps = ["dl_txOffered", "dl_rxOfferedthroughput"] 
dl_ms =   ["dl_meanDelay", "dl_meanJitter"]
ul_mbps = ["ul_txOffered", "ul_rxOfferedthroughput"] 
ul_ms =   ["ul_meanDelay", "ul_meanJitter"]


# ===================
def suffix_col_ip (col):
    if ((col in dl_mbps) | (col in ul_mbps)) : return " (in Mbps)"
    if ((col in dl_ms) | (col in ul_ms)) : return " (in ms)"    
    return "" # ====================================
# =============



#   -----------------------------------------------------------
def enrich_dict_with_dir (targets, directionz, functionz, srcDf, input_dict) : 
    """ 
    " using directions ul, dl
    " example :
    " targets = ["tbSize",  "mcs",  "rv",   "SINR_dB",  "CQI",  "corrupt",  "TBler"]
    " directionz = ["UL", "DL"]
    " functionz = ["mean", "median", "var"]
    """
    for target in targets :
        for direction in directionz :
            for function in functionz : 
                column = target+"_"+direction+"_"+function
                input_dict [column].append (srcDf[srcDf.direction==direction][target].apply(function))                
    return input_dict
#   --------------


#   -----------------------------------------------------------
def enrich_dict_with_val (target_col, srcDf, input_dict, tag) :  
    """ 
    " using value count ...
    " ex mac/phy messages, etc
    """  
    targets = srcDf[target_col].value_counts ().index  
    for target in targets :
            column = tag + "_nb_"+target
            input_dict [column].append (len(srcDf[srcDf[target_col]==target]))                
    return input_dict
#   --------------


#   -----------------------------------------------------------
def enrich_dict (targets, functionz, srcDf, input_dict, use_functionz = True) : 
    for target in targets :
        if (use_functionz) : 
            for function in functionz : 
                column = target+"_"+function
                input_dict [column].append (srcDf[target].apply(function))
        else :
            column = target
            input_dict [column].append (srcDf[target].values[0])
    return input_dict
#   --------------


# ---------------------------------------------------
def initializeDict (targets, directionz, functionz) :
    output_dict = dict()
    for target in targets :
        for direction in directionz :
            for function in functionz : 
                column = target+"_"+direction+"_"+function
                output_dict[column] = []
    return output_dict
# ---------------------------------------------------

# ---------------------------------------------------
def initialize_dict_from_list (col_list) :
    return {col : [] for col in col_list}
# ---------------------------------------------------

# ------------------------------------------------------------
def initialize_col_with_dir (targets, directionz, functionz) :
    output_col_list = []
    for target in targets :
        for direction in directionz :
            for function in functionz : 
                column = target+"_"+direction+"_"+function
                output_col_list.append(column)
    return output_col_list
# ---------------------------------------------------


# ---------------------------------------------------------------
def initialize_col_with_val (target_col, srcDf, tag) :
    """ 
    " using value count ...
    """  
    return [tag + "_nb_"+target for target in srcDf[target_col].value_counts ().index]
# ------------------------------------------------------------------------------------    

# ---------------------------------------------------



# ------------------------------------------------------------
def initialize_col (targets, functionz) :
    output_col_list = []
    for target in targets :
        for function in functionz : 
            column = target+"_"+function
            output_col_list.append(column)
    return output_col_list
# ---------------------------------------------------




# ------------------------------------------------------------
#  handy but long function that casts floats and integers to their smallest subtype 
#  usage : >>> reduce_memory_usage(tps_october)
#          >>> Mem. usage decreased to 509.26 Mb (76.9% reduction)
#  c.f. https://pub.towardsai.net/6-pandas-mistakes-that-silently-tell-you-are-a-rookie-f075c91595e9
# ----------------------------------------
def reduce_memory_usage(df, verbose=True):
    numerics = ["int8", "int16", "int32", "int64", "float16", "float32", "float64"]
    start_mem = df.memory_usage().sum() / 1024 ** 2
    for col in df.columns:
        col_type = df[col].dtypes
        if col_type in numerics:
            c_min = df[col].min()
            c_max = df[col].max()
            if str(col_type)[:3] == "int":
                if c_min > np.iinfo(np.int8).min and c_max < np.iinfo(np.int8).max:
                    df[col] = df[col].astype(np.int8)
                elif c_min > np.iinfo(np.int16).min and c_max < np.iinfo(np.int16).max:
                    df[col] = df[col].astype(np.int16)
                elif c_min > np.iinfo(np.int32).min and c_max < np.iinfo(np.int32).max:
                    df[col] = df[col].astype(np.int32)
                elif c_min > np.iinfo(np.int64).min and c_max < np.iinfo(np.int64).max:
                    df[col] = df[col].astype(np.int64)
            else:
                if (
                    c_min > np.finfo(np.float16).min
                    and c_max < np.finfo(np.float16).max
                ):
                    df[col] = df[col].astype(np.float16)
                elif (
                    c_min > np.finfo(np.float32).min
                    and c_max < np.finfo(np.float32).max
                ):
                    df[col] = df[col].astype(np.float32)
                else:
                    df[col] = df[col].astype(np.float64)
    end_mem = df.memory_usage().sum() / 1024 ** 2
    if verbose:
        print(
            "Mem. usage decreased to {:.2f} Mb ({:.1f}% reduction)".format(
                end_mem, 100 * (start_mem - end_mem) / start_mem
            )
        )
    return df
# ------------------------------------------------------------



colors = ['aliceblue', 'antiquewhite', 'aqua', 'aquamarine', 'azure', 'beige', 'bisque', 'black', 
          'blanchedalmond', 'blue', 'blueviolet', 'brown', 'burlywood', 'cadetblue', 'chartreuse', 
          'chocolate', 'coral', 'cornflowerblue', 'cornsilk', 'crimson', 'cyan', 'darkblue', 
          'darkcyan', 'darkgoldenrod', 'darkgray', 'darkgreen', 'darkkhaki', 'darkmagenta', 
          'darkolivegreen', 'darkorange', 'darkorchid', 'darkred', 'darksalmon', 'darkseagreen', 
          'darkslateblue', 'darkslategray', 'darkturquoise', 'darkviolet', 'deeppink', 'deepskyblue', 
          'dimgray', 'dodgerblue', 'firebrick', 'floralwhite', 'forestgreen', 'fuchsia', 'gainsboro', 
          'ghostwhite', 'gold', 'goldenrod', 'gray', 'green', 'greenyellow', 'honeydew', 'hotpink', 
          'indianred', 'indigo', 'ivory', 'khaki', 'lavender', 'lavenderblush', 'lawngreen', 'lemonchiffon', 
          'lightblue', 'lightcoral', 'lightcyan', 'lightgoldenrodyellow', 'lightgreen', 'lightgray', 'lightpink', 
          'lightsalmon', 'lightseagreen', 'lightskyblue', 'lightslategray', 'lightsteelblue', 'lightyellow', 
          'lime', 'limegreen', 'linen', 'magenta', 'maroon', 'mediumaquamarine', 'mediumblue', 'mediumorchid',
          'mediumpurple', 'mediumseagreen', 'mediumslateblue', 'mediumspringgreen', 'mediumturquoise',
          'mediumvioletred', 'midnightblue', 'mintcream', 'mistyrose', 'moccasin', 'navajowhite', 'navy',
          'oldlace', 'olive', 'olivedrab', 'orange', 'orangered', 'orchid', 'palegoldenrod', 'palegreen', 
          'paleturquoise', 'palevioletred', 'papayawhip', 'peachpuff', 'peru', 'pink', 'plum', 'powderblue', 
          'purple', 'red', 'rosybrown', 'royalblue', 'saddlebrown', 'salmon', 'sandybrown', 'seagreen', 
          'seashell', 'sienna', 'silver', 'skyblue', 'slateblue', 'slategray', 'snow', 'springgreen', 
          'steelblue', 'tan', 'teal', 'thistle', 'tomato', 'turquoise', 'violet', 'wheat', 'white', 
          'whitesmoke', 'yellow', 'yellowgreen']

# take a look at this --> matplotlib.colors.cnames.items()
# https://matplotlib.org/stable/gallery/color/named_colors.html
# https://stackoverflow.com/questions/22408237/named-colors-in-matplotlib    


styles = [".", ",","o","v", "^","<",">","1","2","3","4","8","s","p","P","*","h","H","+","x","X","D","d","|","_"]*6
linestyles = ['-', '--', '-.', ':', '', 'solid', 'dashed', 'dashdot', 'dotted', '-']*3
#https://matplotlib.org/stable/api/markers_api.html




