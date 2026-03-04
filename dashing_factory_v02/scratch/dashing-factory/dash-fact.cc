/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2019 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 as
 *   published by the Free Software Foundation;
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include "dash-fact.h"
#include "tsn-flow.h" 
#include "dash-fact.utils.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("dash-fact");


static bool g_rxPdcpCallbackCalled = false;
static bool g_rxRxRlcPDUCallbackCalled = false;
Time g_txPeriod = Seconds(0.1);
Time delay;
std::fstream m_ScenarioFile;


/*
 * TraceSink, RxRlcPDU connects the trace sink with the trace source (RxPDU). It connects the UE with gNB and vice versa.
 */
void RxRlcPDU (std::string path, uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t rlcDelay)
    {
      g_rxRxRlcPDUCallbackCalled = true;
      delay = Time::FromInteger(rlcDelay,Time::NS);
      std::cout<<"\n rlcDelay in NS (Time):"<< delay<<std::endl;
    
      std::cout<<"\n\n Data received at RLC layer at:"<<Simulator::Now()<<std::endl;
    
      m_ScenarioFile << "\n\n Data received at RLC layer at:"  << Simulator::Now () << std::endl;
      m_ScenarioFile << "\n rnti:" << rnti  << std::endl;
      m_ScenarioFile << "\n delay :" << rlcDelay << std::endl;
    }

void RxPdcpPDU (std::string path, uint16_t rnti, uint8_t lcid, uint32_t bytes, uint64_t pdcpDelay)
    {
      std::cout << "\n Packet PDCP delay:" << pdcpDelay << "\n";
      g_rxPdcpCallbackCalled = true;
    }

void ConnectUlPdcpRlcTraces ()
    {
      Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/UeMap/*/DataRadioBearerMap/*/LtePdcp/RxPDU",
                        MakeCallback (&RxPdcpPDU));
    
      Config::Connect ("/NodeList/*/DeviceList/*/LteEnbRrc/UeMap/*/DataRadioBearerMap/*/LteRlc/RxPDU",
                          MakeCallback (&RxRlcPDU));
      NS_LOG_INFO ("Received PDCP RLC UL");
    }    // ------------------------------------
// ----


// ------------------------------------------------------------------------------------------------
   DashFact::DashFact ()
    //: // m_bufferSpace (0),
      // m_player (this->GetObject<DashClient> (), m_bufferSpace),
      // m_segmentFetchTime (Seconds (0))
   { 
      // 
    }
// ------------------------------------------------------------------------------------------------



// --------------------------------------------------------------------------------------------------                                        
   void DashFact::TraceRto () {Config::Connect ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/RTO", 
                               MakeCallback(&DashFact::RtoTracer, this));}
                           // ---------------------------------------------
   void DashFact::RtoTracer (std::string context, Time oldval, Time newval) {                                                                        
      int source, destination; std::tie(source, destination) = ExtractNodesFromContext (context);                        
      m_outRTOStatsFile << destination << "," 
                        << Simulator::Now ().GetSeconds () << "," 
                        << source << ","
                        << oldval.GetSeconds () << "," 
                        << newval.GetSeconds () << std::endl;
} // ---------------------------------------------------------

// --------------------------------------------------------------------------------------------------
   void DashFact::TraceTCP () 
       {
           // Metrics if type int
              std::string intTraces[10] = { "BytesInFlight", "CongestionWindow", "CongestionWindowInflated", 
                                            "SlowStartThreshold", "RWND", "AdvWND"};
                                            /// "CongState", "EcnState", "PacingRate"}; 
              for (int i = 0; i < 6; i++) {
                  /// std::cout << "/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/"+intTraces[i]+"\n";  
                  Config::Connect    ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/"+intTraces[i], 
                  MakeCallback(&DashFact::TCPIntTracer, this));
              }


           // Metrics if type SequenceNumber32
              Config::Connect    ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/NextTxSequence", 
              MakeCallback(&DashFact::TCPSeqTracer, this));
              Config::Connect    ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/HighestSequence", 
              MakeCallback(&DashFact::TCPSeqTracer, this));   
              Config::Connect    ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/HighestRxSequence", 
              MakeCallback(&DashFact::TCPSeqTracer, this));                                 

           // Metrics if type Time
              Config::Connect ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/RTO", 
              MakeCallback(&DashFact::TCPTimeTracer, this));
              Config::Connect ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/RTT", 
              MakeCallback(&DashFact::TCPTimeTracer, this));                                                        
                      
           // Metrics of type rate 
              Config::Connect ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/PacingRate", 
              MakeCallback(&DashFact::TCPRateTracer, this));

           // CongState, EcnState
              Config::Connect ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/CongState", 
              MakeCallback(&DashFact::TCPCongStateTracer, this));
              Config::Connect ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/EcnState", 
              MakeCallback(&DashFact::TCPEcnStateTracer, this));                                                                      
       }

void DashFact::TCPIntTracer (std::string context, uint32_t oldval, uint32_t newval) 
    { TCPTracerX <uint32_t> (context, oldval, newval);}
void DashFact::TCPSeqTracer (std::string context, SequenceNumber32 oldval, SequenceNumber32 newval) 
    { TCPTracerX <SequenceNumber32> (context, oldval, newval);} 
void DashFact::TCPTimeTracer (std::string context, Time oldval, Time newval) 
    { TCPTracerX <Time> (context, oldval, newval);} 
void DashFact::TCPRateTracer (std::string context, DataRate oldval, DataRate newval) 
    { TCPTracerX <DataRate> (context, oldval, newval);} 
void DashFact::TCPCongStateTracer (std::string context, TcpSocketState::TcpCongState_t oldval, TcpSocketState::TcpCongState_t newval) 
    { TCPTracerX <TcpSocketState::TcpCongState_t> (context, oldval, newval);}                                     
void DashFact::TCPEcnStateTracer (std::string context, TcpSocketState::EcnState_t oldval, TcpSocketState::EcnState_t newval) 
    { TCPTracerX <TcpSocketState::EcnState_t> (context, oldval, newval);}                                     



   /// void DashFact::TCPTracerInt (std::string context, uint32_t oldval, uint32_t newval) {
   ///    int source, destination; std::tie(source, destination) = ExtractNodesFromContext (context);
   ///    std::string metric = ExtractMetricFromContext (context);                  
   ///        std::string total_path = m_output_directory + m_tcp_traceFiles [metric];          
   ///        std::ofstream recapStream;                 
   ///        recapStream.open (total_path.c_str (), std::ios_base::app); recapStream.setf (std::ios_base::fixed);         
   ///        recapStream << Simulator::Now ().GetSeconds () << "," 
   ///                    << destination << "," 
   ///                    << source << ","
   ///                    << oldval << "," 
   ///                    << newval << std::endl;                      
   ///         recapStream.close ();   
   /// }
// --------------------------------------------------------------------------------------------------                                        




// -------------------------
template<typename T> void  // --------------------------------------------
DashFact:: TCPTracerX (std::string context, T oldValue, T newValue)
{
      int source, destination; std::tie(source, destination) = ExtractNodesFromContext (context);
      std::string metric = ExtractMetricFromContext (context);  
      std::string total_path = m_output_directory + m_tcp_traceFiles [metric];          
      NS_LOG_INFO ("Trying to add content to " << total_path);
      NS_LOG_INFO ("    Context :  " << context);
      NS_LOG_INFO ("    oldValue : " << oldValue << ",  newValue : " << newValue);

      std::ofstream recapStream;                 
      recapStream.open (total_path.c_str (), std::ios_base::app); recapStream.setf (std::ios_base::fixed);         
      recapStream << Simulator::Now ().GetSeconds () << "," 
                  << destination << ","
                  << source << ",";
      if constexpr (std::is_same_v<T, Time>) { 
          NS_LOG_INFO ("Kayna .........");
          recapStream << oldValue.GetSeconds () << "," 
                      << newValue.GetSeconds () << std::endl;
      } else {
          NS_LOG_INFO ("maKaynachh .........");             
          recapStream << oldValue << "," 
                      << newValue << std::endl;         
      }
       recapStream.close ();                          
}         // ------------------------------------------------------
// ----




// --------------------------------------------------------------------------------------------------                                        
   void DashFact::TraceRtt () {Config::Connect ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/RTT", 
                               MakeCallback(&DashFact::RttTracer, this));}
                           // ---------------------------------------------
   void DashFact::RttTracer (std::string context, Time oldval, Time newval) {                                                                        
      int source, destination; std::tie(source, destination) = ExtractNodesFromContext (context);                        
      m_outRTTStatsFile << Simulator::Now ().GetSeconds () << "," 
                        << destination << "," 
                        << source << ","
                        << oldval.GetSeconds () << "," 
                        << newval.GetSeconds () << std::endl;
} // ---------------------------------------------------------





/** -------------------------------------------------------------------
 * A callback function that redirects a call to the scenario instance.
 * @param scenario A pointer to a simulation instance
 * @param params RxPacketTraceParams structure containing RX parameters
 * ---------------------------------------------------------------------*/
// void UeReceptionTrace (Dash5G* scenario, RxPacketTraceParams params)
//     { scenario->UeReception (params);}
// ------------------------------------------------------------
   /// void DashFact::TraceSinr (NetDeviceContainer ueNetDev) {
   ///    // Trace for SINR (another format)
   ///       for (uint32_t i = 0; i < ueNetDev.GetN(); i++) {
   ///          Ptr<NrSpectrumPhy> ue1SpectrumPhy = DynamicCast<NrUeNetDevice>(ueNetDev.Get(i))->GetPhy(0)->GetSpectrumPhy();
   ///          /// ue1SpectrumPhy->TraceConnectWithoutContext("RxPacketTraceUe", MakeBoundCallback(&UeReceptionTrace, this));
   ///          ue1SpectrumPhy->TraceConnectWithoutContext("RxPacketTraceUe", MakeBoundCallback(&DashFact::SinrTracer, this));
   ///       } 
   /// }
   // ---------------------------------------------
   ///  void DashFact::SinrTracer (RxPacketTraceParams params){
   ///     m_outSinrFile << params.m_cellId << params.m_rnti << "\t" << 10 * log10 (params.m_sinr) << std::endl;      
   ///  }  // ---------------------------------------------
   ///   ///  void DashFact::UeReception (RxPacketTraceParams params) {
   ///      m_outSinrFile << params.m_cellId << params.m_rnti << "\t" << 10 * log10 (params.m_sinr) << std::endl;
   ///  }   
   // ---------------------------------------------------------



