#include "ns3/applications-module.h"
#include "ns3/config-store.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/nr-helper.h"
#include "ns3/nr-mac-scheduler-tdma-rr.h"
#include "ns3/nr-module.h"
#include "ns3/nr-point-to-point-epc-helper.h"
#include "ns3/point-to-point-helper.h"
#include <ns3/antenna-module.h>
#include <ns3/buildings-helper.h>
#include "ns3/point-to-point-module.h"
#include "ns3/dash-module.h"
#include <string>
#include <fstream>

#include <regex>

#include "ns3/flow-monitor-module.h"
#include "dash-fact.utils.h"





using namespace ns3;




// Online C++ compiler to run C++ program online









std::tuple<int, int>  ExtractNodesFromContext (const std::string s) 
{    
    std::smatch match; int sourceNode, destinationNode;
    std::regex rgx_srcNode (".*/NodeList/(\\w+)/*");
    if (std::regex_search(s.begin(), s.end(), match, rgx_srcNode))
        destinationNode = stoi(match[1]);
    std::regex rgx_dstNode (".*SocketList/(\\w+)/*");
    if (std::regex_search(s.begin(), s.end(), match, rgx_dstNode))
         sourceNode = stoi(match[1]);        
    return std::make_tuple(sourceNode, destinationNode);
}


std::string  ExtractMetricFromContext (const std::string s) 
{    
    std::smatch match; std::string metric;
    std::regex rgx_metric (".*/\*/(\\w+)");
    if (std::regex_search(s.begin(), s.end(), match, rgx_metric))
        metric = match[1];
    return metric;
}



std::string
BuildFileNameString (std::string directoryName, std::string filePrefix, std::string tag)
{
  std::ostringstream oss;
  oss << directoryName << filePrefix << tag;
  return oss.str ();
}



void
LogNodes (NodeContainer nodes, bool position)
{
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  for (auto j = nodes.Begin (); j != nodes.End (); ++j)
    {
      Ptr<Node> object = *j;
      // get node name
      std::string nodeName = Names::FindName (object);
      if (nodeName.empty ()) {
          nodeName = std::to_string (object->GetId ());
        }
                
      NS_LOG_UNCOND ("\t\t-------------");
      NS_LOG_UNCOND ("\t\tnodeName = " << nodeName );
      NS_LOG_UNCOND ("\t\t-----------------------------");

      
      if ((nodeName != "remoteHost") && position) {
              NS_LOG_UNCOND ("\t\tPosition : " << object->GetObject<MobilityModel>()
                                                        ->GetPosition() ); 
      }                                                   
      NS_LOG_UNCOND ("\t\tInterfaces: "); // log all ips for that node with interface index
      Ptr<Ipv4> node_ipv4 = object->GetObject<Ipv4> ();    
      for (uint32_t i = 0; i < node_ipv4->GetNInterfaces (); i++) {
          Ipv4Address ipv4Address = node_ipv4->GetAddress (i, 0).GetLocal ();        
          if (ipv4Address == Ipv4Address::GetLoopback ()) {
              NS_LOG_UNCOND ("\t\t\t" << i << ": " << ipv4Address << " (Loopback)");
            }
          else {
              NS_LOG_UNCOND ("\t\t\t" << i << ": " << ipv4Address);
            }
        }
      // print all routes for that node
      Ptr<Ipv4StaticRouting> staticRouting = ipv4RoutingHelper.GetStaticRouting (node_ipv4);
      uint32_t nRoutes = staticRouting->GetNRoutes ();
      NS_LOG_UNCOND ("\t\tRoutes: ");
      for (uint32_t j = 0; j < nRoutes; j++)
        {
          Ipv4RoutingTableEntry route = staticRouting->GetRoute (j);
          NS_LOG_UNCOND ("\t\t\t" << route.GetDest () << " (" << route.GetDestNetworkMask ()
                                << ") --> " << route.GetGateway () << " (interface "
                                << route.GetInterface () << ")");
        }
      NS_LOG_UNCOND ("\t\t----------------------------------------------------\n");
    }
  NS_LOG_UNCOND ("---");
}


