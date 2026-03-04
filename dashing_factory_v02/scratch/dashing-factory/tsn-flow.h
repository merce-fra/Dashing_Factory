

#include <iostream> 
#include <fstream>
#include <string>
#include <cassert>

#include "ns3/core-module.h" 
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-helper.h"
//#include "ns3/constantpositionmobilitymodel"

//Flow monitor stuffs
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/flow-monitor-module.h"

// TSN stuffs
#include "ns3/traffic-control-module.h"


#include "ns3/application.h"

using namespace ns3;

// --------------------------------------------
//  Tsn Class for our customizable flow generator
//   in concordence with our specefic scenario
// --------------------------------------------
     
class TsnFlow : public Application
     {
     public:
       TsnFlow ();
       virtual ~TsnFlow ();
     
       /**
        * Register this type.
        * \return The TypeId.
        */
       static TypeId GetTypeId (void);
       //void Setup (Ptr<Socket> socket, Address address, uint32_t packetSize, uint32_t nPackets, DataRate dataRate);
       void Setup (Ptr<Socket> socket, Address address, uint32_t nPackets, uint32_t packetSize, DataRate dataRate, 
                   Time myperiod);    
       uint32_t GenerateRandom (uint32_t myMin, uint32_t myMax);
       void GenerateFlow (uint16_t port , Ipv4Address myInterface, Ptr<Node> DstNode, Time myStart, Time myStop,
                          Ptr<Node> SrcNode, uint32_t nPackets, uint32_t packetSize, DataRate dataRate,
                          Time myperiod);             
     private:
       virtual void StartApplication (void);
       virtual void StopApplication (void);
     
       void ScheduleTx (void);
       void SendPacket (void);

       Ptr<Socket>     m_socket;
       Address         m_peer;
       uint32_t        m_packetSize;
       uint32_t        m_nPackets;
       DataRate        m_dataRate;
       EventId         m_sendEvent;
       bool            m_running;
       uint32_t        m_packetsSent; 


      // Adopt the same ISA class-labeling scheme  to categorize applications, 
      // i.e., groups applications according to mission-criticality, 
      // e.g., the more critical “Class 0: Emergency,” versus the less critical “Class 5: Logging, Downloading, and Uploading.”
       uint8_t         m_class_id; //0 -> 5
       std::string     m_class_label; 
       std::string     m_description; 

      // E2E delay or deadline 
       uint8_t         m_deadline;//ms
       uint8_t         m_deadline_typical;//ms
       double          m_deadline_strict;

      // The size, in bytes, of the information portion of a single transmission
       uint32_t        m_payloadSize;
       uint8_t         m_payloadSize_min;       
       uint32_t        m_payloadSize_max;

      // Transmission period or application’s control cycle, 
      // related to (update rate in Hz, i.e., the number of transmissions that occur 
      // in one second at the app layer of a device. 
       Time            m_periodicity;
       uint8_t         m_periodicity_typical; 
       uint32_t        m_periodicity_max;              
     };
     
      //  Domain       |i.d| Usage Class         | Description        | e2e(ms) / payload (B) / period(ms   
      // --------------+---+---------------------+--------------------+-----------------------------  
      //  Safety       | 0 | Emergency action    | Always critical    | 4-0.5/6-24/8-1  |
      // --------------+---+---------------------+--------------------+-------------  
      //  Control      | 1 | Closed loop         | Often critical     | 4-0.25/8-64/8-0.5  |
      //               |   | regulatory control  |                    |                 |
      //               +   +---------------------+--------------------+-------------  
      //               | 2 | Closed loop         | Usually            | 20-4/8-64/40-8  |
      //               |   | supervisory control | non-critical       |                 |
      //               +   +---------------------+--------------------+-------------  
      //               | 3 | Open loop control   | Human in the       | 4-0.5/8-64/8-1  |
      //               |   |                     | loop               |                 |
      // --------------+---+---------------------+--------------------+-------------  
      //  Monitoring   | 4 | Alerting            | Short-term         | 50-4/12-33KB/100-8  |
      //               |   |                     | operational        |  |
      //               |   |                     | consequence (e.g.  |  |
      //               |   |                     | event-based        |  |
      //               |   |                     | maintenance)       |  |
      // --------------+---+---------------------+--------------------+-------------  
      //               | 5 | Logging,            | No immediate       | Clause 5.6  |
      //               |   | Downloading, and    | operational        | Clause 5.7  |
      //               |   | Uploading           | consequence        | Clause 5.8  |
      //               |   |                     | (e.g., history     | Clause 5.9  |
      //               |   |                     | collection,        |  |
      //               |   |                     | sequence of events,|  |
      //               |   |                     | preventive         |  |
      //               |   |                     | maintenance)       |  |
      // --------------+---+----------------------------+--------------------+-------------  