// -----------------------------
// Initialize some trace files
// -------------------------------------------------------------------
   void DashFact::Initialize_someTraceFiles (){

              m_outRssiFile        << "timestamp"  << "\t" << "rssidBm\n";
              m_agv_outRssiFile    << "timestamp"  << "\t" << "rssidBm\n";
              m_outSnrFile         << "timestamp"  << "\t" << "snr\n";
              m_agv_outSnrFile     << "timestamp"  << "\t" << "snr\n";

              m_agv_outSinrFile    << "timestamp"  << "\t" << "cellId"   << "\t" << "rnti"   << "\t" << "nodeId"  << "\t" 
                                   << "frameNum"   << "\t" << "corrupt"  << "\t" << "mcs"    << "\t" << "rv"      << "\t" << "sinr" << "\t"
                                   << "sinrMin"    << "\t" << "tbler"    << "\t" << "tbSize\n";
   
              m_outSinrFile        << "timestamp"  << "\t" << "cellId"   << "\t" << "rnti"  << "\t" << "nodeId"  << "\t" 
                                   << "frameNum"   << "\t" << "corrupt"  << "\t" << "mcs"   << "\t" << "rv"      << "\t" << "sinr" << "\t"
                                   << "sinrMin"    << "\t" << "tbler"    << "\t" << "tbSize\n";                                

              m_outCouplingFile << "timestamp"  << "\t" << "node"   << "\t" << "nodeId"  << "\t" << "imsi"  << "\t" 
                                << "gnb"  << "\t" << "cellId"  << "\t" << "gnb_x"  << "\t" << "gnb_y"  << "\t" 
                                << "ue_x\t" << "ue_y"  << "\t" << "gain_db"  << "\t" << "shadowing_loss_db\n";
          m_agv_outCouplingFile << "timestamp"  << "\t" << "node"   << "\t" << "nodeId"  << "\t" << "imsi"  << "\t" 
                                << "gnb"  << "\t" << "cellId"  << "\t" << "gnb_x"  << "\t" << "gnb_y"  << "\t" 
                                << "ue_x\t" << "ue_y"  << "\t" << "gain_db"  << "\t" << "shadowing_loss_db\n";   

          m_agv_outPositionsFile << "timestamp,node,x,y,velocity,imsi,cellId,cellIdPhy,rntiPhy\n";                                                   
          m_outPositionsFile  << "node"  << "\t" << "nodeId"   << "\t" << "imsi"  << "\t" 
                                << "x" << "\t" << "y"  << "\t" << "z\n"; 
          m_gnb_outPositionsFile  << "gnb"  << "\t" << "nodeId"   << "\t" 
                                << "x" << "\t" << "y"  << "\t" << "z\n"; 

             m_outDistancesFile << "nodeId" << "\t" << "distance" << "\t" << "gnbId" << std::endl;   
  
          } // --------------------------------------------------------------------------                                                 
// --------




// ---------------------------------
// Initialize RTO csv trace files
// 
//  n.b. be sure about the source nodes. We are still in doubt about these !
//       the current idea is that the socket are ordered on the same way 
//       as nodes, which I think is not necessary in this case
// -------------------------------------------------------------------
   void DashFact::Initialize_RtxTraceFile (){
              m_outRTOStatsFile << "destination,timeStamp,orignNode,oldValue,newValue\n";                                              
              m_outRTTStatsFile << "destination,timeStamp,orignNode,oldValue,newValue\n";                                              
          } // --------------------------------------------------------------------------                                                 
// --------

void set_RTTOutdir ( std::string outdir) {Config::SetDefault("ns3::TcpSocketBase::RTTOutDir", StringValue(outdir));}
/// static void TraceRtt () { Config::ConnectWithoutContext ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/RTT", MakeCallback (&RttTracer));}



// ---------------------------------
// Initialize TCP csv trace files
// 
//  n.b. file names are also coded in almost a hard way 
//       on tcp-socket-base.h
// -------------------------------------------------------------------
   void DashFact::Initialize_TcpTraceFiles (std::string output_directory){

          dictionnary tcp_traceFiles;
          tcp_traceFiles ["PacingRate"]  = "tcp_pacing_rat.csv"; // PacingRate
          tcp_traceFiles ["CongestionWindow"]  = "tcp_congestion_window.csv"; // Congestion Window
          tcp_traceFiles ["CongestionWindowInflated"]  = "tcp_inflated_congestion_window.csv"; //  Inflated Congestion window
          tcp_traceFiles ["SlowStartThreshold"]  = "tcp_slow_start_threshold.csv"; //Slow Start Threshold 
          tcp_traceFiles ["CongState"]  = "tcp_congestion_state.csv"; //Congestion State
          tcp_traceFiles ["EcnState"]  = "tcp_ecnState.csv"; // EcnState
          tcp_traceFiles ["NextTxSequence"]  = "tcp_next_tx_sequence.csv"; // Next sequence number to send (SND.NXT)
          tcp_traceFiles ["HighestRxSequence"]  = "tcp_highest_rx_sequence.csv"; // HighestRxSequence
          tcp_traceFiles ["HighestSequence"]  = "tcp_highest_sequence.csv"; // Highest sequence number ever sent in socket's life time
          tcp_traceFiles ["BytesInFlight"]  = "tcp_bytes_inflight.csv"; // Bytes inflight
          tcp_traceFiles ["RWND"]  = "tcp_RWND.csv"; // Remote side's flow control window
          tcp_traceFiles ["AdvWND"]  = "tcp_AdvWND.csv"; // Advertised Window Size
          tcp_traceFiles ["RTT"]  = "tcp_RTT.csv"; // Last RTT sample
          tcp_traceFiles ["RTO"]  = "tcp_RTO.csv"; // Retransmission timeout

          m_tcp_traceFiles = tcp_traceFiles;

          // for (auto const& [parameter, csv] : dictionnary)
          for (auto const& [parameter, csv] : tcp_traceFiles) {                                                  
                  std::string total_path = output_directory + csv;
                  std::ofstream recapStream;                 
                  recapStream.open (total_path.c_str ()); recapStream.setf (std::ios_base::fixed);
                  if (!recapStream.is_open ()) { NS_ABORT_MSG ("Can't open file " << total_path);}    
                  recapStream << "timeStamp,destination,orignNode,oldValue,newValue\n";
                  recapStream.close ();          
           }   
 }   //---------------------
// -------------- 






// ------------------------------------
void DashFact::closeOutputFiles (){
    m_outSinrFile.close ();
    m_agv_outSinrFile.close ();
    m_outSnrFile.close ();
    m_agv_outSnrFile.close ();
    m_outRssiFile.close ();
    m_agv_outRssiFile.close ();
    m_outCouplingFile.close ();
    m_agv_outCouplingFile.close ();
    m_outPositionsFile.close ();
    m_agv_outPositionsFile.close ();    
    m_gnb_outPositionsFile.close ();
    m_outDistancesFile.close ();
    m_outDashStatsFile.close ();
}; // ---------------------

// ------------------------
   DashFact::~DashFact () { closeOutputFiles ();}
// ---------------------


// ----------------------------------------------------------------------
   void UeReceptionTrace(DashFact* scenario, RxPacketTraceParams params) 
       { scenario->UeReception (params); } 
   
   void DashFact::UeReception(RxPacketTraceParams params)
       { m_outSinrFile << Simulator::Now().GetSeconds() << "s\t" 
                           << params.m_cellId << "\t" << params.m_rnti << "\t" << "-1" 
                           << "\t" << params.m_frameNum
                           << "\t" << params.m_corrupt
                           << "\t" << uint32_t (params.m_mcs)
                           << "\t" << uint32_t (params.m_rv)
                           << "\t" << 10 * log10(params.m_sinr)
                           << "\t" << 10 * log10(params.m_sinrMin)
                           << "\t" << params.m_tbler
                           << "\t" << params.m_tbSize
                           << std::endl; }       
// ----------------------------------



// ----------------------------------------------------------------------
   void AgvReceptionTrace(DashFact* scenario, RxPacketTraceParams params) 
       { scenario->AgvReception (params); } 
   
   void DashFact::AgvReception(RxPacketTraceParams params)
       { m_agv_outSinrFile << Simulator::Now().GetSeconds() << "s\t" 
                           << params.m_cellId << "\t" << params.m_rnti << "\t" << "24" 
                           << "\t" << params.m_frameNum
                           << "\t" << params.m_corrupt
                           << "\t" << uint32_t (params.m_mcs)
                           << "\t" << uint32_t (params.m_rv)
                           << "\t" << 10 * log10(params.m_sinr)
                           << "\t" << 10 * log10(params.m_sinrMin)
                           << "\t" << params.m_tbler
                           << "\t" << params.m_tbSize
                           << std::endl; }
// ----------------------------------



/**  ----------------------------------------------------------------------
 * A callback function that redirects a call to the scenario instance.
 * @param scenario A pointer to a simulation instance
 * @param rssidBm rssidBm RSSI value in dBm
 * 
 * Inside (model/NrInterference) : .AddTraceSource ("RssiPerProcessedChunk",
 *                                                  "Rssi per processed chunk.",
 *                                                   MakeTraceSourceAccessor (&NrInterference::m_rssiPerProcessedChunk),
 *                                                  "ns3::RssiPerProcessedChunk::TracedCallback")
 *   ------------------------------------------------------------------------*/
   void UeRssiPerProcessedChunkTrace(DashFact* scenario, double rssidBm) 
       { scenario->UeRssiPerProcessedChunk (rssidBm);}
   
   void DashFact::UeRssiPerProcessedChunk (double rssidBm)
       { m_outRssiFile << Simulator::Now().GetSeconds() << "s\t" 
                       << rssidBm << std::endl; }
// ------------------------------------------



/* ----------------------------------------------------------------------
 * A callback function that redirects a call to the scenario instance.
 * @param scenario A pointer to a simulation instance
 * @param rssidBm rssidBm RSSI value in dBm
 *-----------------------------------------*/
void AgvRssiPerProcessedChunkTrace (DashFact* scenario, double rssidBm)
    { scenario->AgvRssiPerProcessedChunk (rssidBm); }

void DashFact::AgvRssiPerProcessedChunk (double rssidBm)
    { m_agv_outRssiFile << Simulator::Now().GetSeconds() << "s\t" 
                       << rssidBm << std::endl; }
// --------------------------------------------------




/* ----------------------------------------------------------------------
 * A callback function that redirects a call to the scenario instance.
 * @param scenario A pointer to a simulation instance
 * @param SNR SNR RSSI value in dBm
 *-----------------------------------------*/
void AgvSnrPerProcessedChunkTrace (DashFact* scenario, double snr)
    { scenario->AgvSnrPerProcessedChunk (snr); }

void DashFact::AgvSnrPerProcessedChunk (double snr)
    { m_agv_outSnrFile << Simulator::Now().GetSeconds() << "s\t" 
                   << 10 * log10 (snr) << std::endl; }

void SnrPerProcessedChunkTrace (DashFact* scenario, double snr)
    { scenario->SnrPerProcessedChunk (snr); }

void DashFact::SnrPerProcessedChunk (double snr)
    { m_outSnrFile << Simulator::Now().GetSeconds() << "s\t" 
                   << 10 * log10 (snr) << std::endl; }                   
// --------------------------------------------------



// ----------------------------------------------------------------------
   /// void ReportPowerLena(DashFact* scenario, RxPacketTraceParams params) 
   ///     { scenario->UeReception (params); }
   /// 
   /// void DashFact::UeReception(RxPacketTraceParams params)
   ///     { m_outSinrFile << params.m_cellId << params.m_rnti
   ///                     << "\t" << 10 * log10(params.m_sinr)
   ///                     << std::endl; }
