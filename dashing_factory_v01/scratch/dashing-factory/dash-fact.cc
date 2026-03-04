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
#include "dash-fact.utils.h"

#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("dash-fact");


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
    m_outSnrFile.close ();
    m_outRssiFile.close ();
    m_outUePositionsFile.close ();
    m_outEnbPositionsFile.close ();
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
                       << params.m_cellId << "\t" << params.m_rnti 
                       << "\t" << 10 * log10(params.m_sinr)
                       << std::endl; }
// ----------------------------------



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
{
    NodeContainer ueNodesFiltered;
    bool correctDistance = true;

    for (NodeContainer::Iterator itUe = ueNodes.Begin(); itUe != ueNodes.End(); itUe++)
    {
        correctDistance = true;
        Ptr<MobilityModel> ueMm = (*itUe)->GetObject<MobilityModel>();
        Vector uePos = ueMm->GetPosition();

        for (NodeContainer::Iterator itGnb = gnbNodes.Begin(); itGnb != gnbNodes.End(); itGnb++)
        { 
            Ptr<MobilityModel> gnbMm = (*itGnb)->GetObject<MobilityModel>();
            Vector gnbPos = gnbMm->GetPosition();
            double x = uePos.x - gnbPos.x;
            double y = uePos.y - gnbPos.y;
            double distance = sqrt(x * x + y * y);

            if (distance < minDistance)
            {
                correctDistance = false;
                // NS_LOG("The UE node "<<(*itUe)->GetId() << " has wrong position, discarded.");
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
    return ueNodesFiltered;
}

// ------------------------------------------------------------
   ns3::NodeContainer DashFact::ue_positions_random (int ueCount, int ueHeight, const NodeContainer gnbNodes,
                        const NodeContainer ueNodes, double minDistance) {
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
               NodeContainer sn =
                   SelectWellPlacedUes(smallBoxCandidateNodes, smallBoxGnbNode, minDistance, 10);
               selectedUeNodes.Add(sn);
           }
       }
       return selectedUeNodes;
     }      
// --------------------------------------------------------


// -------------------------------------------------------------------------
void DashFact::BuildFileNames (std::string tag) {

  std::string directoryName = m_outputDir;
  std::string filenameSinr = BuildFileNameString (directoryName, "sinrs", tag);
  std::string filenameSnr = BuildFileNameString (directoryName, "snrs", tag);
  std::string filenameRssi = BuildFileNameString (directoryName, "rssi", tag);
  std::string filenameUePositions = BuildFileNameString (directoryName, "ue-positions", tag);
  std::string filenameGnbPositions = BuildFileNameString (directoryName, "gnb-positions", tag);
  std::string filenameDistances = BuildFileNameString (directoryName, "distances", tag);
  std::string filenameDashStats = BuildFileNameString (directoryName, "dashStats", tag);

  m_outSinrFile.open (filenameSinr.c_str ()); m_outSinrFile.setf (std::ios_base::fixed);
  if (!m_outSinrFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameSinr);}

  m_outSnrFile.open (filenameSnr.c_str ()); m_outSnrFile.setf (std::ios_base::fixed);
  if (!m_outSnrFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameSnr); }

  m_outRssiFile.open (filenameRssi.c_str ()); m_outRssiFile.setf (std::ios_base::fixed);
  if (!m_outRssiFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameRssi); }

  m_outUePositionsFile.open (filenameUePositions.c_str ());
  m_outUePositionsFile.setf (std::ios_base::fixed);
  if (!m_outUePositionsFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameUePositions); }

  m_outEnbPositionsFile.open (filenameGnbPositions.c_str ());
  m_outEnbPositionsFile.setf (std::ios_base::fixed);
  if (!m_outEnbPositionsFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameGnbPositions);}

  m_outDistancesFile.open (filenameDistances.c_str ());
  m_outDistancesFile.setf (std::ios_base::fixed);
  if (!m_outDistancesFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameDistances);}

  m_outDashStatsFile.open (filenameDashStats.c_str ());
  m_outDashStatsFile.setf (std::ios_base::fixed);
  if (!m_outDashStatsFile.is_open ()) { NS_ABORT_MSG ("Can't open file " << filenameDashStats);}  

} // ----------------------------------------------------------



// ------------------------------------------------------------------------------------------------ 
   void DashFact::Run ( double hBS, double hUT, int users, double frequency, double bandwidth, 
                        double txPower, double target_dt, uint32_t bufferSpace, std::string window, 
                        double simulationTime, double serverStart, double clientStart, 
                        double serverStop, double clientStop,
                        std::string simTag, std::string output_dir, uint8_t logging_level, 
                        uint8_t scenario )