/**
std::string
GetNodeName (Ipv4Address ipAddress, NodeContainer ueContainer) 
**/
/// std::string
void 
GetNodeName_v2 (NodeContainer nodes, Ipv4Address ipAddress)
{
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  for (auto j = nodes.Begin (); j != nodes.End (); ++j) {
      Ptr<Node> object = *j;
      NS_LOG_UNCOND ("\t\tInterfaces: "); // log all ips for that node with interface index
      Ptr<Ipv4> node_ipv4 = object->GetObject<Ipv4> ();    
      for (uint32_t i = 0; i < node_ipv4->GetNInterfaces (); i++) {
          Ipv4Address node_ipv4Address = node_ipv4->GetAddress (i, 0).GetLocal ();        
          if (node_ipv4Address == ipAddress) {
              std::string nodeName = Names::FindName (object);
              if (nodeName.empty ()) {
                  nodeName = std::to_string (object->GetId ());
              }            
              NS_LOG_UNCOND ("\t\t\t add found at the node " << nodeName << 
                             " at the interface (" << i << ")");
              /// return nodeName; 
              break;              
          }
      }
  }
  NS_LOG_UNCOND ("\t\t\t Addr not found, return << unknown >>");
  /// return "unknown";
}


std::string
GetNodeName_v3 (NodeContainer nodes, Ipv4Address ipAddress)
{
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  for (auto j = nodes.Begin (); j != nodes.End (); ++j) {
      Ptr<Node> object = *j;
      /// NS_LOG_UNCOND ("\t\tInterfaces: "); // log all ips for that node with interface index
      Ptr<Ipv4> node_ipv4 = object->GetObject<Ipv4> ();    
      for (uint32_t i = 0; i < node_ipv4->GetNInterfaces (); i++) {
          Ipv4Address node_ipv4Address = node_ipv4->GetAddress (i, 0).GetLocal ();        
          if (node_ipv4Address == ipAddress) {
              std::string nodeName = Names::FindName (object);
              if (nodeName.empty ()) {
                  nodeName = std::to_string (object->GetId ());
              }            
              /// NS_LOG_UNCOND ("\t\t\t add found at the node " << nodeName << 
              ///                " at the interface (" << i << ")");
              return nodeName; 
          }
      }
  }
  /// NS_LOG_UNCOND ("\t\t\t Addr not found, return << unknown >>");
  return "unknown";
}





void getNodePosition_v2 (NodeContainer nodes, Ipv4Address ipAddress)
{
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  for (auto j = nodes.Begin (); j != nodes.End (); ++j)
    {
      Ptr<Node> object = *j;
      std::string nodeName = Names::FindName (object);
      if (nodeName.empty ()) {
          nodeName = std::to_string (object->GetId ());
        }
                
      NS_LOG_UNCOND ("\t\t-------------");
      NS_LOG_UNCOND ("\t\tnodeName = " << nodeName );
      NS_LOG_UNCOND ("\t\t-----------------------------");

      
      if ((nodeName != "remoteHost")) {
              NS_LOG_UNCOND ("\t\tPosition : " << object->GetObject<MobilityModel>()
                                                        ->GetPosition() ); 
      }                                                   
    }
  NS_LOG_UNCOND ("---");
};



  // -------------------------------------------------------------------
  // Initialize csv output files with columns name (to be analyzed next)
  // -------------------------------------------------------------------
     void InitializeCsvFile (std::string mode, std::string logFile){
         // by mode we mean ul/dl file or a recap logfile. 
         // The header differ in each case

            std::ofstream logStream (logFile.c_str ());
            std::string headLine;      

            // Is this a ul/dl ou recap log file ?
               if (mode == "ul" || mode == "dl"){
                   // ul Flows, dl Flows                                         
                   headLine = "flowId,sourceNode,sourceNodePosition,sourceAddress,sourcePort," 
                              "destAddress,destPort,protocol,txPackets,txBytes,txOffered,rxBytes,rxOffered"
                              "throughput,meanDelay,meanJitter,rxPackets,lostPackets,lostPacketsRatio\n";   
               } else {
                   // recap 
                   headLine = "start,duration,ul_meanThroughput,ul_meanDelay,ul_meanJitter,ul_meanLossRatio,"
                              "dl_meanThroughput,dl_meanDelay,dl_meanJitter,dl_meanLossRatio\n";
               }

            logStream << headLine; logStream.close ();
   }   //---------------------
  // --------------   