// ----------------------------------




// -------------------------------------------------------------------------------------------
   Ptr<ListPositionAllocator> 
   DashFact::ue_positions_static (int users, int hUT, bool randomness) {
          // Creating positions of end users according to the 3gpp TR 38.900 Figure 7.2.-1
             Ptr<ListPositionAllocator> uePositionAlloc = CreateObject<ListPositionAllocator>();
             for (uint8_t j = 0; j < 2; j++){
                 for (uint8_t i = 0; i < 6; i++){                 
                     uePositionAlloc->Add (Vector ( i * 20 - 5, j * 20, hUT));
                   }
               }
             return uePositionAlloc;   
     }
// ----------------------------------



NodeContainer
DashFact::SelectWellPlacedUes(const NodeContainer ueNodes, const NodeContainer gnbNodes,
                              double minDistance, uint32_t numberOfUesToBeSelected)
{   /// called using  NodeContainer sn = SelectWellPlacedUes (agvCandidateNodes, gnbNodes, minDistance, 1);
    NodeContainer ueNodesFiltered;
    bool correctDistance = true;
    /// NS_LOG_UNCOND ("Start SelectWell Placed Ues ...");
    /// NS_LOG_UNCOND ("                           " << ueNodes.GetN () << " nodes inside ueNodes ()");    
    for (NodeContainer::Iterator itUe = ueNodes.Begin(); itUe != ueNodes.End(); itUe++)
    {
        /// NS_LOG_UNCOND ("inside ueNodes loops ...");
        correctDistance = true;
        Ptr<MobilityModel> ueMm = (*itUe)->GetObject<MobilityModel>();
        Vector uePos = ueMm->GetPosition();

        /// NS_LOG_UNCOND ("                           " << gnbNodes.GetN () << " nodes inside gnbNodes ()");    
        for (NodeContainer::Iterator itGnb = gnbNodes.Begin(); itGnb != gnbNodes.End(); itGnb++)
        { 
            /// NS_LOG_UNCOND ("inside gnbNodes loops ...");
            Ptr<MobilityModel> gnbMm = (*itGnb)->GetObject<MobilityModel>();
            Vector gnbPos = gnbMm->GetPosition();
            double x = uePos.x - gnbPos.x;
            double y = uePos.y - gnbPos.y;
            double distance = sqrt(x * x + y * y);

            if (distance < minDistance)
            {
                correctDistance = false;
                /// NS_LOG_UNCOND ("The UE node "<<(*itUe)->GetId() << " has wrong position, discarded.");
                break;
            }  
          else
            {
              // Ptr<Node> object = *j;
              // // get node name
              // std::string nodeName = Names::FindName (object);               
              m_outDistancesFile << (*itUe)->GetId () << "\t" << distance << "\t" 
                                 << (*itGnb)->GetId () << std::endl;
            }                                                          
        }


        if (correctDistance)
        {
            ueNodesFiltered.Add(*itUe);
        }
        if (ueNodesFiltered.GetN() >= numberOfUesToBeSelected)
        {
            // there are enough candidate UE nodes
            break;
        }
    }
    /// NS_LOG_UNCOND (ueNodesFiltered.GetN () << " nodes selected, selectWellPlacedUes done ...");
    return ueNodesFiltered;
}

// ------------------------------------------------------------
   ns3::NodeContainer DashFact::ue_positions_random (int ueCount, int ueHeight, const NodeContainer gnbNodes,
                        const NodeContainer ueNodes, double minDistance) {

       /// NS_LOG_UNCOND ("ENTER --> ue_positions_random ");

       MobilityHelper mobility;
       double minBigBoxX = -10.0; double minBigBoxY = -15.0;
       double maxBigBoxX = 110.0; double maxBigBoxY = 35.0;
   
       // Creating positions of the UEs according to the 3gpp TR 38.900 and
       // R11700144, uniformly randombly distributed in the rectangular area
       NodeContainer selectedUeNodes;
       for (uint8_t j = 0; j < 2; j++)
       {
           double minSmallBoxY = minBigBoxY + j * (maxBigBoxY - minBigBoxY) / 2;
   
           for (uint8_t i = 0; i < 6; i++)
           {
               double minSmallBoxX = minBigBoxX + i * (maxBigBoxX - minBigBoxX) / 6;
               Ptr<UniformRandomVariable> ueRandomVarX = CreateObject<UniformRandomVariable>();
   
               double minX = minSmallBoxX;             double minY = minSmallBoxY;
               double maxX = minSmallBoxX + (maxBigBoxX - minBigBoxX) / 6 - 0.0001;
               double maxY = minSmallBoxY + (maxBigBoxY - minBigBoxY) / 2 - 0.0001;
   
               Ptr<RandomBoxPositionAllocator> ueRandomRectPosAlloc = CreateObject<RandomBoxPositionAllocator>();
               ueRandomVarX->SetAttribute("Min", DoubleValue(minX));
               ueRandomVarX->SetAttribute("Max", DoubleValue(maxX));
               ueRandomRectPosAlloc->SetX(ueRandomVarX);
               Ptr<UniformRandomVariable> ueRandomVarY = CreateObject<UniformRandomVariable>();
               ueRandomVarY->SetAttribute("Min", DoubleValue(minY));
               ueRandomVarY->SetAttribute("Max", DoubleValue(maxY));
               ueRandomRectPosAlloc->SetY(ueRandomVarY);
               Ptr<ConstantRandomVariable> ueRandomVarZ = CreateObject<ConstantRandomVariable>();
               ueRandomVarZ->SetAttribute("Constant", DoubleValue(ueHeight));
               ueRandomRectPosAlloc->SetZ(ueRandomVarZ);
   
               uint8_t smallBoxIndex = j * 6 + i;
   
               NodeContainer smallBoxCandidateNodes;
               NodeContainer smallBoxGnbNode;
   
               smallBoxGnbNode.Add(gnbNodes.Get(smallBoxIndex));
   
               for (uint32_t n = smallBoxIndex * ueCount / 12;
                    n < smallBoxIndex * static_cast<uint32_t>(ueCount / 12) +
                            static_cast<uint32_t>(ueCount / 12);
                    n++)
               {
                   smallBoxCandidateNodes.Add(ueNodes.Get(n));
               }
               mobility.SetPositionAllocator(ueRandomRectPosAlloc);
               mobility.Install(smallBoxCandidateNodes);
               /// NS_LOG_UNCOND ("[ue] CALL  --> SelectWellPlacedUes ...");
               /// NS_LOG_UNCOND ("              " << smallBoxCandidateNodes.GetN() << " nodes in smallBoxCandidateNodes");               
               NodeContainer sn =
                   SelectWellPlacedUes(smallBoxCandidateNodes, smallBoxGnbNode, minDistance, 10);
               selectedUeNodes.Add(sn);
           }
       }
       return selectedUeNodes;
     }      
// --------------------------------------------------------





// ------------------------------------------------------------
   ns3::NodeContainer DashFact::agv_positions_random (int ueHeight, const NodeContainer gnbNodes,
                                                      const NodeContainer agvNodes, double minDistance) 
        // called using --> selectedAgvNodes = agv_positions_random (1, hUT, enbNodes, agvNodes, 0)
   {
      /// NS_LOG_UNCOND ("ENTER --> avg_positions_random ");      

      MobilityHelper mobility;
      double minBigBoxX = -10.0; double minBigBoxY = -15.0;
      double maxBigBoxX = 110.0; double maxBigBoxY = 35.0;

      double minX = minBigBoxX; double maxX = maxBigBoxX;
      double minY = minBigBoxY; double maxY = maxBigBoxY;      

      NodeContainer selectedAgvNodes, agvCandidateNodes;
      bool agvAccepted = false; 
      while (not agvAccepted)
        {
          Ptr<UniformRandomVariable> agvRandomVarX = CreateObject<UniformRandomVariable> ();                  
          agvRandomVarX->SetAttribute ("Min", DoubleValue (minX));
          agvRandomVarX->SetAttribute ("Max", DoubleValue (maxX));
    
          Ptr<UniformRandomVariable> agvRandomVarY = CreateObject<UniformRandomVariable> ();
          agvRandomVarY->SetAttribute ("Min", DoubleValue (minY));
          agvRandomVarY->SetAttribute ("Max", DoubleValue (maxY));
    
          Ptr<ConstantRandomVariable> agvRandomVarZ = CreateObject<ConstantRandomVariable> ();
          agvRandomVarZ->SetAttribute ("Constant", DoubleValue (ueHeight));
    
          Ptr<RandomBoxPositionAllocator> agvRandomRectPosAlloc = CreateObject<RandomBoxPositionAllocator> ();
          agvRandomRectPosAlloc->SetX (agvRandomVarX);
          agvRandomRectPosAlloc->SetY (agvRandomVarY);
          agvRandomRectPosAlloc->SetZ (agvRandomVarZ);
    
          mobility.SetPositionAllocator (agvRandomRectPosAlloc);
          mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel", 
                                    "Mode", StringValue("Time"),
                                    "Time", StringValue("2s"),
                                    "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=20.0]"),
                                    "Bounds", RectangleValue(Rectangle(minX, maxX, minY, maxY))); 

          for (uint32_t j = 0; j < agvNodes.GetN (); j++) {         
                   agvCandidateNodes.Add(agvNodes.Get(j));
          };
          /// NS_LOG_UNCOND ("      next, mobility.Install (agvCandidateNodes);");          
          /// NS_LOG_UNCOND ("            (" << agvCandidateNodes.GetN() << " nodes in agvCandidateNodes)");             
          mobility.Install (agvCandidateNodes);
          /// NS_LOG_UNCOND ("[agv] CALL  --> SelectWellPlacedUes (agvCandidateNodes, gnbNodes, minDistance, 1);");                      
          NodeContainer sn = SelectWellPlacedUes (agvCandidateNodes, gnbNodes, minDistance, 1);
      
          if (sn.GetN ()>0) { agvAccepted = true; selectedAgvNodes.Add (sn);}
          else { NS_LOG_UNCOND ("--> not accepted agv position");}
        }   
      
      if (selectedAgvNodes.GetN()>0) {
          for (uint32_t j = 0; j < selectedAgvNodes.GetN (); j++)
            {
              Vector agv_v = selectedAgvNodes.Get (j)->GetObject<MobilityModel> ()->GetPosition ();          
              NS_LOG_UNCOND ("- Selected (AGV) node (" << selectedAgvNodes.Get (j)->GetId () << ")" << \
                             "  :\tx=" << agv_v.x << "\t,y=" << agv_v.y << "\t,z=" << agv_v.z);
            }
        }    
      else { NS_LOG_UNCOND ("---> no AGV selected");}  
      return selectedAgvNodes;
     }      
// --------------------------------------------------------




