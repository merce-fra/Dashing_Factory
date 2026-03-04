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

using namespace ns3;


/**
 * \ingroup examples
 * \file cttc-3gpp-indoor-calibration.cc
 * \brief Simulation script for the NR-MIMO Phase 1 system-level calibration
 *
 * The scenario implemented in the present simulation script is according to
 * the topology described in 3GPP TR 38.900 V15.0.0 (2018-06) Figure 7.2-1:
 * "Layout of indoor office scenarios".
 *
 * The simulation assumptions and the configuration parameters follow
 * the evaluation assumptions agreed at 3GPP TSG RAN WG1 meeting #88,
 * and which are summarised in R1-1703534 Table 1.
 * In the following Figure is illustrated the scenario with the gNB positions
 * which are represented with "x". The UE nodes are randomly uniformly dropped
 * in the area. There are 10 UEs per gNB.
 *
 * <pre>
 *   +----------------------120 m------------------ +
 *   |                                              |
 *   |                                              |
 *   |      x      x      x      x      x-20m-x     |
 *   |                                        |     |
 *   50m                                     20m    |
     |                                        |     |
 *   |      x      x      x      x      x     x     |
 *   |                                              |
 *   |                                              |
 *   +----------------------------------------------+
 * </pre>
 * The results of the simulation are files containing data that is being
 * collected over the course of the simulation execution:
 *
 * - SINR values for all the 120 UEs
 * - SNR values for all the 120 UEs
 * - RSSI values for all the 120 UEs
 *
 * Additionally there are files that contain:
 *
 * - UE positions
 * - gNB positions
 * - distances of UEs from the gNBs to which they are attached
 *
 * The file names are created by default in the root project directory if not
 * configured differently by setting resultsDirPath parameter of the Run()
 * function.
 *
 * The file names by default start with the prefixes such as "sinrs", "snrs",
 * "rssi", "gnb-positions,", "ue-positions" which are followed by the
 * string that briefly describes the configuration parameters that are being
 * set in the specific simulation execution.
 */



typedef std::map<std::string, std::string> dictionnary;

/**
 * \brief Main class
 */
class DashFact
{

public:

  /// /**
  ///  * Initializer ...
  ///  *
  ///  * @param centralFrequencyBand The band central frequency ... etc
  ///  */
  DashFact();

  
  /**
   * \brief Destructor that closes the output file stream and finished the
   * writing into the files.
   */
  // ~Nr3gppIndoorCalibration ();
  ~DashFact ();

  /// static 
  /// void TraceRtt (std::string rtt_tr_file_name);
  /// static 
  /// void RttTracer (Time oldval, Time newval);


/** ------------------------
 * Function used to log sinr
 **/
    
  /**
   * Function that will actually configure all the simulation parameters,
   * topology and run the simulation by using the parameters that are being
   * configured for the specific run.
   *
   * @param centralFrequencyBand The band central frequency ... etc
   */
     void Run ( double hBS, double hUT, int users, double frequency, double bandwidth, 
                          double txPower, double target_dt, uint32_t bufferSpace, std::string window, 
                          double simulationTime, double serverStart, double clientStart, 
                          double serverStop, double clientStop,
                          std::string simTag, std::string output_dir, uint8_t logging_level,
                          uint8_t scenario );
  



  /**
   * \brief Function selects UE nodes that are placed with a minimum
   * distance from its closest gNB.
   * \param ueNodes - container of UE nodes
   * \param gnbNodes - container of gNB nodes
   * \param min3DDistance - the minimum that shall be between UE and gNB
   * \param numberOfUesToBeSelected - the number of UE nodes to be selected
   * from the original container
   */
  NodeContainer SelectWellPlacedUes (const NodeContainer ueNodes, const NodeContainer gnbNodes,
                                     double min3DDistance, uint32_t numberOfUesToBeSelected);

  Ptr<ListPositionAllocator>
  ue_positions_static (int users, int hUT, bool randomness);
  
  
  ns3::NodeContainer
  ue_positions_random (int ueCount, int ueHeight, const NodeContainer gnbNodes,
                       const NodeContainer ueNodes, double minDistance);


  /// std::string
  /// BuildFileNameString (std::string directoryName, std::string filePrefix, std::string tag);
  
  void
  BuildFileNames (std::string tag);

  void 
  closeOutputFiles ();

  void
  LogPositions (NodeContainer selectedUeNodes, NodeContainer gNbNodes);



  void RtoTracer (std::string context, Time oldval, Time newval);
  void TraceRto ();
  void Initialize_RtxTraceFile ();
  
  void RttTracer (std::string context, Time oldval, Time newval);
  void TraceRtt ();
  void Initialize_RttTraceFile ();  
  void Initialize_TcpTraceFiles (std::string output_directory);


  void TraceTCP (); 
  void TCPIntTracer (std::string context, uint32_t oldval, uint32_t newval);
  void TCPSeqTracer (std::string context, SequenceNumber32 oldval, SequenceNumber32 newval);
  void TCPTimeTracer (std::string context, Time oldval, Time newval);
  void TCPRateTracer (std::string context, DataRate oldval, DataRate newval);
  void TCPCongStateTracer (std::string context, TcpSocketState::TcpCongState_t oldval, TcpSocketState::TcpCongState_t newval);
  void TCPEcnStateTracer (std::string context, TcpSocketState::EcnState_t oldval, TcpSocketState::EcnState_t newval);

  template<typename T> void 
  TCPTracerX (std::string parameter, T oldValue, T newValue);      

  void TraceSinr (NetDeviceContainer ueNetDev);
  void SinrTracer (RxPacketTraceParams params);

  void UeReception (RxPacketTraceParams params);

  uint8_t m_loggingLevel;
  std::string m_simTag;
  std::string m_outputDir;
  bool m_firstRtt;

  /// std::ofstream get_sinrOutputFile ();


private:
  dictionnary m_tcp_traceFiles;
  std::string m_output_directory;
  std::ofstream m_outSinrFile;         //!< the output file stream for the SINR file
  std::ofstream m_outSnrFile;          //!< the output file stream for the SNR file
  std::ofstream m_outRssiFile;         //!< the output file stream for the RSSI file
  std::ofstream m_outUePositionsFile;  //!< the output file stream for the UE positions file
  std::ofstream m_outEnbPositionsFile; //!< the output file stream for the gNB positions file
  std::ofstream m_outDistancesFile;    //!< the output file stream for the distances file
  std::ofstream m_outDashStatsFile;    //!< the output file stream for Dash stats
  std::ofstream m_outRTTStatsFile;    //!< the output file stream for TCP's RTT stats
  std::ofstream m_outRTOStatsFile;    //!< the output file stream for TCP's RTO stats

};