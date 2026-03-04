
#include "dash-fact.h"
#include "dash-fact.utils.h"


// 
//  Copyright (c) 2023 MERCE
// 
//  This example describes how to setup a simulation using 
// 
//  With the default configuration, the example will create ... .
// 
//  The example will print on-screen.
// 
//   $ ./waf --run "cttc-nr-demo --Help"
// 
// 


using namespace ns3;
NS_LOG_COMPONENT_DEFINE ("dashing-factory");


int
main(int argc, char* argv[])
  {

    // LogComponentEnable ("DashServer", LOG_LEVEL_ALL);
    // LogComponentEnable ("DashClient", LOG_LEVEL_ALL);
    // LogComponentEnable ("TcpSocket", LOG_LEVEL_ALL);
    // LogComponentEnable ("Ipv4RawSocketImpl", LOG_LEVEL_ALL);
    // LogComponentEnable ("MpegPlayer", LOG_LEVEL_ALL);
    // LogComponentEnable ("HttpParser", LOG_LEVEL_ALL);
    // LogComponentEnable ("MPEGHeader", LOG_LEVEL_ALL);
    // LogComponentEnable ("HTTPHeader", LOG_LEVEL_ALL);

       std::string simTag = "36nodes";
       std::string output_dir = "./outputs/";
       uint8_t scenario = 2;

    // Log file for SINR, SNR, RSSI, UEs,
       bool logging = false;
       uint8_t logging_level = 3;
       uint32_t mySeed = 99;

    // Gnb positions and respective distances ...
       std::ofstream m_outSinrFile;         //!< the output file stream for the SINR file
       std::ofstream m_outSnrFile;          //!< the output file stream for the SNR file
       std::ofstream m_outRssiFile;         //!< the output file stream for the RSSI file
       std::ofstream m_outUePositionsFile;  //!< the output file stream for the UE positions file
       std::ofstream m_outAgvPositionsFile;  //!< the output file stream for the UE positions file       
       std::ofstream m_outEnbPositionsFile; //!< the output file stream for the gNB positions file
       std::ofstream m_outDistancesFile;    //!< the output file stream for the distances file
       std::ofstream m_outDashStatsFile;    //!< the output file stream for Dash stats

 // Parameters based on ramous et al. calibration study
 // Parameters according to 3GPP TR 38.901 V16.1.0 (2019-12) Table 7.8-7 
    std::string inFScenario = "InF-SparseLow"; //"InF-SparseHigh", "InF-DenseHigh", "InF-DenseLow", "InF-SparseLow"
    bool cellScan = false; double beamSearchAngleStep = 10.0;  
    double centralFrequencyBand = 3.5e9; // 3.5 or 28 GHz
    double bandwidthBand = 100e6;
    uint16_t numerology = 2;
    double totalTxPower = 30; // The gNB power

    // my old parameters 
       // double frequency = 30e9; //3.75e9;// 700e6;  // central frequency for BS     
       // double bandwidth = 40e6; //3e6; // 50e6;  bandwidth for BS
       // double txPower = 23; //20; // 40;  txPower

    double simulation_time = 10; //10;
    double serverStart = 0; //10;
    double clientStart = 0.25; //10;
    double hBS = 10; // base station antenna height in meters
    double hUT = 1.5; // user antenna height in meters       


    // -----------------
    // Video Parameters
    // -----------------
	   int users = 12; //12;24;36;48 // number of users connected to base station
       // target dt
          double target_dt = 35.0;
       // buffer space size for DASH
          uint32_t bufferSpace = 30000000;
       // window size for DASH
          std::string window = "10s";
    // -------------------------------  



  // ----------------------------------------------------------------------
  // From here, we instruct the ns3::CommandLine class of all the input parameters
  // that we may accept as input, as well as their description, and the storage
  // variable.
  //
    CommandLine cmd;
    cmd.AddValue ("logging", "Enable logging", logging);
    cmd.AddValue ("logging_level", "Logging level", logging_level);
    cmd.AddValue ("simulationTime", "Duration of simulation", simulation_time);

    cmd.AddValue ("cellScan",
                  "Use beam search method to determine beamforming vector,"
                  "true to use cell scanning method",
                  cellScan);
    cmd.AddValue ("beamSearchAngleStep",
                  "Beam search angle step for beam search method",
                  beamSearchAngleStep);

    cmd.AddValue ("serverStartTime", "Start time of server", serverStart);
    cmd.AddValue ("clientStartTime", "Start time of client", clientStart);
    cmd.AddValue ("users", "Number of UEs", users);
    cmd.AddValue ("outputDir", "directory where to store simulation results", output_dir);
    cmd.AddValue ("scenario",
                  "the scenario to use. Choose between downlink [1], or uplink [2]",
                  scenario); 
    cmd.AddValue ("inFScenario",
                  "InFactory scenario. Choose between [InF-SparseLow], [InF-SparseHigh], [InF-DenseHigh], [InF-DenseLow] or [InF-SparseLow]",
                  inFScenario);                   
    cmd.AddValue ("seed", "Random seed to use", mySeed);                                   
    cmd.AddValue ("simTag", "tag to be appended to output filenames to distinguish simulation campaigns",
                   simTag);    
    cmd.Parse (argc, argv);   // Parse the command line
  // ---------------------------------------------------

 // -----------------------------------------------
 // Check if the frequency is in the allowed range.
 // If you need to add other checks, here is the best position to put them.
 // -----------------------------------------------------------------------
   SeedManager::SetSeed (mySeed);
   RngSeedManager::SetRun (mySeed);    
 // --------------------------------------------


 // --------------------------------------------------------------------------
 // If the logging variable is set to true, enable the log of some components
 // through the code. The same effect can be obtained through the use
 // of the NS_LOG environment variable:
 // export NS_LOG="UdpClient=level_info|prefix_time|prefix_func|prefix_node:UdpServer=..."
 // Usually, the environment variable way is preferred, as it is more customizable,
 // and more expressive.
 // ----------------------
    if (logging)
      {
        // LogComponentEnable ("UdpClient", LOG_LEVEL_INFO);
        // LogComponentEnable ("UdpServer", LOG_LEVEL_INFO);
        // LogComponentEnable ("LtePdcp", LOG_LEVEL_INFO);          
        // LogComponentEnable ("EvalsvcClient", LOG_LEVEL_INFO);
        // LogComponentEnable ("EvalsvcServer", LOG_LEVEL_INFO);                  
        LogComponentEnable ("TcpSocketBase", LOG_LEVEL_ALL);                                
      }  
      
 // ------

    DashFact dashFact_scenario;

    
    dashFact_scenario.Run ( hBS, hUT, users, numerology, centralFrequencyBand, 
                            bandwidthBand, cellScan, beamSearchAngleStep, totalTxPower, 
                            target_dt, bufferSpace, window, 
                            simulation_time, serverStart, clientStart, 
                            simulation_time, // serverStop, 
                            simulation_time - 0.25, // clientStop,
                            simTag, output_dir, logging_level,
                            scenario, inFScenario, mySeed);
                                                    
    return 0;  
 
}