// ------------------------------------------------------------
   ns3::NodeContainer DashFact::agv_SrcDst_random (int ueHeight, const NodeContainer gnbNodes,
                                                   const NodeContainer agvNodes, double minDistance, 
                                                   double simulationTime ) 
        // called using --> selectedAgvNodes = agv_positions_random (1, hUT, enbNodes, agvNodes, 0)
   {
      /// NS_LOG_UNCOND ("ENTER --> avg_positions_random ");      

      MobilityHelper mobility;
      double minBigBoxX = -10.0; double minBigBoxY = -15.0;
      double maxBigBoxX = 110.0; double maxBigBoxY = 35.0;

      double minX = minBigBoxX; double maxX = maxBigBoxX;
      double minY = minBigBoxY; double maxY = maxBigBoxY; 

      double velocityX;
      double velocityY;

      NodeContainer selectedAgvNodes, agvCandidateNodes;
      bool agvAccepted = false; 
      while (not agvAccepted)
        {
          Ptr<UniformRandomVariable> agvRandomVarX = CreateObject<UniformRandomVariable> ();                                         
          Ptr<UniformRandomVariable> agvRandomVarY = CreateObject<UniformRandomVariable> ();                    
          double x_Src = agvRandomVarX->GetValue(minX, maxX);
          double y_Src = agvRandomVarY->GetValue(minY, maxY);          
          double x_Dst = agvRandomVarX->GetValue(minX, maxX);
          double y_Dst = agvRandomVarY->GetValue(minY, maxY);

          // Calculate the distance between (x0, y0) and (x1, y1)
          double distance = std::sqrt(std::pow(x_Dst - x_Src, 2) + std::pow(y_Dst - y_Src, 2));          

          // Calculate the velocity
          velocityX = (x_Dst - x_Src) / simulationTime; // Velocity in the x-direction
          velocityY = (y_Dst - y_Src) / simulationTime; // Velocity in the y-direction

          // Print the results
              NS_LOG_UNCOND ("Starting Point: (" << x_Src << ", " << y_Src << ")" );
              NS_LOG_UNCOND ("Destination Point: (" << x_Dst << ", " << y_Dst << ")" );
              NS_LOG_UNCOND ("Distance: " << distance << " units" );
              NS_LOG_UNCOND ("Velocity: (" << velocityX << ", " << velocityY << ") units/s" <<
                             "(" << std::sqrt(std::pow(velocityX, 2) + std::pow(velocityY, 2)) << "m/s)" );
          
          // Set the initial position to (x0, y0)
              MobilityHelper mobility;
              Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator>();
              positionAlloc->Add(Vector(x_Src, y_Src, ueHeight));
              mobility.SetPositionAllocator(positionAlloc);
              mobility.SetMobilityModel("ns3::ConstantVelocityMobilityModel");                                      

          for (uint32_t j = 0; j < agvNodes.GetN (); j++) {         
                   agvCandidateNodes.Add(agvNodes.Get(j));
          };
          /// NS_LOG_UNCOND ("      next, mobility.Install (agvCandidateNodes);");          
          /// NS_LOG_UNCOND ("            (" << agvCandidateNodes.GetN() << " nodes in agvCandidateNodes)");             
          mobility.Install (agvCandidateNodes);
          /// NS_LOG_UNCOND ("[agv] CALL  --> SelectWellPlacedUes (agvCandidateNodes, gnbNodes, minDistance, 1);");                      
          NodeContainer sn = SelectWellPlacedUes (agvCandidateNodes, gnbNodes, minDistance, 1);
      
          if (sn.GetN ()>0) { agvAccepted = true; selectedAgvNodes.Add (sn);}
          else { NS_LOG_UNCOND ("--> not accepted agv position");}
        }   
      
      if (selectedAgvNodes.GetN()>0) {
          for (uint32_t j = 0; j < selectedAgvNodes.GetN (); j++)
            {
              Vector agv_v = selectedAgvNodes.Get (j)->GetObject<MobilityModel> ()->GetPosition ();          
              NS_LOG_UNCOND ("- Selected (AGV) node (" << selectedAgvNodes.Get (j)->GetId () << ")" << \
                             "  :\tx=" << agv_v.x << "\t,y=" << agv_v.y << "\t,z=" << agv_v.z);
            }

           Ptr<ConstantVelocityMobilityModel> agvMobility = selectedAgvNodes.Get(0)->GetObject<ConstantVelocityMobilityModel>();
           agvMobility->SetVelocity(Vector(velocityX, velocityY, 0.0));
        }    
      else { NS_LOG_UNCOND ("---> no AGV selected");}  
      return selectedAgvNodes;
     }      
// --------------------------------------------------------





// -------------------------------------------------------------------------
void DashFact::BuildFileNames (std::string tag) {

  std::string directoryName = m_outputDir;
  std::string filenameSinr =          BuildFileNameString (directoryName, "sinrs", tag);
  std::string agv_filenameSinr =      BuildFileNameString (directoryName, "agv-sinrs", tag);
  std::string filenameSnr =           BuildFileNameString (directoryName, "snrs", tag);
  std::string agv_filenameSnr =       BuildFileNameString (directoryName, "agv-snrs", tag);
  std::string filenameRssi =          BuildFileNameString (directoryName, "rssi", tag);
  std::string agv_filenameRssi =      BuildFileNameString (directoryName, "agv-rssi", tag);
  std::string filenameCoupling =      BuildFileNameString (directoryName, "coupling", tag);
  std::string agv_filenameCoupling =  BuildFileNameString (directoryName, "agv-coupling", tag);
  std::string filenamePositions =     BuildFileNameString (directoryName, "ue-positions", tag);
  std::string agv_filenamePositions = BuildFileNameString (directoryName, "agv-positions", tag);  
  std::string gnb_filenamePositions = BuildFileNameString (directoryName, "gnb-positions", tag);
  std::string filenameDistances =     BuildFileNameString (directoryName, "distances", tag);
  std::string filenameDashStats =     BuildFileNameString (directoryName, "dashStats", tag);


  std::string filenameUlMacStats = BuildFileNameString (directoryName, "UlMacStats", tag);
  std::string filenameDlMacStats = BuildFileNameString (directoryName, "DlMacStats", tag);
  Config::SetDefault ("ns3::MacStatsCalculator::UlOutputFilename", StringValue (filenameUlMacStats));
  Config::SetDefault ("ns3::MacStatsCalculator::DlOutputFilename", StringValue (filenameDlMacStats));

  // RLC
     std::string filenameUlRlcStats = BuildFileNameString (directoryName, "UlRlcStats", tag);  
     Config::SetDefault ("ns3::NrBearerStatsCalculator::UlRlcOutputFilename", StringValue (filenameUlRlcStats));  
     std::string filenameDlRlcStats = BuildFileNameString (directoryName, "DlRlcStats", tag);  
     Config::SetDefault ("ns3::NrBearerStatsCalculator::DlRlcOutputFilename", StringValue (filenameDlRlcStats));    
  // PDCP 
     std::string filenameUlPdcpStats = BuildFileNameString (directoryName, "UlPdcpStats", tag);  
     Config::SetDefault ("ns3::NrBearerStatsCalculator::UlPdcpOutputFilename", StringValue (filenameUlPdcpStats));  
     std::string filenameDlPdcpStats = BuildFileNameString (directoryName, "DlPdcpStats", tag);  
     Config::SetDefault ("ns3::NrBearerStatsCalculator::DlPdcpOutputFilename", StringValue (filenameDlPdcpStats));    


  // for NrPhyRx/TxTraces, we use tagz
  /// auto last_index = directoryName.size() - 19;
  /// auto myytag = directoryName.substr(18, last_index);
  // auto na = directoryName.size()-3;

  // NS_LOG_UNCOND ("tag = " << m_simTag);

  Config::SetDefault ("ns3::NrPhyRxTrace::SimTag", StringValue (std::to_string(m_simTag)));  
  // this is about NrPhyRxTrace::ReportCurrentCellRsrpSinrCallback function [/contrib/nr/helper/nr-phy-rx-trace.cc]
  //      Recall : 
  //      NrPhyRxTrace::ReportCurrentCellRsrpSinrCallback 
  //                    (Ptr<NrPhyRxTrace> phyStats, std::string path, uint16_t cellId, uint16_t rnti, double power, 
  //                    double avgSinr, uint16_t bwpId) 
  //                    m_rsrpSinrFile << "Time" << "\t" << "cellId" <<  "\t"
  //                                   << "bwpId" <<   "\t"
  //                                   << "avgSINR" << "\t"
  //                                   << "power" << std::endl;





  // Config::SetDefault ("ns3::NrPhyRxTrace::SimTag", StringValue (myytag));  
  // Config::SetDefault ("ns3::NrMacRxTrace::SimTag", UintegerValue (m_simTag));  
  // Config::SetDefault ("ns3::DashClient::outputDir", StringValue(m_outputDir));    

  m_outSinrFile.open (filenameSinr.c_str ()); m_outSinrFile.setf (std::ios_base::fixed);
  if (!m_outSinrFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameSinr);}

  m_agv_outSinrFile.open (agv_filenameSinr.c_str ()); m_agv_outSinrFile.setf (std::ios_base::fixed);
  if (!m_agv_outSinrFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << agv_filenameSinr);}


  m_outSnrFile.open (filenameSnr.c_str ()); m_outSnrFile.setf (std::ios_base::fixed);
  if (!m_outSnrFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameSnr); }

  m_agv_outSnrFile.open (agv_filenameSnr.c_str ()); m_agv_outSnrFile.setf (std::ios_base::fixed);
  if (!m_agv_outSnrFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << agv_filenameSnr); }

  m_outRssiFile.open (filenameRssi.c_str ()); m_outRssiFile.setf (std::ios_base::fixed);
  if (!m_outRssiFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameRssi); }

  m_agv_outRssiFile.open (agv_filenameRssi.c_str ()); m_agv_outRssiFile.setf (std::ios_base::fixed);
  if (!m_agv_outRssiFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << agv_filenameRssi); }

  m_outCouplingFile.open (filenameCoupling.c_str ()); m_outCouplingFile.setf (std::ios_base::fixed);
  if (!m_outCouplingFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameCoupling); }

  m_agv_outCouplingFile.open (agv_filenameCoupling.c_str ()); m_agv_outCouplingFile.setf (std::ios_base::fixed);
  if (!m_agv_outCouplingFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << agv_filenameCoupling); }

  m_outPositionsFile.open (filenamePositions.c_str ());
  m_outPositionsFile.setf (std::ios_base::fixed);
  if (!m_outPositionsFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenamePositions); }

  m_agv_outPositionsFile.open (agv_filenamePositions.c_str ());
  m_agv_outPositionsFile.setf (std::ios_base::fixed);
  if (!m_agv_outPositionsFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << agv_filenamePositions); }

  m_gnb_outPositionsFile.open (gnb_filenamePositions.c_str ());
  m_gnb_outPositionsFile.setf (std::ios_base::fixed);
  if (!m_gnb_outPositionsFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << gnb_filenamePositions);}

  m_outDistancesFile.open (filenameDistances.c_str ());
  m_outDistancesFile.setf (std::ios_base::fixed);
  if (!m_outDistancesFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameDistances);}

  m_outDashStatsFile.open (filenameDashStats.c_str ());
  m_outDashStatsFile.setf (std::ios_base::fixed);
  if (!m_outDashStatsFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameDashStats);}  
  // Config::SetDefault("ns3::DashClient::outputDir", StringValue(m_outputDir));  

} // ----------------------------------------------------------


   /// -------------------------------------------------------------------------------------
   /// void DashFact::PositionTracer (std::string context, Ptr<const MobilityModel> model) {
   ///    Vector position = model->GetPosition (); 
   ///    NS_LOG_UNCOND (context << " x = " << position.x << ", y = " << position.y <<
   ///                              ", velocity =" << model->GetVelocity ());
   ///                              // ", relative Speed (m/s) = " << model->GetRelativeSpeed ());
   ///    // int source; 
   ///    // source = ExtractNodesFromPositionContext (context);s
   ///    m_outAgvPositionsFile << Simulator::Now ().GetSeconds () << ","       
   ///                          << "agv0" << "," << position.x << ","
   ///                          << position.y << "," << model->GetVelocity () << "m/s" << std::endl;    
   /// } // ------------------------------------------------------------------------------ 



  // -------------------------------------------------------------------------------------
  // Function to print and log position and velocity
        void DashFact::PositionTracer (Ptr<Node> node)        
     // void PositionTracer (Ptr<Node> node, std::ofstream csvFile)        
       {
           Ptr<MobilityModel> mobility = node->GetObject<MobilityModel>();
           Vector pos = mobility->GetPosition();
           Vector vel = mobility->GetVelocity();
           double time = Simulator::Now().GetSeconds();       
       
           // Print to console
              std::cout << "Time: " << time << "s, "
                        << "Position: (" << pos.x << ", " << pos.y << "), "
                        << "Velocity: (" << vel.x << ", " << vel.y << ") m/s " 
                        << "(" << std::sqrt(std::pow(vel.x, 2) + std::pow(vel.y, 2)) << "m/s)" 
                        << std::endl;

           uint64_t imsi = node -> GetDevice (0)->GetObject<NrUeNetDevice> ()->GetImsi ();           
           uint64_t cellId = node -> GetDevice (0)->GetObject<NrUeNetDevice> ()->GetCellId ();                                             
           uint64_t cellIdPhy = node -> GetDevice (0)->GetObject<NrUeNetDevice> ()->GetPhy(0)->GetCellId ();
           uint64_t rntiPhy = node -> GetDevice (0)->GetObject<NrUeNetDevice> ()->GetPhy(0)->GetRnti ();


           // Write to CSV file           
              m_agv_outPositionsFile << time << "," << "agv0" << "," 
                                    << pos.x << "," << pos.y << "," 
                                    << std::sqrt(std::pow(vel.x, 2) + std::pow(vel.y, 2)) << "m/s," 
                                    << imsi << "," << cellId << "," << cellIdPhy << "," << rntiPhy 
                                    << std::endl;               
       
           // Schedule the next call to this function
              /// Simulator::Schedule(Seconds(1.0), &PositionTracer, node, csvFile);
              Simulator::Schedule(Seconds(0.1), &DashFact::PositionTracer, this, node);                            
       }   


  // --------------------------------------------------
  // Function to print and log position and velocity
     void DashFact::ChannelTracer (NodeContainer agvNodes, NodeContainer enbNodes)        
         {
            // NS_LOG_UNCOND ("ChannelTracer called at" << Simulator::Now ().GetSeconds ());            
            // Calculate the coupling loss for agv
            for (uint32_t i = 0; i <agvNodes.GetN (); ++i)
               {
                  Ptr<MobilityModel> rxMob = agvNodes.Get (i)->GetObject<MobilityModel> ();
                  static std::string nodeName = Names::FindName(agvNodes.Get (i));
                  uint32_t nodeId = agvNodes.Get (i)-> GetId ();
                  uint64_t imsi   = agvNodes.Get (i)-> GetDevice (0)->GetObject<NrUeNetDevice> ()->GetImsi ();           
                  uint64_t cellId = agvNodes.Get (i)-> GetDevice (0)->GetObject<NrUeNetDevice> ()->GetCellId ();                                                        
                  for (uint32_t j = 0; j < enbNodes.GetN (); j++) 
                        {    
                           Ptr<MobilityModel> txMob = enbNodes.Get (j)->GetObject<MobilityModel> ();
                           // check the channel condition
                           Ptr<ChannelCondition> cond = m_condModel->GetChannelCondition (txMob, rxMob);
                           cond->GetLosCondition();
                           double  propagationGainDb = m_propagationLossModel->CalcRxPower(0, txMob, rxMob);
                           double shadowingLoss = m_propagationLossModel->DoCalcRxPower (0.0, txMob, rxMob);
                           //NS_LOG_UNCOND ("     propagationGainDb=" << propagationGainDb << ", shadowingLoss=" << shadowingLoss); 
                           // timestamp ue_name nodeId imsi gnb_id
                           // gnb_x gnb_y ue_x ue_y gain_db shadowing_loss (pathloss(db))
                           m_agv_outCouplingFile << Simulator::Now ().GetSeconds () << "\t"                          
                                                << nodeName << "\t"
                                                << nodeId << "\t"
                                                << imsi << "\t"
                                                << enbNodes.Get (j)-> GetId () << "\t"
                                                << cellId << "\t"                                                                                
         
                                                << txMob->GetPosition ().x << "\t"
                                                << txMob->GetPosition ().y << "\t"
                                                << rxMob->GetPosition ().x << "\t"
                                                << rxMob->GetPosition ().y << "\t"
                                                << propagationGainDb << "\t"
                                                << shadowingLoss << std::endl; // pathloss [dB]                                                                                                                                                                                                                                             
                        }
               }
            Simulator::Schedule(Seconds(0.1), &DashFact::ChannelTracer, this, agvNodes, enbNodes);                               
       }  
  // --------------------------------------------------            



