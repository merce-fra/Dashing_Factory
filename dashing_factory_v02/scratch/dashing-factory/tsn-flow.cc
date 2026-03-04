

#include "tsn-flow.h"  


NS_LOG_COMPONENT_DEFINE ("TsnFlow");
// --------------------------------------------
//  Tsn Class for our customizable flow generator
//   in concordence with our specefic scenario
// --------------------------------------------
     
 
     TsnFlow::TsnFlow ()
       : m_socket (0),
         m_peer (),
         m_packetSize (0),
         m_nPackets (0),
         m_dataRate (0),
         m_sendEvent (),
         m_running (false),
         m_packetsSent (0)
     {
     }
     
     TsnFlow::~TsnFlow ()
     {
       m_socket = 0;
     }
     
     /* static */
     TypeId TsnFlow::GetTypeId (void)
     {
       static TypeId tid = TypeId ("TsnFlow")
         .SetParent<Application> ()
         .SetGroupName ("Tutorial")
         .AddConstructor<TsnFlow> ()
         ;
       return tid;
     }
     
     void
     TsnFlow::Setup (Ptr<Socket> socket, Address address, uint32_t nPackets, uint32_t packetSize, DataRate dataRate,
                     Time myperiod)
     {
       m_socket = socket;
       m_peer = address;
       m_packetSize = packetSize;
       m_nPackets = nPackets;
       m_dataRate = dataRate;
       m_periodicity = myperiod;
       //m_packetSize = GenerateRandom(minLength, maxLength) ;  
     }
     
     void
     TsnFlow::StartApplication (void)
     {
       m_running = true;
       m_packetsSent = 0;
       m_socket->Bind ();
       m_socket->Connect (m_peer);
       SendPacket ();
     }
     
     void
     TsnFlow::StopApplication (void)
     {
       m_running = false;
     
       if (m_sendEvent.IsRunning ())
         {
           Simulator::Cancel (m_sendEvent);
         }
     
       if (m_socket)
         {
           m_socket->Close ();
         }
     }
     
     void
     TsnFlow::SendPacket (void)
     {
       Ptr<Packet> packet = Create<Packet> (m_packetSize);
       NS_LOG_INFO  ("               >>  Application level : @" << Simulator::Now() << 
                                       " Send packet of class " << m_class_label << 
                                       " (id " << m_class_id << "," << 
                                       " size " << packet->GetSize() <<")");
       m_socket->Send (packet);     
       ScheduleTx ();
       // if (++m_packetsSent < m_nPackets) {ScheduleTx ();}       
     }
     
     void
     TsnFlow::ScheduleTx (void)
     {
       if (m_running)
         {
           /// Time tNext (Seconds (m_packetSize * 8 / static_cast<double> (m_dataRate.GetBitRate ())));
           /// m_sendEvent = Simulator::Schedule (tNext, &TsnFlow::SendPacket, this);
           m_sendEvent = Simulator::Schedule (m_periodicity, &TsnFlow::SendPacket, this);           
         }
     }
     
     uint32_t
     TsnFlow::GenerateRandom (uint32_t myMin, uint32_t myMax)
     {
       // We needed to generate a random number (result) to be used to peek
       // a frame length between a <myMin>-<myMax> values
     
       Ptr<UniformRandomVariable> x = CreateObject<UniformRandomVariable> ();
       x->SetAttribute ("Min", DoubleValue(myMin));
       x->SetAttribute ("Max", DoubleValue(myMax)); 
       uint32_t result = x->GetInteger ();
       return result;
     } 


     void
     TsnFlow::GenerateFlow (uint16_t sinkPort, Ipv4Address destAddress, Ptr<Node> DstNode, Time myStart, Time myStop,\
                            Ptr<Node> SrcNode, uint32_t nPackets, uint32_t packetSize, DataRate dataRate,\
                            Time myperiod)
     { 
          /// uint16_t sinkPort = std::stoi(myPath);
          Address sinkAddress (InetSocketAddress (destAddress, sinkPort));          

          Ptr<Ipv4> node_ipv4 = DstNode->GetObject<Ipv4>();   
          Ipv4StaticRoutingHelper ipv4RoutingHelper;                   
          Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting(node_ipv4);          

          PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));          
          ApplicationContainer sinkApps = packetSinkHelper.Install (DstNode);                  
          sinkApps.Start (myStart);
          sinkApps.Stop (myStop);   

          Ptr<Socket> ns3UdpSocket = Socket::CreateSocket (SrcNode, UdpSocketFactory::GetTypeId ());                    
          Ptr<TsnFlow> myFlow = CreateObject<TsnFlow> ();          
          myFlow->Setup (ns3UdpSocket, sinkAddress, nPackets, packetSize, dataRate, myperiod);                    
          SrcNode->AddApplication (myFlow);          
          myFlow->SetStartTime (myStart);          
          myFlow->SetStopTime (myStop); 

          // NS_LOG_UNCOND ("sinkAddress done ...");
          // NS_LOG_UNCOND ("DstNode is " << Names::FindName(DstNode) << ", dstAdress = " << destAddress);
          // NS_LOG_UNCOND ("packetSinkHelper done ...");          
          // NS_LOG_UNCOND ("sinkApps done ...");                    
          // NS_LOG_UNCOND ("ns3UdpSocket create socket done ...");                              
          // NS_LOG_UNCOND ("ns3UdpSocket myFlow create object done ...");                                        
          // NS_LOG_UNCOND ("myFlow->Setup done ...");                                        
          // NS_LOG_UNCOND ("SrcNode->AddApplication done ...");                                        
          // NS_LOG_UNCOND ("myFlow->SetStartTime done ...");                                        
          // NS_LOG_UNCOND ("Generate traffic flow during [(" << myStart << ")-(" << myStop << ")]");
          // NS_LOG_UNCOND ("                 sinkAddress : [(" << sinkAddress << ")");
          // NS_LOG_UNCOND ("Generate traffic flow during [(" << myStart << ")-(" << myStop << ")]");

     }  
// ----------------------------------------------------------------     