// -------------------------------------------------------------------
// Initialize csv output files with columns name (to be analyzed next)
// -------------------------------------------------------------------
ns3::Vector 
GetNodePosition (Ipv4Address ipAddress, NodeContainer ueContainer) {
  if (ipAddress != Ipv4Address("1.0.0.2")){
      Ipv4StaticRoutingHelper ipv4RoutingHelper;
      for (auto j = ueContainer.Begin (); j != ueContainer.End (); ++j) {
          Ptr<Node> object = *j;       
          Ptr<Ipv4> node_ipv4 = object->GetObject<Ipv4> ();    
          for (uint32_t i = 0; i < node_ipv4->GetNInterfaces (); i++) {
              Ipv4Address localIpv4Address = node_ipv4->GetAddress (i, 0).GetLocal ();
              if (localIpv4Address == ipAddress) {
                  return (object->GetObject<MobilityModel>()->GetPosition());   
              }
          }     
      }
  }      
  return ns3::Vector (999,999,999);
}


std::string
GetNodeName (Ipv4Address ipAddress, NodeContainer ueContainer) {
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  /// NS_LOG_UNCOND ("Looking for node name with addr ("<< ipAddress<<") among " << ueContainer.GetN() << " nodes");    
  for (auto j = ueContainer.Begin (); j != ueContainer.End (); ++j) {
      Ptr<Node> object = *j;
      Ptr<Ipv4> node_ipv4 = object->GetObject<Ipv4> ();    
      /// NS_LOG_UNCOND ("    Is it node (" << std::to_string (object->GetId ()) 
      ///                                   << ") ?  (i.e. has " << node_ipv4->GetNInterfaces () 
      ///                                   << " interfaces)");              
      for (uint32_t i = 0; i < node_ipv4->GetNInterfaces (); i++) {
          Ipv4Address localIpv4Address = node_ipv4->GetAddress (i, 0).GetLocal ();
          NS_LOG_UNCOND ("    Is it  ("<< localIpv4Address <<") ?? ");              
          if (localIpv4Address == ipAddress) {
              std::string nodeName = Names::FindName (object); //get node name                  
              NS_LOG_UNCOND ("    Yes it is ... ");              
              if (nodeName.empty ()) {
                  nodeName = std::to_string (object->GetId ());
              }
              return nodeName;
          }
          else {
              /// NS_LOG_UNCOND ("    No it is not ... ");              
          }
      }     
  }
  /// NS_LOG_UNCOND ("There is a prob somewhere, unable to find ("<< ipAddress<<")");    
  return "unknow";
}


void
LogMsg (std::string msg) {
  NS_LOG_UNCOND ("\n \n" << msg);
}




std::tuple<double, double, double, double>

LogFlowStats (Ipv4FlowClassifier::FiveTuple t,  
              uint32_t flowId, struct ns3::FlowMonitor::FlowStats flowStats, std::string csvfile,
              double flowDuration, NodeContainer ueContainer){
    /**/    
    std::stringstream protoStream; protoStream << (uint16_t) t.protocol;
    std::ofstream logStream; 
    double throughput, meanDelay, meanJitter, packetLossRatio;
    logStream.open (csvfile.c_str (), std::ios_base::out | std::ios_base::app);        
    if (t.protocol == 6) {protoStream.str ("TCP");}
    if (t.protocol == 17) {protoStream.str ("UDP");}

    double timeTakenClient = flowStats.timeLastTxPacket.GetSeconds () - flowStats.timeFirstTxPacket.GetSeconds ();
    
    logStream    << flowId << "," << GetNodeName_v3(ueContainer, t.sourceAddress) << "," 
                 << GetNodePosition(t.sourceAddress, ueContainer) << "," 
                 << t.sourceAddress << "," << t.sourcePort << "," 
                 << t.destinationAddress << "," << t.destinationPort << "," << protoStream.str () << "," 
                 << flowStats.txPackets << ","
                 << flowStats.txBytes << ","
                 << flowStats.txBytes * 8.0 / timeTakenClient / 1024.0 / 1024.0   << "Mbps,"
                 << flowStats.rxBytes << ",";   

    double timeTaken = flowStats.timeLastRxPacket.GetSeconds () - flowStats.timeFirstTxPacket.GetSeconds ();

    if (flowStats.rxPackets > 0) {
        throughput = flowStats.rxBytes * 8.0 / timeTaken / 1024 / 1024;
        meanDelay =  1000 * flowStats.delaySum.GetSeconds () / flowStats.rxPackets;
        meanJitter = 1000 * flowStats.jitterSum.GetSeconds () / flowStats.rxPackets;
        logStream << throughput << "Mbps," << meanDelay << "ms," << meanJitter << "ms,";
    } else {
        logStream << "0Mbps,0ms,0ms,";
    }
    logStream << flowStats.rxPackets << ",";
    logStream << flowStats.lostPackets << ",";
    if ((flowStats.lostPackets > 0) & (flowStats.rxPackets > 0)) {
        packetLossRatio = (double) (flowStats.lostPackets/ (double) (flowStats.rxPackets + flowStats.lostPackets));
    } 
    else {packetLossRatio = 0;}

    logStream << packetLossRatio << "," << std::endl;
    logStream.close ();
    /**/
    return std::make_tuple(throughput, meanDelay, meanJitter, packetLossRatio);
}
/**/