// ------------------------------------------------------------------------------------------------ 
   void DashFact::Run ( double hBS, double hUT, int users, uint16_t numerology, double centralFrequencyBand, 
                        double bandwidthBand, bool cellScan, double beamSearchAngleStep,
                        double totalTxPower, double target_dt, uint32_t bufferSpace, std::string window, 
                        double simulationTime, double serverStart, double clientStart, 
                        double serverStop, double clientStop,
                        std::string scenarioTag, std::string output_dir, uint8_t logging_level, 
                        uint8_t scenario, std::string indoorScenario, uint32_t mySeed)

{  

   NS_LOG_UNCOND ("DashFact::Run : indoorScenario : " << indoorScenario);
   m_loggingLevel = logging_level;
   m_outputDir = output_dir;
   m_scenarioTag = scenarioTag;

   // tag related to the random sim, used to differenciate between runs
   // m_outputDir = ./outputs/12nodes/2901/
   //               ./outputs/InF-SparseLow/7339/
   // auto last_index = m_outputDir.size() - 19;
   // m_simTag = std::stoi(m_outputDir.substr(18, last_index));// 
      m_simTag = mySeed; 


   // double r = 0.20; // Density clutter is 0.20 for SL and SH, 0.60 meters for DL and DH
   // double h_c = 2.00;//High clutter is 2.0 for SH and SL and 6.0 for DL and DH
   uint8_t timeValueMs = 5;
   Time timeRes = MilliSeconds(10); // time resolution


   SystemWallClockTimestamp m_stamp;  //!< Elapsed wallclock time.  
   BuildFileNames (".csv");   
   m_output_directory = output_dir; // Initialize_TcpTraceFiles (output_dir); 

   // Initialize_RtxTraceFile ();
   // Initialize_outSinrFile (); 
   // Initialize_PosTraceFile ();   
   // Initialize_couplingTraceFile (); 

   Initialize_someTraceFiles ();  


    // ------------------------------------------
    // Setup eNB base stations base station nodes
    // ------------------------------------------	
       NodeContainer enbNodes;
       // One base station in this simulation
          enbNodes.Create(12);
       // Creating positions of the gNB 
       // according to the 3gpp TR 38.900 Figure 7.2.-1
          Ptr<ListPositionAllocator> enbPositionAlloc = CreateObject<ListPositionAllocator>();
          for (uint8_t j = 0; j < 2; j++){
              for (uint8_t i = 0; i < 6; i++){
                	if ((indoorScenario == "InF-DenseHigh") or (indoorScenario == "InF-SparseHigh")) { 
                      // NS_LOG_UNCOND ("This is " << indoorScenario << " enb high = 8");
                      enbPositionAlloc->Add (Vector ( i * 20, j * 20, 8));                      
                  }  else { enbPositionAlloc->Add (Vector ( i * 20, j * 20, 1.5));
                            // NS_LOG_UNCOND ("This is " << indoorScenario << " enb high = 1.5");
                  }
              }   
            }
       // Setup mobility helper for base stations (immobile)
          MobilityHelper enbmobility;
          enbmobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
          enbmobility.SetPositionAllocator(enbPositionAlloc);
          enbmobility.Install(enbNodes);
    // ------------------------------------------


    // -----------
    // Setup UE
    // ------------------------------------------	
       // Node container for UEs
          NodeContainer ueNodes, agvNodes, allNodes, endNodes;
          ueNodes.Create(users); endNodes.Add (ueNodes);

       // Position end users
       // Creating positions of end users according to the 3gpp TR 38.900 Figure 7.2.-1
          bool randomness = true;
          if (!randomness){
               Ptr<ListPositionAllocator> uePositionAlloc = ue_positions_static (users, hUT, true);
               // Setup mobility helper for base stations (immobile)
                  MobilityHelper uemobility;
                  uemobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
                  uemobility.SetPositionAllocator(uePositionAlloc);
                  uemobility.Install(ueNodes);
          }
          else {
             ns3::NodeContainer selectedNodes;
             selectedNodes = ue_positions_random (users, hUT, enbNodes, ueNodes, 0);
             ueNodes = selectedNodes;
          }

          for (uint32_t j = 0; j < ueNodes.GetN (); j++) {
              Names::Add ("ue"+std::to_string(j), ueNodes.Get (j));
          }    

       // Create AGV node
          agvNodes.Create (1); 
          endNodes.Add(agvNodes);
          bool agv_scenario = true;
          if (agv_scenario){   
             ns3::NodeContainer selectedAgvNodes;
             /// selectedAgvNodes = agv_positions_random (1, hUT, enbNodes, agvNodes, 0);
             selectedAgvNodes = agv_SrcDst_random (hUT, enbNodes, agvNodes, 0, simulationTime);
             agvNodes = selectedAgvNodes;            
          }         

          Ptr<Node> agvNode = agvNodes.Get(0); ///Names::Add ("agvNode", agvNode);  
          for (uint32_t j = 0; j < agvNodes.GetN (); j++) {
              Names::Add ("agv"+std::to_string(j), agvNodes.Get (j));
          }                      
    // ------------------------------------------



   
    // ------------------------------------------
    // Setup Configuration
    // ------------------------------------------	

  // ------------------------------------------
  // To choose the sub-scenario of Indoor Factory
  // ------------------------------------------	  
   	BandwidthPartInfo::Scenario phy_scenario;
           if (indoorScenario == "InF-SparseLow")  { phy_scenario = BandwidthPartInfo::InF_SL;}
   	else if (indoorScenario == "InF-DenseLow")   { phy_scenario = BandwidthPartInfo::InF_DL;}
   	else if (indoorScenario == "InF-SparseHigh") { phy_scenario = BandwidthPartInfo::InF_SH;}
   	else if (indoorScenario == "InF-DenseHigh")  { phy_scenario = BandwidthPartInfo::InF_DH;}
   	else                                         { NS_ABORT_MSG("Unsupported scenario");}
  // ------------------------------------------	  
// ----------------------------
// Choosing the channel model
// ----------------------------
 	if  (indoorScenario == "InF-SparseLow") { 
      // create the channel condition model
 		   m_condModel = CreateObject<ThreeGppIndoorFactorySLChannelConditionModel>();
 		   // m_condModel->SetAttribute("r_density_SL", DoubleValue(r));
      // density clutter is 0.20 for SL
      // High clutter is 2.0 for SL
 		   m_condModel->SetAttribute("r_density_SL", DoubleValue(0.2));         
 	}
 	else if (indoorScenario == "InF-DenseLow") {
 		// create the channel condition model
 		   m_condModel = CreateObject<ThreeGppIndoorFactoryDLChannelConditionModel>();
 		   // m_condModel->SetAttribute("r_density_DL", DoubleValue(r));
         // density clutter is 0.60 meters for DL
         // High clutter is 6.0 for DL   
            m_condModel->SetAttribute("r_density_DL", DoubleValue(0.6));

 	}
 	else if (indoorScenario == "InF-SparseHigh") {
 		// create the channel condition model
 		   m_condModel = CreateObject<ThreeGppIndoorFactorySHChannelConditionModel>();
         // density clutter is 0.20 for SH,
         // High clutter is 2.0 for SH 
         // m_condModel->SetAttribute("r_density_SH", DoubleValue(r));
         // m_condModel->SetAttribute("hc_SH", DoubleValue(h_c));
            m_condModel->SetAttribute("r_density_SH", DoubleValue(0.2));
            m_condModel->SetAttribute("hc_SH",        DoubleValue(6.0));             
 	}
 	else if (indoorScenario == "InF-DenseHigh") {
 		// create the channel condition model
 		   m_condModel = CreateObject<ThreeGppIndoorFactoryDHChannelConditionModel>();
 		// m_condModel->SetAttribute("r_density_DH", DoubleValue(r));
 		// m_condModel->SetAttribute("hc_DH", DoubleValue(h_c));
      // density clutter is 0.60 meters for DH
      // High clutter is 6.0 for DH      
      // "hc_DH" --> "height of clutter"
 		   m_condModel->SetAttribute("r_density_DH", DoubleValue(0.6));
 		   m_condModel->SetAttribute("hc_DH", DoubleValue(6.0));               



 	}
   m_condModel->SetAttribute("UpdatePeriod", TimeValue(MilliSeconds(timeValueMs)));
	// Channel Model
	   Ptr<ThreeGppChannelModel> channelModel = CreateObject<ThreeGppChannelModel>();
	   channelModel->SetAttribute("Scenario", StringValue(indoorScenario));
	   channelModel->SetAttribute("Frequency", DoubleValue(centralFrequencyBand));
	   channelModel->SetAttribute("ChannelConditionModel", PointerValue(m_condModel));
   // Choosing the channel model
      if      (indoorScenario == "InF-SparseLow")  { m_propagationLossModel = CreateObject<ThreeGppIndoorFactorySLPropagationLossModel>();}
      else if (indoorScenario == "InF-DenseLow")   { m_propagationLossModel = CreateObject<ThreeGppIndoorFactoryDLPropagationLossModel>();}
      else if (indoorScenario == "InF-SparseHigh") { m_propagationLossModel = CreateObject<ThreeGppIndoorFactorySHPropagationLossModel>();}
      else if (indoorScenario == "InF-DenseHigh")  { m_propagationLossModel = CreateObject<ThreeGppIndoorFactoryDHPropagationLossModel>();}
      
      m_propagationLossModel->SetAttribute("Frequency", DoubleValue(centralFrequencyBand));
      m_propagationLossModel->SetAttribute("ShadowingEnabled", BooleanValue(true));
      m_propagationLossModel->SetAttribute("ChannelConditionModel", PointerValue(m_condModel));


       // NR simulation helpers
          Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
          Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
          Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
          nrHelper->SetBeamformingHelper(idealBeamformingHelper); nrHelper->SetEpcHelper(epcHelper);

       // some default values 
          Config::SetDefault ("ns3::LteEnbRrc::SrsPeriodicity", UintegerValue (320));
          // required for legacy code from the NR module
             Config::SetDefault("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue(999999999));          

       // Spectrum configuration
          BandwidthPartInfoPtrVector allBwps; CcBwpCreator ccBwpCreator; const uint8_t numCcPerBand = 1;
          /// enum BandwidthPartInfo::Scenario scenarioEnum = BandwidthPartInfo::InF_SL;
          /// BandwidthPartInfo::InH_OfficeOpen_LoS; //previous UMa;
          CcBwpCreator::SimpleOperationBandConf bandConf(centralFrequencyBand, bandwidthBand, numCcPerBand, phy_scenario);
          OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);
          nrHelper->InitializeOperationBand(&band);
          allBwps = CcBwpCreator::GetAllBwps({band});

       // Configure ideal beamforming
          if (cellScan) { 
            idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (CellScanBeamforming::GetTypeId ()));
            idealBeamformingHelper->SetBeamformingAlgorithmAttribute ("BeamSearchAngleStep", DoubleValue (beamSearchAngleStep));} 
          else { idealBeamformingHelper->SetAttribute ("BeamformingMethod", TypeIdValue (DirectPathBeamforming::GetTypeId ()));}

       // Configure scheduler
          /// nrHelper->SetSchedulerTypeId(NrMacSchedulerTdmaRR::GetTypeId());
          nrHelper->SetSchedulerTypeId (TypeId::LookupByName ("ns3::NrMacSchedulerTdmaPF"));          

       // Antenna Configuration, mine is diff then Ramous et al. 
       // I opted for mine ... 
   
          // Antenna configuration for UE
             nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(2));
             nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(4));
             nrHelper->SetUeAntennaAttribute("AntennaElement", PointerValue(CreateObject<IsotropicAntennaModel>()));
          // Antennas for the gNbs
             nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(8));
             nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
             nrHelper->SetGnbAntennaAttribute("AntennaElement", PointerValue(CreateObject<IsotropicAntennaModel>()));

       NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice(enbNodes, allBwps);
       NetDeviceContainer ueNetDev =  nrHelper->InstallUeDevice (ueNodes, allBwps);
       NetDeviceContainer agvNetDev = nrHelper->InstallUeDevice (agvNodes, allBwps);       

       // Assign random stream
          int64_t randomStream = 1;
          randomStream += nrHelper->AssignStreams(enbNetDev, randomStream);
          randomStream += nrHelper->AssignStreams(ueNetDev, randomStream);
          randomStream += nrHelper->AssignStreams(agvNetDev, randomStream);          

          for(uint32_t u = 0; u < enbNetDev.GetN(); ++u) {
               nrHelper->GetGnbPhy(enbNetDev.Get(u), 0)->SetAttribute ("NoiseFigure", DoubleValue (7));
               nrHelper->GetGnbPhy(enbNetDev.Get(u), 0)->SetTxPower(totalTxPower); // my old config
               /// nrHelper->GetGnbPhy(enbNetDev.Get(u), 0)->SetAttribute ("txPower", DoubleValue (10 * log10 (totalTxPower)));
               /// nrHelper->GetGnbPhy(enbNetDev.Get(u), 0)->SetAttribute ("Numerology", DoubleValue (numerology));
               nrHelper->GetGnbPhy(enbNetDev.Get(u), 0)->SetNumerology (numerology);
          }                                                              

          // my old value : 10, Ramous study : 9 
          for(uint32_t u = 0; u < ueNetDev.GetN(); ++u) {          
             nrHelper->GetUePhy (ueNetDev.Get (u), 0)->SetAttribute ("NoiseFigure", DoubleValue (9));                            
          }

          for(uint32_t u = 0; u < agvNetDev.GetN(); ++u) {          
             nrHelper->GetUePhy (agvNetDev.Get (u), 0)->SetAttribute ("NoiseFigure", DoubleValue (9));                            
          }

       // When all the configuration is done, explicitly call UpdateConfig ()
          for (auto it = enbNetDev.Begin(); it != enbNetDev.End(); ++it)  { DynamicCast<NrGnbNetDevice>(*it)->UpdateConfig();}
          for (auto it =  ueNetDev.Begin(); it !=  ueNetDev.End(); ++it)  { DynamicCast<NrUeNetDevice>(*it)->UpdateConfig();}
          for (auto it = agvNetDev.Begin(); it != agvNetDev.End(); ++it)  { DynamicCast<NrUeNetDevice>(*it)->UpdateConfig();}            
    // -----------------------------------------------------------	   




    // ------------------------------------------
    // Setup Internet
    // ------------------------------------------	

       // Create internet and install IP stack on UEs (PGW - Packet Gateway)
          NodeContainer remoteHostContainer; remoteHostContainer.Create(1);
          Ptr<Node> remoteHost = remoteHostContainer.Get(0); endNodes.Add(remoteHost);
          Names::Add ("remoteHost", remoteHost);
          InternetStackHelper internet; internet.Install(remoteHostContainer);

       // Connect Remote host to pgw and setup routing
          allNodes.Add (NodeContainer::GetGlobal ());
          Ptr<Node> pgw = epcHelper->GetPgwNode (); Names::Add ("pgw", pgw);
          Ptr<Node> sgw = epcHelper->GetSgwNode (); Names::Add ("sgw", sgw);
          Ptr<Node> mme = allNodes.Get (allNodes.GetN ()-2); Names::Add ("mme", mme);                    

          PointToPointHelper p2ph; 
		    p2ph.SetDeviceAttribute("DataRate", DataRateValue(DataRate("100Gb/s")));
          p2ph.SetDeviceAttribute("Mtu", UintegerValue(2500));
          p2ph.SetChannelAttribute("Delay", TimeValue(Seconds(0)));// previous Seconds(0.010)

          NetDeviceContainer internetDevices = p2ph.Install(pgw, remoteHost);
          Ipv4AddressHelper ipv4h;
          ipv4h.SetBase("1.0.0.0", "255.0.0.0");
          Ipv4InterfaceContainer internetIPInterfaces = ipv4h.Assign(internetDevices);
          Ipv4Address remoteHostAddr = internetIPInterfaces.GetAddress(1);
          Ipv4StaticRoutingHelper ipv4RoutingHelper;
          Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting(remoteHost->GetObject<Ipv4>());
          remoteHostStaticRouting->AddNetworkRouteTo(Ipv4Address("7.0.0.0"), Ipv4Mask("255.0.0.0"), 1);

          internet.Install(ueNodes);
          Ipv4InterfaceContainer ueIpIface;
          ueIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(ueNetDev));   

          internet.Install(agvNodes);
          Ipv4InterfaceContainer agvIpIface;
          agvIpIface = epcHelper->AssignUeIpv4Address(NetDeviceContainer(agvNetDev));                                                                                       
    // -----------------------------------------------------------------------------


    // ---------------------------
    // Setup Application Layer
    // ------------------------------------------	
       uint16_t port = 80;
       ApplicationContainer clientApps, agv_clientApps; ApplicationContainer serverApps, agv_serverApps;

       //  targetDt : Target time difference between receiving and playing a frame. [s].
       //  window   : Window for measuring the average throughput. [s].
       //  bufferSpace  : Space in bytes that is used for buffering the video

  // Configuration parameters for UL and DL Traffic parameters for the Flows 

       if (scenario == 1) 
           { 
               // downlink scenario
               // Client Side 
                  for(uint32_t u = 0; u < ueNodes.GetN(); ++u) {
                      Ptr<Node> ueNode = ueNodes.Get(u);
                      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(ueNode->GetObject<Ipv4>());
                      ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
                      // Apply Dash Algorithm
                         DashClientHelper client ("ns3::TcpSocketFactory", InetSocketAddress(remoteHostAddr, port), "ns3::FdashClient");
                         client.SetAttribute ("VideoId", UintegerValue(1));
                         client.SetAttribute ("TargetDt", TimeValue (Seconds(target_dt)));
                         client.SetAttribute ("window", TimeValue(Time(window)));
                         client.SetAttribute("bufferSpace", UintegerValue(bufferSpace));
                         client.SetAttribute("outputDir", UintegerValue(bufferSpace));
                         clientApps.Add(client.Install(ueNodes.Get(u)));
                   }
               // Server side 
                  DashServerHelper server ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
                  serverApps = server.Install(remoteHost);
                  nrHelper->AttachToClosestEnb(ueNetDev, enbNetDev);
           }
       else 
           {
               if (scenario == 2) 
                  {
                    // uplink scenario
                    // Client Side 
                       // Simulator::Schedule (Seconds (0.1), &set_RTTOutdir, output_dir);
                       for(uint32_t u = 0; u < ueNodes.GetN(); ++u) 
                           {
                               Ptr<Node> ueNode = ueNodes.Get(u);
                               Ptr<Ipv4> node_ipv4 = ueNode->GetObject<Ipv4>();                      
                               Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(node_ipv4);
                               ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
                               /// Ipv4Address ipv4Address = node_ipv4->GetAddress (1, 0).GetLocal ();                                                                       

                               // Time to send the actual traffic flow
                                  Ptr<TsnFlow> myFlow_n = CreateObject<TsnFlow> ();
                               // GenerateFlow ( std::string myPath, Ipv4Address myInterface, Ptr<Node> DstNode, Time myStart, 
                               //                Time myStop, Ptr<Node> SrcNode, uint32_t nPackets, uint32_t packetSize,
                               //                DataRate dataRate, Time periodicity(ms))
                                  uint16_t sinkPort = 2000+u;
                                  myFlow_n->GenerateFlow (sinkPort, remoteHostAddr, remoteHost, 
                                                          Seconds (clientStart), Seconds (clientStop), ueNode, 100, 36,
                                                       // Seconds (0.0001), Seconds (clientStop), ueNode, 100, 100,
                                                          DataRate (100), MilliSeconds(8));

                               NS_LOG_INFO (  "new traffic flow entry; source =" <<
                                               Names::FindName(ueNode) << ", dst = " << Names::FindName(remoteHost) << 
                                              "(start=" << Seconds (clientStart) << ", " <<                              
                                              "(end=" << Seconds (clientStop) << "), port=" << sinkPort);                              
                           }

                       nrHelper->AttachToClosestEnb(ueNetDev, enbNetDev);  
                       for(uint32_t u = 0; u < agvNodes.GetN(); ++u) 
                           {
                               Ptr<Node> agvNode = agvNodes.Get(u);
                               Ptr<Ipv4> agv_node_ipv4 = agvNode->GetObject<Ipv4>();                      
                               Ptr<Ipv4StaticRouting> agvStaticRouting = ipv4RoutingHelper.GetStaticRouting(agv_node_ipv4);
                               agvStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
                               Ipv4Address agv_ipv4Address = agv_node_ipv4->GetAddress (1, 0).GetLocal ();                          

                               // Apply Dash Algorithm
                                  DashClientHelper 
                                  agv_client ("ns3::TcpSocketFactory", InetSocketAddress(agv_ipv4Address, port), \
                                              "ns3::FdashClient");
                                  agv_client.SetAttribute ("VideoId", UintegerValue(1));
                                  agv_client.SetAttribute ("TargetDt", TimeValue (Seconds(target_dt)));
                                  agv_client.SetAttribute ("window", TimeValue(Time(window)));
                                  NS_LOG_INFO ("(run) mytag :" << m_simTag);
                                  // agv_client.SetAttribute ("outputDir", StringValue(std::string(m_outputDir)));
                                  agv_client.SetAttribute("mytag", UintegerValue(m_simTag));
                                  agv_client.SetAttribute("bufferSpace", UintegerValue(bufferSpace));
                                  agv_clientApps.Add(agv_client.Install(remoteHost));

                               // Server side 
                                  DashServerHelper agv_server ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
                                  agv_serverApps = agv_server.Install(agvNode);
                            }
                        nrHelper->AttachToClosestEnb(agvNetDev, enbNetDev);         
                  }          
               else  
                  {
                    // uplink scenario
                    // Client Side 
                       // Simulator::Schedule (Seconds (0.1), &set_RTTOutdir, output_dir);
         
                       for(uint32_t u = 0; u < ueNodes.GetN(); ++u) 
                           {
                               Ptr<Node> ueNode = ueNodes.Get(u);
                               Ptr<Ipv4> node_ipv4 = ueNode->GetObject<Ipv4>();                      
                               Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(node_ipv4);
                               ueStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
                               Ipv4Address ipv4Address = node_ipv4->GetAddress (1, 0).GetLocal ();                          
            
                               // Apply Dash Algorithm
                                  DashClientHelper client ("ns3::TcpSocketFactory", InetSocketAddress(ipv4Address, port), "ns3::FdashClient");
                                  client.SetAttribute ("VideoId", UintegerValue(1));
                                  client.SetAttribute ("TargetDt", TimeValue (Seconds(target_dt)));
                                  client.SetAttribute ("window", TimeValue(Time(window)));
                                  client.SetAttribute("bufferSpace", UintegerValue(bufferSpace));
                                  clientApps.Add(client.Install(remoteHost));
            
                               // Server side 
                                  DashServerHelper server ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
                                  serverApps = server.Install(ueNode);
                           }
      
                       nrHelper->AttachToClosestEnb(ueNetDev, enbNetDev);  
                       for(uint32_t u = 0; u < agvNodes.GetN(); ++u) 
                           {
                               Ptr<Node> agvNode = agvNodes.Get(u);
                               Ptr<Ipv4> agv_node_ipv4 = agvNode->GetObject<Ipv4>();                      
                               Ptr<Ipv4StaticRouting> agvStaticRouting = ipv4RoutingHelper.GetStaticRouting(agv_node_ipv4);
                               agvStaticRouting->SetDefaultRoute(epcHelper->GetUeDefaultGatewayAddress(), 1);
                               Ipv4Address agv_ipv4Address = agv_node_ipv4->GetAddress (1, 0).GetLocal ();                          
            
                               // Apply Dash Algorithm
                                  DashClientHelper agv_client ("ns3::TcpSocketFactory", InetSocketAddress(agv_ipv4Address, port), "ns3::FdashClient");
                                  agv_client.SetAttribute ("VideoId", UintegerValue(1));
                                  agv_client.SetAttribute ("TargetDt", TimeValue (Seconds(target_dt)));
                                  agv_client.SetAttribute ("window", TimeValue(Time(window)));
                                  agv_client.SetAttribute("bufferSpace", UintegerValue(bufferSpace));
                                  agv_clientApps.Add(agv_client.Install(remoteHost));
            
                               // Server side 
                                  DashServerHelper agv_server ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
                                  agv_serverApps = agv_server.Install(agvNode);
                            }
                        nrHelper->AttachToClosestEnb(agvNetDev, enbNetDev);                                                                                  
                   } // else if (scenario == 2)
            }// else if (scenario == 1) 

       // Setup Apps start time and stop time
          serverApps.Start(Seconds(serverStart)); // 00
          clientApps.Start(Seconds(clientStart)); // 0.25
          serverApps.Stop(Seconds(serverStop)); // serverStop
          clientApps.Stop(Seconds(clientStop)); // clientStop 

          agv_serverApps.Start(Seconds(serverStart)); // 00
          agv_clientApps.Start(Seconds(clientStart)); // 0.25
          agv_serverApps.Stop(Seconds(serverStop)); // simulationTime
          agv_clientApps.Stop(Seconds(clientStop)); // serverStop - 0.25


          // ---------
	       //  Traces
          // ----------------------------
          // EnableDlDataPhyTraces();
          // EnableDlCtrlPhyTraces();
          // EnableUlPhyTraces();
          // EnableRlcSimpleTraces();
          // EnableRlcE2eTraces();
          // EnablePdcpSimpleTraces();
          // EnablePdcpE2eTraces();
          // EnableGnbPhyCtrlMsgsTraces();
          // EnableUePhyCtrlMsgsTraces();
          // EnableGnbMacCtrlMsgsTraces();
          // EnableUeMacCtrlMsgsTraces();
          // EnableDlMacSchedTraces();
          // EnableUlMacSchedTraces();
          // EnablePathlossTraces();
             nrHelper->EnableTraces();

    // ----------------------------
    // SNR + RSSI info 
    // --------------------------------------        
      for (uint32_t i = 0; i < ueNetDev.GetN (); i++)
          {
              Ptr<NrSpectrumPhy > ue1SpectrumPhy = DynamicCast<NrUeNetDevice> (ueNetDev.Get (i))->GetPhy (0)->GetSpectrumPhy ();
              Ptr<NrInterference> ue1SpectrumPhyInterference = ue1SpectrumPhy->GetNrInterference ();
              NS_ABORT_IF (!ue1SpectrumPhyInterference);
              ue1SpectrumPhyInterference->TraceConnectWithoutContext ("RssiPerProcessedChunk", 
                                                                      MakeBoundCallback (&UeRssiPerProcessedChunkTrace, this));
              ue1SpectrumPhyInterference->TraceConnectWithoutContext ( "SnrPerProcessedChunk", \
                                           MakeBoundCallback (&SnrPerProcessedChunkTrace, this));                                                                      
          }

      for (uint32_t i = 0; i < agvNetDev.GetN (); i++)
          {
              Ptr<NrSpectrumPhy > agv1SpectrumPhy = DynamicCast<NrUeNetDevice> (agvNetDev.Get (i))->GetPhy (0)->GetSpectrumPhy ();
              Ptr<NrInterference> agv1SpectrumPhyInterference = agv1SpectrumPhy->GetNrInterference ();
              NS_ABORT_IF (!agv1SpectrumPhyInterference);

              // Rssi per processed chunk
              agv1SpectrumPhyInterference->TraceConnectWithoutContext ( "RssiPerProcessedChunk", \
                                           MakeBoundCallback (&AgvRssiPerProcessedChunkTrace, this));
              // Snr per processed chunk
              agv1SpectrumPhyInterference->TraceConnectWithoutContext ( "SnrPerProcessedChunk", \
                                           MakeBoundCallback (&AgvSnrPerProcessedChunkTrace, this));

          }          

      // Calculate the coupling loss for uez
         for (uint32_t i = 0; i <ueNodes.GetN (); ++i)
             {
                 Ptr<MobilityModel> rxMob = ueNodes.Get (i)->GetObject<MobilityModel> ();
                 uint32_t nodeId = ueNodes.Get (i)-> GetId ();
                 uint64_t imsi = ueNodes.Get (i)-> GetDevice (0)->GetObject<NrUeNetDevice> ()->GetImsi ();           
                 uint64_t cellId = ueNodes.Get (i)-> GetDevice (0)->GetObject<NrUeNetDevice> ()->GetCellId ();                                             
                 for (uint32_t j = 0; j < enbNodes.GetN (); j++) 
                     {    
                         Ptr<MobilityModel> txMob = enbNodes.Get (j)->GetObject<MobilityModel> ();
                         // check the channel condition
                         Ptr<ChannelCondition> cond = m_condModel->GetChannelCondition (txMob, rxMob);
                         cond->GetLosCondition();
                         double  propagationGainDb = m_propagationLossModel->CalcRxPower(0, txMob, rxMob);
                         double shadowingLoss = m_propagationLossModel->DoCalcRxPower (0.0, txMob, rxMob);
                         // timestamp ue_name nodeId imsi gnb_id
                         // gnb_x gnb_y ue_x ue_y gain_db shadowing_loss (pathloss(db))
                         m_outCouplingFile << Simulator::Now ().GetSeconds () << "\t"                          
                                           << Names::FindName(ueNodes.Get (i)) << "\t"
                                           << nodeId << "\t"
                                           << imsi << "\t"
                                           << enbNodes.Get (j)-> GetId () << "\t"
                                           << cellId << "\t"                                           
                  
                                           << txMob->GetPosition ().x << "\t"
                                           << txMob->GetPosition ().y << "\t"
                                           << rxMob->GetPosition ().x << "\t"
                                           << rxMob->GetPosition ().y << "\t"
                                           << propagationGainDb << "\t"
                                           << shadowingLoss << std::endl; // pathloss [dB]
                     }
         }

      Simulator::Schedule(Seconds(0.5), &DashFact::ChannelTracer, this, agvNodes, enbNodes);                   

      
      m_outCouplingFile.close ();
      // m_agv_outCouplingFile.close ();
      




          // Schedule the first call to print position and velocity
             Simulator::Schedule(Seconds(0.0), &DashFact::PositionTracer, this, agvNodes.Get (0));


             // p2ph.EnablePcapAll("dash_fact_p2ph", true);
             // internet.EnablePcapIpv4 ("remoteHost.pcap", "remoteHost", 1, true);             
             // internet.EnablePcapIpv4 ("ue1.pcap", "ue1", 1, true);             

             // TCP trace 
                // Config::Connect("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*", MakeCallback(&CourseChange));
                std::string rtt_tr_file_name = "rtt_traceFile";  m_firstRtt = true;              
                if (rtt_tr_file_name.compare ("") != 0)
                  {
                       // Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), &DashFact::TraceRTX, this);
                       // Simulator::Schedule (Seconds(serverStart + 1), &TraceRto);
                          // Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), &DashFact::TraceBytesInFlight, this);                       
                          // Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), &DashFact::TraceNextTxSequence, this);                       
                          /// -->    Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), &DashFact::TraceTCP, this);                       
                          /// Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), &DashFact::TraceTCPInt, this);                       
                          /// Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), 
                          ///                   &Config::Connect ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/RTO", 
                          ///                   &DashFact::TraceRTX, this);
                  }                


             // Trace for SINR (another format)
                for (uint32_t i = 0; i < ueNetDev.GetN(); i++) {
                     Ptr<NrSpectrumPhy > ue1SpectrumPhy = DynamicCast<NrUeNetDevice> (ueNetDev.Get (i))->GetPhy (0)->GetSpectrumPhy ();
                     ue1SpectrumPhy->TraceConnectWithoutContext ("RxPacketTraceEnb", MakeBoundCallback (&UeReceptionTrace, this));                                                                                    
                }                       

             // Trace for SINR (another format)
                for (uint32_t i = 0; i < agvNetDev.GetN(); i++) {                                          
                     auto agvPhy = DynamicCast<NrUeNetDevice>(agvNetDev.Get(i))->GetPhy(0);
                     Ptr<NrSpectrumPhy> agv1SpectrumPhy = agvPhy->GetSpectrumPhy();
                     agv1SpectrumPhy->TraceConnectWithoutContext("RxPacketTraceUe", MakeBoundCallback(&AgvReceptionTrace, this));
                }                
                
             // -------------------------------------------------------------
             // Log ueNodes and enbNodes positions and respective distances
             // -------------------------------------------------------------	                               
                for (uint32_t j = 0; j < ueNodes.GetN (); j++) {
                    // Names::Add ("ue"+std::to_string(j), ueNodes.Get (j));
                    uint32_t nodeId = ueNodes.Get (j)-> GetId ();
                    uint64_t imsi = ueNodes.Get (j)-> GetDevice (0)->GetObject<NrUeNetDevice> ()->GetImsi ();           
                    Vector v = ueNodes.Get (j)->GetObject<MobilityModel> ()->GetPosition ();
                    m_outPositionsFile   << Names::FindName(ueNodes.Get (j)) << "\t" 
                                         << nodeId << "\t" 
                                         << imsi << "\t" 
                                         << v.x << "\t" << v.y << "\t" << v.z << std::endl;
                  }

                for (uint32_t j = 0; j < agvNodes.GetN (); j++) {
                    // Names::Add ("agv"+std::to_string(j), agvNodes.Get (j));
                    uint32_t nodeId = agvNodes.Get (j)-> GetId ();
                    uint64_t imsi = agvNodes.Get (j)-> GetDevice (0)->GetObject<NrUeNetDevice> ()->GetImsi ();           
                    Vector v = agvNodes.Get (j)->GetObject<MobilityModel> ()->GetPosition ();
                    m_outPositionsFile   << Names::FindName(agvNodes.Get (j)) << "\t" 
                                         << nodeId << "\t" 
                                         << imsi << "\t" 
                                         << v.x << "\t" << v.y << "\t" << v.z << std::endl;
                  }                  
              
                for (uint32_t j = 0; j < enbNodes.GetN (); j++) {
                    Names::Add ("gnb"+std::to_string(j), enbNodes.Get (j));           
                    Vector v = enbNodes.Get (j)->GetObject<MobilityModel> ()->GetPosition ();
                    m_gnb_outPositionsFile << Names::FindName(enbNodes.Get (j)) << "\t" 
                                          << enbNodes.Get (j)->GetId () << "\t" 
                                          << v.x << "\t" << v.y << "\t" << v.z << std::endl;
                  }
             // -------------------------------------------


             // FlowMonitor stuffs [Part ONE]
                // NS_LINFOOND ("FlowMonitor stuffs... ");
                FlowMonitorHelper flowmonHelper;
                NodeContainer endpointNodes;
                endpointNodes.Add (remoteHost);
                endpointNodes.Add (ueNodes);
                endpointNodes.Add (agvNodes);                
                Ptr<ns3::FlowMonitor> monitor = flowmonHelper.Install (endpointNodes);  

          // ------------------------------------------	   


    // ----------------------------
    // Log Nodes at the second (5)
    // --------------------------------------        
       if (m_loggingLevel >= 3) {
           Simulator::Schedule (Seconds (0.1), &LogNodes, allNodes, false);
           /// Simulator::Schedule (Seconds (1), &GetNodeName_v2, ueNodes, Ipv4Address("7.0.0.5"));
           /// NS_INFOCOND (GetNodeName_v2 (ueContainer, t.sourceAddress));
       }  // --------------------------------------------------------------------------                    
    // ---------



    // ---------------------------------------------------
	 // Simulator Start | Stop, Duration and Log routines
    // ---------------------------------------------------	   
       Simulator::Stop(Seconds(simulationTime));
       m_stamp.Stamp ();
       std::string startStamp = m_stamp.ToString ();
       Simulator::Run();
    // ------------------




    // ------------------------------------
    // Traces (2) : Stats related to Dash  
    // ------------------------------------------------------------------------              
         m_outDashStatsFile << "node,InterruptionTime,interruptions,framesPlayed,avgRateByFrame,avgRateBytes,"
                            << "maxRate,minminRate,minRate,avgRate,avgDt,estAvgBufferTime,changes,initBuffer,queueLength,queueLengthInBytes" 
                            << std::endl;              
   
         // Get the Dash Algorithm stat back
            // NS_LOG_UNCOND ("time to output stats for (normal) dash flows (nb clientApps = "<< clientApps.GetN () << ")");                  
            // for (int k = 0; k < users; k++) {
            //     Ptr<DashClient> app = DynamicCast<DashClient> (clientApps.Get(k));              
            //     m_outDashStatsFile << k << "," << app->GetStatsCsv ();                            
            // }

         // Get the Dash Algorithm stat back
            NS_LOG_UNCOND ("time to output stats for agv dash flows (nb agv_clientApps = "<< agv_clientApps.GetN () << ")");         
            /// for (int k = users; k < users + int(agvNodes.GetN ()); k++) {
            for (int k = 0; k < int(agvNodes.GetN ()); k++) {               
                NS_LOG_UNCOND ("    - stats for dashflow nb ("<< k << ")");
                Ptr<DashClient> app = DynamicCast<DashClient> (agv_clientApps.Get(k));              
                m_outDashStatsFile << users+k << "," << app->GetStatsCsv ();                            
            }            
    // ----------------------------------------------------------------------------


    // ------------------------------------
    // Traces (2) : Stats related to Dash  
    // ------------------------------------------------------------------------              
       m_stamp.Stamp ();
       std::string endStamp = m_stamp.ToString ();
       time_t duration = m_stamp.GetInterval ();

       if (m_loggingLevel >= 2) {
           NS_LOG_INFO ("    -----------------");          
           NS_LOG_INFO ("    Start wall clock: " << startStamp << std::endl);                    
           NS_LOG_INFO ("    ------------");
           NS_LOG_INFO ("   ( SIMULATION )");
           NS_LOG_INFO ("    ------------");
           NS_LOG_INFO ("   Start in : " << startStamp);
           NS_LOG_INFO ("     End in : " << endStamp);
           NS_LOG_INFO ("   Duration : " << duration << "s\n");
           NS_LOG_INFO ("   -----------");
           NS_LOG_INFO ("  ( Statistics )");
           NS_LOG_INFO ("   -----------");
        }
       Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
       LogResults (classifier, monitor, Seconds (simulationTime), Seconds (clientStart), 1, startStamp,
                   std::to_string (duration), endNodes, m_scenarioTag, m_outputDir);

       Simulator::Destroy();
}

