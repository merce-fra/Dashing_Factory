#include "ns3/core-module.h"
#include "ns3/flow-monitor-module.h"
#include <map>

#include <tuple>

using namespace ns3;

typedef std::map<std::string, std::string> dictionnary;
typedef std::map<std::string, std::string>::iterator dictIterator;



std::tuple<int, int>  ExtractNodesFromContext (const std::string s);
std::string  ExtractMetricFromContext (const std::string s);


void getNodePosition_v2 (NodeContainer nodes, Ipv4Address ipAddress);

std::string  GetNodeName_v3 (NodeContainer nodes, Ipv4Address ipAddress);

void  GetNodeName_v2 (NodeContainer nodes, Ipv4Address ipAddress);

std::string
BuildFileNameString (std::string directoryName, std::string filePrefix, std::string tag);

void LogNodes (NodeContainer nodes, bool position);


void
LogResults_old (Ptr<Ipv4FlowClassifier> classifier, Ptr<ns3::FlowMonitor> monitor, 
            Time simTime, Time clientAppStartTime, uint8_t scenario,
            std::string startStamp, std::string  exp_duration, NodeContainer ueContainer,
            std::string simTag, std::string outputDir);


void
LogResults (Ptr<Ipv4FlowClassifier> classifier, Ptr<ns3::FlowMonitor> monitor, 
               Time simTime, Time clientAppStartTime, uint8_t scenario,
               std::string startStamp, std::string  exp_duration, NodeContainer ueContainer,
               std::string simTag, std::string outputDir);            

void InitializeCsvFile ();
void InitializeCsvFile (std::string mode, std::string logFile);

ns3::Vector GetNodePosition (Ipv4Address ipAddress, NodeContainer ueContainer);
std::string GetNodeName (Ipv4Address ipAddress, NodeContainer ueContainer);
void LogMsg (std::string msg);
std::tuple<double, double, double, double> LogFlowStats (Ipv4FlowClassifier::FiveTuple t,  
              uint32_t flowId, struct ns3::FlowMonitor::FlowStats flowStats, std::string csvfile,
              double flowDuration, NodeContainer ueContainer);

void LogRecapStats (std::string csvfile, std::string startTime, std::string flowDuration,
              double ul_meanThroughput, double ul_meanDelay,
              double dl_meanThroughput, double dl_meanDelay);              


/// void TraceRtt (std::string rtt_tr_file_name);
/// void RttTracer (Time oldval, Time newval);