/**/
void
LogRecapStats (std::string csvfile, std::string startTime, std::string flowDuration,
              double ul_meanThroughput, double ul_meanDelay, double ul_meanJitter, double ul_meanLossRatio,
              double dl_meanThroughput, double dl_meanDelay, double dl_meanJitter, double dl_meanLossRatio){
    /**/               
    std::ofstream logStream; 
    logStream.open (csvfile.c_str (), std::ios_base::out | std::ios_base::app);        

    logStream    << startTime << "," << flowDuration << "," 
                 << ul_meanThroughput << "Mbps," << ul_meanDelay << "ms," << ul_meanJitter << "ms," << ul_meanLossRatio << ","
                 << dl_meanThroughput << "Mbps," << dl_meanDelay << "ms," << dl_meanJitter << "ms," << dl_meanLossRatio << ",";         

    logStream.close ();
    /**/

}
/**/





/**/
void
LogResults_old (Ptr<Ipv4FlowClassifier> classifier, Ptr<ns3::FlowMonitor> monitor, 
            Time simTime, Time clientAppStartTime, uint8_t scenario,
            std::string startStamp, std::string  exp_duration, NodeContainer ueContainer,
            std::string simTag, std::string log_dir)
{
  std::string ul_tag = "ulFlows_";
  std::string dl_tag = "dlFlows_";
  std::string recap_tag = "recapStats";

  std::string ul_logFile, dl_logFile, recap_logFile; 
  recap_logFile = log_dir+recap_tag+".csv"; InitializeCsvFile ("recap", recap_logFile);                  
       
  monitor->CheckForLostPackets ();
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  uint16_t ulFlows=0, dlFlows=0;
  double flowDuration = (simTime - clientAppStartTime).GetSeconds ();
  double ul_throughput, ul_delay, ul_jitter, ul_lossRatio, 
         dl_throughput, dl_delay, dl_jitter, dl_lossRatio;  
  double ul_avgThroughput = 0, ul_avgDelay = 0, ul_avgJitter = 0, ul_avgLossRatio = 0;
  double dl_avgThroughput = 0, dl_avgDelay = 0, dl_avgJitter = 0, dl_avgLossRatio = 0;

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (t.sourceAddress == ns3::Ipv4Address ("1.0.0.2")){
         dlFlows++;
         dl_logFile = log_dir+dl_tag+std::to_string(t.destinationAddress.Get())+".csv"; 
         InitializeCsvFile ("dl", dl_logFile);                           
         std::tie(dl_throughput, dl_delay, dl_jitter, dl_lossRatio) = LogFlowStats (t, i->first, i->second,dl_logFile,flowDuration, ueContainer);  
         dl_avgThroughput += dl_throughput; dl_avgDelay+=dl_delay; dl_avgJitter+=dl_jitter;
         continue;
      }
      else {
         ulFlows++;
         // rd_svc_117440514.csv ?
         ul_logFile = log_dir+ul_tag+std::to_string(t.sourceAddress.Get())+".csv";
         InitializeCsvFile ("ul", ul_logFile);   
         std::tie(ul_throughput, ul_delay, ul_jitter, ul_lossRatio) = LogFlowStats (t, i->first, i->second,ul_logFile,flowDuration, ueContainer);      
         ul_avgThroughput += ul_throughput; ul_avgDelay+=ul_delay; ul_avgJitter+=ul_jitter;
      }
    } //for  
  // log Recap  
  dl_avgThroughput = dl_avgThroughput/dlFlows;  dl_avgDelay     = dl_avgDelay/dlFlows; 
      dl_avgJitter = dl_avgJitter/dlFlows;      dl_avgLossRatio = dl_avgLossRatio/dlFlows; 
  ul_avgThroughput = ul_avgThroughput/ulFlows;  ul_avgDelay     = ul_avgDelay/ulFlows;  
      ul_avgJitter = ul_avgJitter/ulFlows;      ul_avgLossRatio = ul_avgLossRatio/dlFlows;  
  LogRecapStats (recap_logFile, startStamp, exp_duration, 
                 ul_avgThroughput, ul_avgDelay, ul_avgJitter, ul_avgLossRatio,
                 dl_avgThroughput, dl_avgDelay, dl_avgJitter, dl_avgLossRatio);

  NS_LOG_UNCOND ("     [Up  link] - " << ulFlows << " flows");
  NS_LOG_UNCOND ("       (Ul)     Mean flow throughput: " << ul_avgThroughput << " Mbps");
  NS_LOG_UNCOND ("       (Ul)     Mean flow delay: " << ul_avgDelay << " ms");      
  NS_LOG_UNCOND ("       (Ul)     Mean flow jitter: " << ul_avgJitter << " ms");      
  NS_LOG_UNCOND ("       (Ul)     Mean flow loss ratio: " << ul_avgLossRatio);      
  NS_LOG_UNCOND ("     ---------------------"); 
  NS_LOG_UNCOND ("     [Downlink] - " << dlFlows << " flows");
  NS_LOG_UNCOND ("        (Dl)    Mean flow throughput: " << dl_avgThroughput << " Mbps");
  NS_LOG_UNCOND ("        (Dl)    Mean flow delay: " << dl_avgDelay << " ms");        
  NS_LOG_UNCOND ("        (Dl)    Mean flow jitter: " << dl_avgJitter << " ms");        
  NS_LOG_UNCOND ("        (Dl)    Mean flow loss ratio: " << dl_avgLossRatio);        
  NS_LOG_UNCOND ("     ---------------------\n");   
}
/**/