{  
   m_simTag = simTag;
   m_loggingLevel = logging_level;
   m_outputDir = output_dir;
   SystemWallClockTimestamp m_stamp;  //!< Elapsed wallclock time.  
   BuildFileNames (".csv");   
   m_output_directory = output_dir; Initialize_TcpTraceFiles (output_dir); 
   // Initialize_RtxTraceFile ();   

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
                  enbPositionAlloc->Add (Vector ( i * 20, j * 20, hBS));
                }
            }
       // Setup mobility helper for base stations (immobile)
          MobilityHelper enbmobility;
          enbmobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
          enbmobility.SetPositionAllocator(enbPositionAlloc);
          enbmobility.Install(enbNodes);
    // ------------------------------------------


    // ------------------------------------------
    // Setup UE
    // ------------------------------------------	
       // Node container for UEs
          NodeContainer ueNodes, allNodes, endNodes;
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
    // ------------------------------------------



   
    // ------------------------------------------
    // Setup Configuration
    // ------------------------------------------	

       // NR simulation helpers
          Ptr<NrPointToPointEpcHelper> epcHelper = CreateObject<NrPointToPointEpcHelper>();
          Ptr<IdealBeamformingHelper> idealBeamformingHelper = CreateObject<IdealBeamformingHelper>();
          Ptr<NrHelper> nrHelper = CreateObject<NrHelper>();
          nrHelper->SetBeamformingHelper(idealBeamformingHelper); nrHelper->SetEpcHelper(epcHelper);

       // Spectrum configuration
          BandwidthPartInfoPtrVector allBwps; CcBwpCreator ccBwpCreator; const uint8_t numCcPerBand = 1;
          enum BandwidthPartInfo::Scenario scenarioEnum = BandwidthPartInfo::InH_OfficeOpen_LoS; //previous UMa;
          CcBwpCreator::SimpleOperationBandConf bandConf(frequency, bandwidth, numCcPerBand, scenarioEnum);
          OperationBandInfo band = ccBwpCreator.CreateOperationBandContiguousCc(bandConf);
          nrHelper->InitializeOperationBand(&band);
          allBwps = CcBwpCreator::GetAllBwps({band});

       // Configure ideal beamforming
          idealBeamformingHelper->SetAttribute("BeamformingMethod", TypeIdValue(DirectPathBeamforming::GetTypeId()));

       // Configure scheduler
          nrHelper->SetSchedulerTypeId(NrMacSchedulerTdmaRR::GetTypeId());

       // Antenna configuration for UE
          nrHelper->SetUeAntennaAttribute("NumRows", UintegerValue(2));
          nrHelper->SetUeAntennaAttribute("NumColumns", UintegerValue(4));
          nrHelper->SetUeAntennaAttribute("AntennaElement",
          		PointerValue(CreateObject<IsotropicAntennaModel>()));

       // Antennas for the gNbs
          nrHelper->SetGnbAntennaAttribute("NumRows", UintegerValue(8));
          nrHelper->SetGnbAntennaAttribute("NumColumns", UintegerValue(8));
          nrHelper->SetGnbAntennaAttribute("AntennaElement", PointerValue(CreateObject<IsotropicAntennaModel>()));

       NetDeviceContainer enbNetDev = nrHelper->InstallGnbDevice(enbNodes,allBwps);
       NetDeviceContainer ueNetDev = nrHelper->InstallUeDevice(ueNodes,allBwps);

       // Assign random stream
          int64_t randomStream = 1;
          randomStream += nrHelper->AssignStreams(enbNetDev, randomStream);
          randomStream += nrHelper->AssignStreams(ueNetDev, randomStream);
          // nrHelper->GetGnbPhy(enbNetDev.Get(0), 0)->SetTxPower(txPower);
          for(uint32_t u = 0; u < enbNodes.GetN(); ++u) {
               nrHelper->GetGnbPhy(enbNetDev.Get(u), 0)->SetTxPower(txPower);
               nrHelper->GetGnbPhy(enbNetDev.Get(u), 0)->SetAttribute ("NoiseFigure", DoubleValue (7));
          }     
          for(uint32_t u = 0; u < ueNetDev.GetN(); ++u) {          
             nrHelper->GetUePhy (ueNetDev.Get (u), 0)->SetAttribute ("NoiseFigure", DoubleValue (10));                            
          }

       // When all the configuration is done, explicitly call UpdateConfig ()
          for (auto it = enbNetDev.Begin(); it != enbNetDev.End(); ++it)
		      { DynamicCast<NrGnbNetDevice>(*it)->UpdateConfig();}
          for (auto it = ueNetDev.Begin(); it != ueNetDev.End(); ++it)
		      { DynamicCast<NrUeNetDevice>(*it)->UpdateConfig();}
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
    // -----------------------------------------------------------------------------


    // ---------------------------
    // Setup Application Layer
    // ------------------------------------------	
       uint16_t port = 80;
       ApplicationContainer clientApps; ApplicationContainer serverApps;

       //  targetDt : Target time difference between receiving and playing a frame. [s].
       //  window   : Window for measuring the average throughput. [s].
       //  bufferSpace  : Space in bytes that is used for buffering the video

  // Configuration parameters for UL and DL Traffic parameters for the Flows 

       if (scenario == 1) { // downlink scenario
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
                     clientApps.Add(client.Install(ueNodes.Get(u)));
               }
           // Server side 
              DashServerHelper server ("ns3::TcpSocketFactory", InetSocketAddress(Ipv4Address::GetAny(), port));
              serverApps = server.Install(remoteHost);
              nrHelper->AttachToClosestEnb(ueNetDev, enbNetDev);
       }      
       else {
           // uplink scenario
           // Client Side 

              Simulator::Schedule (Seconds (0.1), &set_RTTOutdir, output_dir);

              for(uint32_t u = 0; u < ueNodes.GetN(); ++u) {
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
        }

       // Setup Apps start time and stop time
          serverApps.Start(Seconds(serverStart)); // 00
          clientApps.Start(Seconds(clientStart)); // 0.25
          serverApps.Stop(Seconds(serverStop)); // simulationTime
          clientApps.Stop(Seconds(clientStop)); // serverStop - 0.25

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

             // p2ph.EnablePcapAll("dash_fact_p2ph", true);
                // internet.EnablePcapIpv4 ("remoteHost.pcap", "remoteHost", 1, true);             
                // internet.EnablePcapIpv4 ("ue1.pcap", "ue1", 1, true);             
             // Config::Connect("/NodeList/*/$ns3::MobilityModel/CourseChange", MakeCallback(&CourseChange));

             // TCP trace 
                // Config::Connect("/NodeList/*/$ns3::TcpL4Protocol/SocketList/*", MakeCallback(&CourseChange));
                std::string rtt_tr_file_name = "rtt_traceFile";  m_firstRtt = true;              
                if (rtt_tr_file_name.compare ("") != 0)
                  {
                       // Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), &DashFact::TraceRTX, this);
                       // Simulator::Schedule (Seconds(serverStart + 1), &TraceRto);
                          // Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), &DashFact::TraceBytesInFlight, this);                       
                          // Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), &DashFact::TraceNextTxSequence, this);                       
                              Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), &DashFact::TraceTCP, this);                       
                          /// Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), &DashFact::TraceTCPInt, this);                       
                          /// Simulator::Schedule (Seconds(clientStart) + NanoSeconds (1), 
                          ///                   &Config::Connect ("/NodeList/27/$ns3::TcpL4Protocol/SocketList/*/RTO", 
                          ///                   &DashFact::TraceRTX, this);
                  }                


             // Trace for SINR (another format)
                for (uint32_t i = 0; i < ueNetDev.GetN(); i++) {
                     // auto uePhy = ueNetDevice->GetPhy();
                     // Ptr<NrSpectrumPhy> ue1SpectrumPhy = DynamicCast<NrUeNetDevice>(ueNetDev.Get(i))->GetPhy(0)->GetSpectrumPhy();                     
                     auto uePhy = DynamicCast<NrUeNetDevice>(ueNetDev.Get(i))->GetPhy(0);
                     Ptr<NrSpectrumPhy> ue1SpectrumPhy = uePhy->GetSpectrumPhy();
                     /// ue1SpectrumPhy->TraceConnectWithoutContext("RxPacketTraceUe", MakeBoundCallback(&UeReceptionTrace, this));
                     ue1SpectrumPhy->TraceConnectWithoutContext("RxPacketTraceUe", MakeBoundCallback(&UeReceptionTrace, this));
                     // uePhy->TraceConnectWithoutContext("ReportPowerSpectralDensity", MakeBoundCallback(&ReportPowerLena, powerStats));                     
                }
                
             // -------------------------------------------------------------
             // Log ueNodes and enbNodes positions and respective distances
             // -------------------------------------------------------------	                               
                for (uint32_t j = 0; j < ueNodes.GetN (); j++) {
                    Names::Add ("ue"+std::to_string(j), ueNodes.Get (j));
                    uint32_t nodeId = ueNodes.Get (j)-> GetId ();
                    uint64_t imsi = ueNodes.Get (j)-> GetDevice (0)->GetObject<NrUeNetDevice> ()->GetImsi ();           
                    Vector v = ueNodes.Get (j)->GetObject<MobilityModel> ()->GetPosition ();
                    m_outUePositionsFile << Names::FindName(ueNodes.Get (j)) << "\t" 
                                         << nodeId << "\t" 
                                         << imsi << "\t" 
                                         << v.x << "\t" << v.y << "\t" << v.z << std::endl;
                  }
              
                for (uint32_t j = 0; j < enbNodes.GetN (); j++) {
                    Names::Add ("gnb"+std::to_string(j), enbNodes.Get (j));           
                    Vector v = enbNodes.Get (j)->GetObject<MobilityModel> ()->GetPosition ();
                    m_outEnbPositionsFile << Names::FindName(enbNodes.Get (j)) << "\t" 
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
            for (int k = 0; k < users; k++) {
                Ptr<DashClient> app = DynamicCast<DashClient> (clientApps.Get(k));              
                m_outDashStatsFile << k << "," << app->GetStatsCsv ();                            
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
                   std::to_string (duration), endNodes, m_simTag, m_outputDir);

       Simulator::Destroy();
}