/**/
void
LogResults (Ptr<Ipv4FlowClassifier> classifier, Ptr<ns3::FlowMonitor> monitor, 
               Time simTime, Time clientAppStartTime, uint8_t scenario,
               std::string startStamp, std::string  exp_duration, NodeContainer ueContainer,
               std::string simTag, std::string log_dir)
{
  std::string ul_tag = "ulFlows";
  std::string dl_tag = "dlFlows";
  std::string recap_tag = "recapStats";
  int users = ueContainer.GetN ();

  std::string ul_logFile, dl_logFile, recap_logFile; 
  recap_logFile = log_dir+recap_tag+".csv"; InitializeCsvFile ("recap", recap_logFile);
  dl_logFile    = log_dir+dl_tag+".csv"; InitializeCsvFile ("dl", dl_logFile); 
  ul_logFile    = log_dir+ul_tag+".csv"; InitializeCsvFile ("ul", ul_logFile); 
       
  monitor->CheckForLostPackets ();
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  uint16_t ulFlows=0, dlFlows=0;
  double flowDuration = (simTime - clientAppStartTime).GetSeconds ();
  double ul_throughput, ul_delay, ul_jitter, ul_lossRatio, 
         dl_throughput, dl_delay, dl_jitter, dl_lossRatio;  
  double ul_avgThroughput = 0, ul_avgDelay = 0, ul_avgJitter = 0, ul_avgLossRatio = 0;
  double dl_avgThroughput = 0, dl_avgDelay = 0, dl_avgJitter = 0, dl_avgLossRatio = 0;  

  NS_LOG_UNCOND (" ---------------------------------------------------------------------------------------------------------");
  NS_LOG_UNCOND ("  ul/dl | flowId |     Src     |     Dest    |  throughput   |  e2e delay  |  e2e jitter  | lost Packets  |" );
  NS_LOG_UNCOND (" ---------------------------------------------------------------------------------------------------------");

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i) {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
      if (t.sourceAddress == ns3::Ipv4Address ("1.0.0.2")){
         dlFlows++;
         if (dlFlows < users) {
             std::tie(dl_throughput, dl_delay, dl_jitter, dl_lossRatio) = LogFlowStats (t, i->first, i->second,dl_logFile,flowDuration, ueContainer);  
             dl_avgThroughput += dl_throughput; dl_avgDelay+=dl_delay; dl_avgJitter+=dl_jitter; dl_avgLossRatio += dl_lossRatio;
             NS_LOG_UNCOND ("   dl   | " << dlFlows 
                                << " | " << t.sourceAddress << ":" << t.sourcePort
                                << " | " << t.destinationAddress  << ":" << t.destinationPort
                                << " | " << dl_throughput << " Mbps"
                                << " | " << dl_delay << " ms" 
                                << " | " << dl_jitter << " ms" 
                                << " | " << dl_lossRatio);
             // continue;             
         } else {
             NS_LOG_UNCOND ("dl Flow n°" << dlFlows << " , ignore ... ");
         }
      }
      else {
         ulFlows++;
         if (ulFlows < users) {
             std::tie(ul_throughput, ul_delay, ul_jitter,ul_lossRatio) = LogFlowStats (t, i->first, i->second,ul_logFile,flowDuration, ueContainer);      
             ul_avgThroughput += ul_throughput; ul_avgDelay+=ul_delay; ul_avgJitter+=ul_jitter;  ul_avgLossRatio += ul_lossRatio;
             NS_LOG_UNCOND ("   ul   | " << ulFlows 
                                << " |  " << t.sourceAddress << ":" << t.sourcePort 
                                << " |  " << t.destinationAddress << ":" << t.destinationPort
                                << " | " << ul_throughput << " Mbps"
                                << " | " << ul_throughput << " Mbps"
                                << " | " << ul_delay << " ms"
                                << " | " << ul_jitter << " ms"
                                << " | " << ul_lossRatio);
         } else {
             NS_LOG_UNCOND ("ul Flow n°" << ulFlows << ", ignore ... ");
         }                            
      }
    } //for  
  // log Recap  
  int flows = users - 1; // i.e. end users without the remote station.
  
  dl_avgThroughput = dl_avgThroughput/flows;  dl_avgDelay = dl_avgDelay/flows; 
  dl_avgJitter = dl_avgJitter/flows;  dl_avgLossRatio = dl_avgLossRatio/flows;

  ul_avgThroughput  = ul_avgThroughput/flows;  ul_avgDelay = ul_avgDelay/flows; 
  ul_avgJitter = ul_avgJitter/flows;  ul_avgLossRatio = ul_avgLossRatio/flows;

  LogRecapStats (recap_logFile, startStamp, exp_duration, 
                 ul_avgThroughput, ul_avgDelay, ul_avgJitter, ul_avgLossRatio,
                 dl_avgThroughput, dl_avgDelay, dl_avgJitter, dl_avgLossRatio);                 
  NS_LOG_UNCOND ("We have " << stats.size () << " flows");
  NS_LOG_UNCOND ("");
  NS_LOG_UNCOND ("    -------");
  NS_LOG_UNCOND ("   ( RECA  P )");
  NS_LOG_UNCOND ("    -------");
  NS_LOG_UNCOND ("     [Uplink] - " << ulFlows << " flows (" << flows << " considered)");
  NS_LOG_UNCOND ("       (Ul)     Mean flow throughput: " << ul_avgThroughput << " Mbps");
  NS_LOG_UNCOND ("       (Ul)     Mean flow delay: " << ul_avgDelay << " ms");      
  NS_LOG_UNCOND ("       (Ul)     Mean flow jitter: " << ul_avgJitter << " ms");      
  NS_LOG_UNCOND ("       (Ul)     Mean flow loss ratio: " << ul_avgLossRatio);      
  NS_LOG_UNCOND ("     ---------------------"); 
  NS_LOG_UNCOND ("     [Downlink] - " << dlFlows << " flows (" << flows << " considered)");
  NS_LOG_UNCOND ("        (Dl)    Mean flow throughput: " << dl_avgThroughput << " Mbps");
  NS_LOG_UNCOND ("        (Dl)    Mean flow delay: " << dl_avgDelay << " ms");        
  NS_LOG_UNCOND ("        (Dl)    Mean flow jitter: " << dl_avgJitter << " ms");        
  NS_LOG_UNCOND ("        (Dl)    Mean flow loss ratio: " << dl_avgLossRatio);        
  NS_LOG_UNCOND ("     ---------------------\n");   
}







/**/
