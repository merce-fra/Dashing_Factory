/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 TEI of Western Macedonia, Greece
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Dimitrios J. Vergados <djvergad@gmail.com>
 */

#include <ns3/log.h>
#include <ns3/uinteger.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/simulator.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include "http-header.h"
#include "dash-client.h"

#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"

#include <fstream>

NS_LOG_COMPONENT_DEFINE ("DashClient");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (DashClient);

int DashClient::m_countObjs = 0;

double round_to(double value, double precision = 1.0)
{
    return std::round(value / precision) * precision;
}

TypeId
DashClient::GetTypeId (void)
{
  static TypeId tid =
      TypeId ("ns3::DashClient")
          .SetParent<Application> ()
          .AddConstructor<DashClient> ()
          .AddAttribute ("VideoId", "The Id of the video that is played.", UintegerValue (0),
                         MakeUintegerAccessor (&DashClient::m_videoId),
                         MakeUintegerChecker<uint32_t> (1))
          .AddAttribute ("Remote", "The address of the destination", AddressValue (),
                         MakeAddressAccessor (&DashClient::m_peer), MakeAddressChecker ())
          .AddAttribute ("Protocol", "The type of TCP protocol to use.",
                         TypeIdValue (TcpSocketFactory::GetTypeId ()),
                         MakeTypeIdAccessor (&DashClient::m_tid), MakeTypeIdChecker ())
          /// .AddAttribute ("TargetDt", "The target buffering time", TimeValue (Time ("35s")),
          .AddAttribute ("TargetDt", "The target buffering time", TimeValue (Time ("10s")),
                         MakeTimeAccessor (&DashClient::m_target_dt), MakeTimeChecker ())
          .AddAttribute ("window", "The window for measuring the average throughput (Time)",
                         TimeValue (Time ("10s")), MakeTimeAccessor (&DashClient::m_window),
                         MakeTimeChecker ())
          /// .AddAttribute ("bufferSpace", "The buffer space in bytes", UintegerValue (30000000),
          .AddAttribute ("bufferSpace", "The buffer space in bytes", UintegerValue (1000000),
                         MakeUintegerAccessor (&DashClient::m_bufferSpace),
                         MakeUintegerChecker<uint32_t> ())
          /// .AddAttribute ("outputDir", "output file for stats by client", UintegerValue (30000000),
          ///                MakeUintegerAccessor (&DashClient::m_bufferSpace),
          ///                MakeUintegerChecker<uint32_t> ())                                                
          .AddTraceSource ("Tx", "A new packet is created and is sent",
                           MakeTraceSourceAccessor (&DashClient::m_txTrace),
                           "ns3::Packet::TracedCallback");
  return tid;
}

DashClient::DashClient ()
    : m_bufferSpace (0),
      m_player (this->GetObject<DashClient> (), m_bufferSpace),
      m_rateChanges (0),
      m_target_dt ("35s"),
      m_bitrateEstimate (0.0),
      m_segmentId (0),
      m_socket (0),
      m_connected (false),
      m_totBytes (0),
      m_started (Seconds (0)),
      m_sumDt (Seconds (0)),
      m_lastDt (Seconds (-1)),
      m_id (m_countObjs++),
      m_requestTime ("0s"), 
      m_segment_bytes (0),
      m_bitRate (45000),
      m_window (Seconds (10)),
      m_segmentFetchTime (Seconds (0)),
      m_totalSegFetchTime (Seconds (0)),
      m_receivedSegments (0)
{
  NS_LOG_FUNCTION (this);
  m_parser.SetApp (this); // So the parser knows where to send the received messages

  
  // std::string filenameDashStatsByNode = "dashStatsByNode_" + std::to_string(m_id) + ".csv";
  // m_outDashStatsByNodeFile.open (filenameDashStatsByNode.c_str ()); m_outDashStatsByNodeFile.setf (std::ios_base::fixed);    
  // m_outDashStatsByNodeFile << "timeStamp,Node,newBitRate,oldBitRate,estBitRate,interTime,T,dT,del" 
  //                         << std::endl;
  InitializeCSVs ();                

}

DashClient::~DashClient ()
{
  NS_LOG_FUNCTION (this);
  m_outDashStatsByNodeFile.close ();
  m_outSegStatsByNodeFile.close ();
}

Ptr<Socket>
DashClient::GetSocket (void) const
{
  NS_LOG_FUNCTION (this);
  return m_socket;
}

void
DashClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);

  m_socket = 0;
  m_keepAliveTimer.Cancel ();
  // chain up
  Application::DoDispose ();
}

// Application Methods
void
DashClient::StartApplication (void) // Called at time specified by Start
{
  NS_LOG_FUNCTION (this);

  Ptr<Ipv4> node_ipv4 = GetNode ()->GetObject<Ipv4>();                      
  Ipv4Address ipv4Address = node_ipv4->GetAddress (1, 0).GetLocal ();                          


  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::StartApplication () ");
  NS_LOG_INFO ("                               GetNode () : " << GetNode ()->GetId ());
  NS_LOG_INFO ("                                  dashClient (" << m_id << ")");   
  NS_LOG_INFO ("                               ipv4 Address : " << ipv4Address);
  NS_LOG_INFO ("                               Phy  : " << GetNode ()->GetDevice (0)->GetAddress ());
  /// NS_INFOCOND ("                                 connected with " << m_socket->GetNode ()->GetId ());
  NS_LOG_INFO ("");           


  // Create the socket if not already

  NS_LOG_INFO ("trying to create connection");
  NS_LOG_INFO ("                               trying to create connection...");  
  NS_LOG_INFO ("                               !socket ??");  
  if (!m_socket)
    {
      NS_LOG_INFO ("m_socket is null");

      m_started = Simulator::Now ();
      NS_LOG_INFO ("                                   !socket => create it m_socket...");
      m_socket = Socket::CreateSocket (GetNode (), m_tid);

      // Fatal error if socket type is not NS3_SOCK_STREAM or NS3_SOCK_SEQPACKET
      if (m_socket->GetSocketType () != Socket::NS3_SOCK_STREAM &&
          m_socket->GetSocketType () != Socket::NS3_SOCK_SEQPACKET)
        {
          NS_FATAL_ERROR ("Using HTTP with an incompatible socket type. "
                          "HTTP requires SOCK_STREAM or SOCK_SEQPACKET. "
                          "In other words, use TCP instead of UDP.");
        }

      if (Inet6SocketAddress::IsMatchingType (m_peer))
        {
          NS_LOG_INFO ("                                   bind6 m_socket ...");
          m_socket->Bind6 ();
        }
      else if (InetSocketAddress::IsMatchingType (m_peer))
        {
          NS_LOG_INFO ("                                   bind m_socket ...");          
          m_socket->Bind ();
        }

      NS_LOG_INFO ("                                   set callbacks ...");          
      m_socket->Connect (m_peer);
      m_socket->SetRecvCallback (MakeCallback (&DashClient::HandleRead, this));
      m_socket->SetConnectCallback (MakeCallback (&DashClient::ConnectionSucceeded, this),
                                    MakeCallback (&DashClient::ConnectionFailed, this));
      m_socket->SetSendCallback (MakeCallback (&DashClient::DataSend, this));
      m_socket->SetCloseCallbacks (MakeCallback (&DashClient::ConnectionNormalClosed, this),
                                   MakeCallback (&DashClient::ConnectionErrorClosed, this));

      NS_LOG_INFO ("Connected callbacks");
    }
  NS_LOG_INFO ("Just started connection");
  NS_LOG_INFO ("                               socket created, Just started connection");
  NS_LOG_INFO ("                               m_peer = " << m_peer);
  NS_LOG_INFO ("    [" << Simulator::Now () << "] DashClient::StartApplication () DONE ");
  NS_LOG_INFO ("    ------------------------------------------------------");      
  NS_LOG_INFO ("");             
}

void
DashClient::StopApplication (void) // Called at time specified by Stop
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::StopApplication () ");
  NS_LOG_INFO ("                                  dashClient (" << m_id << ")");   
  NS_LOG_INFO ("                                  m_state (" << m_player.m_state << ")");   
  NS_LOG_INFO ("");           

  if (m_socket != 0)
    {
      m_socket->Close ();
      m_connected = false;

      NS_LOG_INFO ("                                  m_state is now ==> MPEG_PLAYER_DONE");
      m_player.m_state = MPEG_PLAYER_DONE;
    }
  else
    {
      NS_LOG_WARN ("DashClient found null socket to close in StopApplication");
    }
  NS_LOG_INFO ("    [" << Simulator::Now () << "] DashClient::StopApplication () DONE ");
  NS_LOG_INFO ("    ------------------------------------------------------");      
  NS_LOG_INFO ("");                 
}

// Private helpers

void
DashClient::RequestSegment ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::RequestSegment () ");
  NS_LOG_INFO ("                                  dashClient (" << m_id << ")");   
  NS_LOG_INFO ("");           


  if (m_RequestPending)
    {
      NS_LOG_INFO ("Not requesting, found pending m_segmentId = " << m_segmentId);
      return;
    }
  m_RequestPending = true;

  if (m_connected == false)
    {
      return;
    }
  NS_LOG_INFO ("                                  (inside RequestSegment) create packey/http header");  
  NS_LOG_INFO ("                                                          MessageType = " << HTTP_REQUEST);  
  NS_LOG_INFO ("                                                          VideoId = " << m_videoId);  
  NS_LOG_INFO ("                                                          Resolution = " << m_bitRate);  
  NS_LOG_INFO ("                                                          SegmentId = " << m_segmentId);  
  Ptr<Packet> packet = Create<Packet> (0);

  HTTPHeader httpHeader;
  httpHeader.SetSeq (1);
  httpHeader.SetMessageType (HTTP_REQUEST);
  httpHeader.SetVideoId (m_videoId);
  httpHeader.SetResolution (m_bitRate);
  httpHeader.SetSegmentId (m_segmentId++);
  NS_LOG_INFO ("                                                          packet->AddHeader (httpHeader)");  
  NS_LOG_INFO ("                                                          // header to be added to newly created packet ...");  
  packet->AddHeader (httpHeader);

  int res = 0;
  NS_LOG_INFO ("                                                          m_socket->Send (packet))");    
  NS_LOG_INFO ("                                                          // send packet from inside m_socket ...");    
  if (((unsigned) (res = m_socket->Send (packet))) != packet->GetSize ())
    {
      NS_FATAL_ERROR ("Oh oh. Couldn't send packet! res=" << res << " size=" << packet->GetSize ());
    }

  m_requestTime = Simulator::Now ();
  m_segment_bytes = 0;

  NS_LOG_INFO ("    [" << Simulator::Now () << "] DashClient::RequestSegment () DONE ");
  NS_LOG_INFO ("    ------------------------------------------------------");      
  NS_LOG_INFO ("");             
}

void
DashClient::SendBlank ()
{
  HTTPHeader http_header;
  http_header.SetMessageType (HTTP_BLANK);
  http_header.SetVideoId (-1);
  http_header.SetResolution (-1);
  http_header.SetSegmentId (-1);

  Ptr<Packet> blank_packet = Create<Packet> (0);
  blank_packet->AddHeader (http_header);

  m_socket->Send (blank_packet);
}

void
DashClient::CheckBuffer ()
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::CheckBuffer () ");
  NS_LOG_INFO ("                                  dashClient (" << m_id << ")");   
  NS_LOG_INFO ("");             
  NS_LOG_INFO ("                                  m_parser.ReadSocket (m_socket)");   
  m_parser.ReadSocket (m_socket);
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::CheckBuffer () DONE ");
  NS_LOG_INFO ("    ------------------------------------------------------");      
  NS_LOG_INFO ("");               
}

void
DashClient::KeepAliveTimeout ()
{
  if (m_socket != NULL)
    {
      SendBlank ();
    }
  else
    {
      m_keepAliveTimer.Cancel ();
      Time delay = MilliSeconds (300);
      m_keepAliveTimer = Simulator::Schedule (delay, &DashClient::KeepAliveTimeout, this);
    }
}

void
DashClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::HandleRead (socket)  ");
  NS_LOG_INFO ("                               from node" << socket->GetNode ()->GetId ());
  NS_LOG_INFO ("                               dashClient (" << m_id << ")");   
  NS_LOG_INFO ("                               pair add : " << m_peer);
  NS_LOG_INFO ("                               // Called when we receive data from the server");
  NS_LOG_INFO ("");             

  m_keepAliveTimer.Cancel ();
  Time delay = MilliSeconds (300);
  m_keepAliveTimer = Simulator::Schedule (delay, &DashClient::KeepAliveTimeout, this);
  NS_LOG_INFO ("                                  (inside HandleRead) m_parser.ReadSocket (socket) to be called");  
  NS_LOG_INFO ("                                                      // socket to be read by m_parser ");    
  NS_LOG_INFO ("                                                      // An HttpParser object for parsing the incoming stream into http messages");    
  m_parser.ReadSocket (socket);
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::HandleRead () DONE ");
  NS_LOG_INFO ("    ------------------------------------------------------");      
  NS_LOG_INFO ("");               
}

void
DashClient::ConnectionSucceeded (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::ConnectionSucceeded (socket)  ");
  NS_LOG_INFO ("                                  dashClient (" << m_id << ")");   
  NS_LOG_INFO ("                               from node" << socket->GetNode ()->GetId ());
  NS_LOG_INFO ("                               pair add : " << m_peer);
  NS_LOG_INFO ("");               
  NS_LOG_LOGIC ("DashClient Connection succeeded");
  NS_LOG_INFO ("                               m_connected is now true");  
  m_connected = true;
  socket->SetCloseCallbacks (MakeCallback (&DashClient::ConnectionNormalClosed, this),
                             MakeCallback (&DashClient::ConnectionErrorClosed, this));

  NS_LOG_INFO ("                                  (inside ConnectionSucceeded) RequestSegment (); to be called");  
  NS_LOG_INFO ("                                   // Called the next MPEG segment should be requested from the server.");    

  RequestSegment ();
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::ConnectionSucceeded () DONE ");
  NS_LOG_INFO ("    ------------------------------------------------------");        
}

void
DashClient::ConnectionFailed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_LOGIC ("DashClient " << m_id << ", Connection Failed, retrying...");
  m_socket = 0;
  m_connected = false;
  StartApplication ();
}

void
DashClient::ConnectionNormalClosed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO ("DashClient " << m_id << ", Connection closed normally");
}

void
DashClient::ConnectionErrorClosed (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  NS_LOG_INFO ("DashClient " << m_id << ", Connection closed due to error, retrying...");
  m_socket = 0;
  m_connected = false;
  StartApplication ();
}

void DashClient::DataSend (Ptr<Socket>, uint32_t)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::DataSend (Ptr<Socket>, uint32_t)  ");
  NS_LOG_INFO ("                               // called when the data has been transmitted");
  NS_LOG_INFO ("                                  dashClient (" << m_id << ")"); 
  NS_LOG_INFO ("");               


  if (m_connected)
    { // Only send new data if the connection has completed
      NS_LOG_INFO ("                               dashClient " << m_id << ", Something was sent");
      NS_LOG_INFO ("DashClient " << m_id << ", Something was sent");
    }
  else
    {
      NS_LOG_INFO ("DashClient " << m_id << ", NOT CONNECTED!!!!");
    }
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::DataSend () DONE ");
  NS_LOG_INFO ("    ------------------------------------------------------");        

}

bool
DashClient::MessageReceived (Packet message)
{
  NS_LOG_FUNCTION (this << message);
  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::MessageReceived (Packet message)  ");
  NS_LOG_INFO ("                               // Called by the HttpParser when it has received (tryToPushPlayer())");
  NS_LOG_INFO ("                               // a complete HTTP message containing an MPEG frame.");
  NS_LOG_INFO ("                                  dashClient (" << m_id << ")"); 
  NS_LOG_INFO ("");               

  MPEGHeader mpegHeader;
  HTTPHeader httpHeader;

  NS_LOG_INFO ("                                  (inside MessageReceived) m_player.ReceiveFrame (&message)");  
  NS_LOG_INFO ("                                                           // send the frame to the player");
  NS_LOG_INFO ("                                                           // if it doesn't fit in the buffer, don't continue");  

  // Send the frame to the player
  // If it doesn't fit in the buffer, don't continue
  if (!m_player.ReceiveFrame (&message))
    {
      return false;
    }
  NS_LOG_INFO ("                       (Reminder) (inside MessageReceived) dashClient (" << m_id << ")");      
  NS_LOG_INFO ("                                                           m_segment_bytes += message.GetSize () ... ");      
  NS_LOG_INFO ("                                                           m_totBytes += message.GetSize () ... ");      
  m_segment_bytes += message.GetSize ();
  m_totBytes += message.GetSize ();

  NS_LOG_INFO ("                                                           message.RemoveHeader (httpHeader) ...");
  NS_LOG_INFO ("                                                           message.RemoveHeader (mpegHeader) ...");
  message.RemoveHeader (httpHeader);
  message.RemoveHeader (mpegHeader);

  NS_LOG_INFO ("                                                           calculate the buffering time ... ");
  NS_LOG_INFO ("                                                           switch m_player.m_state ? : (" << m_player.m_state << ")");
  
  // Calculate the buffering time
  switch (m_player.m_state)
    {
    case MPEG_PLAYER_PLAYING:    
      NS_LOG_INFO ("                                                                     MPEG_PLAYER_PLAYING --> add actual realplaytime to m_sumDt");
      NS_LOG_INFO ("                                                                     m_sumDt += m_player.GetRealPlayTime (mpegHeader.GetPlaybackTime ()) ");
      m_sumDt += m_player.GetRealPlayTime (mpegHeader.GetPlaybackTime ());
      break;
    case MPEG_PLAYER_PAUSED:
      NS_LOG_INFO ("                                                                     = MPEG_PLAYER_PAUSED -> break;");    
      NS_LOG_INFO ("                                                                      //we are inside mesgReceived");
      break;
    case MPEG_INITIAL_BUFFERING:
      NS_LOG_INFO ("                                                                     = MPEG_INITIAL_BUFFEING -> break;.");    
      NS_LOG_INFO ("                                                                      //we are inside mesgReceived, nothing to do");
      break;
    case MPEG_PLAYER_DONE:
      NS_LOG_INFO ("                                                                     = MPEG_PLAYER_DONE -> return true.");    
      NS_LOG_INFO ("                                                                      //we are inside mesgReceived");    
      return true;
    default:
      NS_FATAL_ERROR ("WRONG STATE");
    }
  
  NS_LOG_INFO ("                                  (inside MessageReceived) Received frame " 
                                                                    << mpegHeader.GetFrameId () << " out of "
                                                                    << MPEG_FRAMES_PER_SEGMENT
                                                                    << "  (state = [" << m_player.m_state << "] )");                                                                    

  /// NS_LOG_INFO ("Received frame " << mpegHeader.GetFrameId () << " out of "
  ///                                << MPEG_FRAMES_PER_SEGMENT);

  // If we received the last frame of the segment
  NS_LOG_INFO ("                                  (inside MessageReceived) is this the last frame of the curr segment ?? ");   
  if (mpegHeader.GetFrameId () == MPEG_FRAMES_PER_SEGMENT - 1)
    {
      NS_LOG_INFO ("                                                           Yes we did (final frame in segment)");         
      NS_LOG_INFO ("                                                           m_RequestPending is now FALSE");         
      NS_LOG_INFO ("                                                           m_segmentFetchTime = Simulator::Now () - m_requestTime");         
      NS_LOG_INFO ("                                                                              = " << Simulator::Now () - m_requestTime);         
      m_RequestPending = false;
      m_segmentFetchTime = Simulator::Now () - m_requestTime;
      m_totalSegFetchTime += m_segmentFetchTime;

      /// NS_LOG_INFO (Simulator::Now ().GetSeconds ()
      NS_LOG_INFO ("                                                           Context : ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");         
      NS_LOG_INFO ("                                                                       "<< Simulator::Now ().GetSeconds () 
                                                                                             << " segment size in bytes: " << m_segment_bytes);
      NS_LOG_INFO ("                                                                       segmentTime (duration): " << m_segmentFetchTime.GetSeconds ());
      NS_LOG_INFO ("                                                                       segmentRate: " << 8 * m_segment_bytes / m_segmentFetchTime.GetSeconds ());
      NS_LOG_INFO ("                                                                     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");         
      NS_LOG_INFO ("                                                           // Feed the bitrate info to the player ");         
      NS_LOG_INFO ("                                                              AddBitRate (now, 8 * m_segment_bytes / m_segmentFetchTime); ");         
      NS_LOG_INFO ("                                                                         (now, 8 *" << m_segment_bytes << "/" 
                                                                                                     <<  m_segmentFetchTime << ")");         

      // Feed the bitrate info to the player
      // This is not segment bit rate. it is achieved throughput over the segment.  
      // It can be confused with just averaging the segment resoulution over a window, 
      // but it is not that. It is the actual rate from time to receive segment.  
      // c.f. https://github.com/akhila-s-rao/predictions-for-edge-enabled-dash/blob/master/model/dash-client.cc    
      AddBitRate (Simulator::Now (), 8 * m_segment_bytes / m_segmentFetchTime.GetSeconds ());

      NS_LOG_INFO ("                                                           // Calculate currDt ");         
      NS_LOG_INFO ("                                                              currDt = m_player.GetRealPlayTime (mpegHeader.GetPlaybackTime ())");         
      NS_LOG_INFO ("                                                                     = m_player.GetRealPlayTime (" << mpegHeader.GetPlaybackTime ().GetSeconds () << ")");         
      Time currDt = m_player.GetRealPlayTime (mpegHeader.GetPlaybackTime ());
      NS_LOG_INFO ("                                                                     = " << currDt.GetSeconds () << "s"); 
      NS_LOG_INFO ("                                                                     // my question is, whats to do with this currDt ?"); 
      NS_LOG_INFO ("                                                                        you can say it's the time you wasted doing something ");
      NS_LOG_INFO ("                                                                        else as playing the video ... a kind of delay"); 
      



      NS_LOG_INFO ("                                                           // tell the player to monitor the buffer level ");
      NS_LOG_INFO ("                                                              LogBufferLevel (currDt) ");         
      NS_LOG_INFO ("                                                              LogBufferLevel (" << currDt << ") ");         

      // And tell the player to monitor the buffer level
      LogBufferLevel (currDt);

      NS_LOG_INFO ("                                                           // dont know why all these initializations ?!");
      NS_LOG_INFO ("                                                                    uint32_t old = m_bitRate");
      NS_LOG_INFO ("                                                                                 = " << m_bitRate );

      uint32_t old = m_bitRate;
      //  double diff = m_lastDt >= 0 ? (currDt - m_lastDt).GetSeconds() : 0;

      NS_LOG_INFO ("                                                                    Time bufferDelay;");
      Time bufferDelay;

      //m_player.CalcNextSegment(m_bitRate, m_player.GetBufferEstimate(), diff,
      //m_bitRate, bufferDelay);

      NS_LOG_INFO ("                                                                    uint32_t prevBitrate = m_bitRate;");
      NS_LOG_INFO ("                                                                                         = " << m_bitRate );      
      uint32_t prevBitrate = m_bitRate;

      NS_LOG_INFO ("                                                           // CalcNextSegment ?? za3ma ");         
      NS_LOG_INFO ("                                                              CalcNextSegment (prevBitrate, m_bitRate, bufferDelay) ");         
      NS_LOG_INFO ("                                                                              ( " << prevBitrate << "," << m_bitRate << "," 
                                                                                                       << bufferDelay << ")");                                                                                                                
     
      CalcNextSegment (prevBitrate, m_bitRate, bufferDelay);
      // should we do old or prevBitRate here ?? 

      NS_LOG_INFO ("                                                           // prevBitrate != m_bitRate ??");         
      NS_LOG_INFO ("                                                              prevBitrate = " <<  prevBitrate);
      NS_LOG_INFO ("                                                              m_bitRate = " <<  m_bitRate);
      if (prevBitrate != m_bitRate)
        {
          NS_LOG_INFO ("                                                               Yes it is --> m_rateChanges++ ");                   
          m_rateChanges++;
        }
      else {
          NS_LOG_INFO ("                                                               Walou --> do nothing ");
      }


         NS_LOG_INFO ("                                                                       segmentTime (duration): " 
                        << m_segmentFetchTime.GetSeconds ());
         NS_LOG_INFO ("                                                                       segmentRate: " 
                        << 8 * m_segment_bytes / m_segmentFetchTime.GetSeconds ());                               


      // ---------------------
      // Some Stats to log
      // ---------------------------------------------------------
         NS_LOG_INFO ("                          // Some Stats :");         
         NS_LOG_INFO ("                             timeStamp : " << Simulator::Now ().GetSeconds () << " Node: " << m_id
                                  << " newBitRate: " << m_bitRate << " oldBitRate: " << old
                                  << " estBitRate: " << GetBitRateEstimate ()); //achieved throughput over the segment
         NS_LOG_INFO ("                             InterruptionTime: " 
                                  << m_player.m_interruption_time.GetSeconds ()
                                  << " T (bufferTime): " << currDt.GetSeconds ()
                                  << " dT (deltaBufferTime): " << (m_lastDt >= Time ("0s") ? (currDt - m_lastDt).GetSeconds () : 0)
                                  << " delayToNxtReq : " << bufferDelay.GetSeconds () << std::endl);
      // --------------------------------------------------------------------------------------------      
   
      // ---------------------
      // Some Stats to log
      // ---------------------------------------------------------
         m_outDashStatsByNodeFile << Simulator::Now ().GetSeconds () << "s," << m_id // timeStamp,Node
                                  << "," << m_bitRate << "," << old // newBitRate,oldBitRate
                                  << "," << GetBitRateEstimate ()   // estBitRate (achieved throughput over the 
                                                                    // last m_windows of received segments)
                                                                    // about m_bitrate and m_bitRate_estimate
                                  << "," << m_player.m_interrruptions // Interruptions 
                                  << "," << m_player.m_interruption_time.GetSeconds () << "s" // InterruptionTime
                                  << "," << round_to(currDt.GetSeconds (), 0.01) << "s" // T or currDt or bufferTime or realplayTime
                                                                                        // this is GetRealPlaytime 
                                                                                        // (mpeg_header.GetPlaybackTime())
                                                                                        // currDt does not seem to indicate 
                                                                                        // the current size of the mpeg player buffer, 
                                                                                        // but is still refered to as buffering time 
                                  << "," << (m_lastDt >= Time ("0") ? (currDt - m_lastDt).GetSeconds () : 0) << "s"// dT
                                  << "," << round_to(GetBufferEstimate (), 0.01) << "s" // estimated/avg bufferTime                                  
                                  << "," << bufferDelay.GetSeconds () << "s" // delay / segmentTime (duration)
                                  << "," << 8 * m_segment_bytes / m_segmentFetchTime.GetSeconds () // segmentRate
                                  << "," << m_player.m_init_buff << "s" << std::endl; // initial buffering time
                               // Some metrics to recall : 
                               //     InterruptionTime : m_player.m_interruption_time.GetSeconds ()
                               //     interruptions : m_player.m_interrruptions
                               //     avgRate : (1.0 * m_player.m_totalRate) / m_player.m_framesPlayed
                               //     minRate : m_player.m_minRate
                               //     AvgDt : m_sumDt.GetSeconds () / m_player.m_framesPlayed
                               //     changes : m_rateChanges << std::endl;
                               

                               //     m_bitRate : the next bit rate to ask for
                               //     bufferDelay : time to wait before requesting the next seg
                               //        both of these metrics are calculated by the ABR algo (CalcNextSegment () method) 
                               //        based on :  1- GetBitRateEstimate ()    : average delay endured by each of the previous segments
                               //                    2- GetBufferDifferential () : which returns the difference (gap) between 
                               //                                                  the two last entries loged in bufferState structure 
                               //                                                  i.e. delays of the two penultimate segments

          // "timeStamp,Node,newBitRate,oldBitRate,estBitRate,interruptions,InterruptionTime,"
          // "bufferTime,deltaBufferTime,estAvgBufferTime,delay_segmentTime,segmentRate,initBuffer" 

      // ------------------------------------------------------------------


      // ---------------------
      // Some Stats to log
      // ---------------------------------------------------------
         m_outSegStatsByNodeFile  << Simulator::Now ().GetSeconds () << "s" //timestamp
                                  << "," << m_segmentId
                                  << "," << mpegHeader.GetFrameId ()
                                  << "," << m_id // timeStamp,Node
                                  << "," << m_player.m_framesPlayed // played frames
                                  << "," << m_requestTime.GetSeconds () << "s" // m_segmentFetchTime = Simulator::Now () - m_requestTime;
                                  << "," << m_segmentFetchTime.GetSeconds () << "s" // m_segmentFetchTime = Simulator::Now () - m_requestTime;
                                  << "," << (uint32_t) m_segment_bytes/1024  << "KB" // segment size
                                  << "," << round_to(currDt.GetSeconds (), 0.01) << "s"// T (bufferTime): 
                                  << "," << (m_lastDt >= Time ("0s") ? (currDt - m_lastDt).GetSeconds () : 0) << "s" //dT (deltaBufferTime):
                                  << "," << round_to(GetBufferEstimate (), 0.01) << "s" // estimated/avg bufferTime                                                                    
                                  << "," << (uint32_t) m_bitRate/1000 << "kbps," << (uint32_t) old/1000 << "kbps" // newBitRate,oldBitRate
                                  << "," << m_rateChanges // rate changes
                                  << "," << (uint32_t) GetBitRateEstimate ()/1000 << "kbps" // estBitRate
                                  << "," << round_to(8 * m_segment_bytes / m_segmentFetchTime.GetSeconds () / 1000, 0.01) << "kbps"// rounded segmentRate
                                  << "," << m_player.m_interrruptions // Interruptions 
                                  << "," << m_player.m_interruption_time.GetSeconds () << "s" // InterruptionTime
                                  << "," << m_player.m_frameBuffer.size ()// queueLength
                                  << "," << m_player.m_frameBuffer.sizeInBytes () / 1024 << "KB" // queueLengthInBytes
                                  << "," << m_player.m_init_buff << "s" << std::endl; // initial buffering time


         NS_LOG_INFO ("                                                           Last frame received. Requesting segment " << m_segmentId);
         NS_LOG_INFO ("==== Last frame received. Requesting segment " << m_segmentId);
      // --------------------------------------------------------------------------




      NS_LOG_INFO ("                                                           // bufferDelay == Seconds (0) ??");         
      NS_LOG_INFO ("                                                                          = " <<  bufferDelay.GetSeconds ());         
      if (bufferDelay == Seconds (0))
        {
          NS_LOG_INFO ("                                                               Kayna --> Requesting frame immediately due zero buffer delay");
          NS_LOG_INFO ("                                                                     --> RequestSegment");
          NS_LOG_INFO ("Requesting frame immediately due zero buffer delay");
          RequestSegment ();
        }
      else
        {
          NS_LOG_INFO ("                                                               walou  --> Schedule (bufferDelay, &DashClient::RequestSegment, this) ");
          NS_LOG_INFO ("                                                                                   (" << bufferDelay << ", &DashClient::RequestSegment, this) ");
          Simulator::Schedule (bufferDelay, &DashClient::RequestSegment, this);
        }


      (void) old;
      NS_LOG_INFO ("                                                           !@#$#@!$@#\t" << Simulator::Now ().GetSeconds () << " old: " << old
      // NS_LOG_INFO ("!@#$#@!$@#\t" << Simulator::Now ().GetSeconds () << " old: " << old
                                  << " new: " << m_bitRate << " t: " << currDt.GetSeconds ()
                                  << " dt: " << (currDt - m_lastDt).GetSeconds ());
      NS_LOG_INFO ("                                                           m_lastDt = currDt;");
      m_lastDt = currDt;
      m_receivedSegments++;
    } // did we received the last frame of the segmentReceived frame ??
    else
    {
      NS_LOG_INFO ("                                                           No, we did not ...");               
      NS_LOG_INFO ("                                                           (reminder : about the question is this");
      NS_LOG_INFO ("                                                                       the last frame of the segmentReceived  \?\?)");
    }

  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::MessageReceived DONE  ");
  NS_LOG_INFO ("    ----------------------------------------------------------------");  
  NS_LOG_INFO ("");               

  return true;
}

void
DashClient::CalcNextSegment (uint32_t currRate, uint32_t &nextRate, Time &delay)
{
  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::CalcNextSegment ()  ");
  NS_LOG_INFO ("                                  dashClient (" << m_id << ")");   
  NS_LOG_INFO ("                                  // currRate = " << currRate);
  NS_LOG_INFO ("                                  // nextRate = " << nextRate );
  NS_LOG_INFO ("                                  // delay = " << delay );  
  NS_LOG_INFO ("");               

  nextRate = currRate;
  delay = Seconds (0);

  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::CalcNextSegment () DONE  ");
  NS_LOG_INFO ("    ------------------------------------------------------");      
  NS_LOG_INFO ("");                 
}

void
DashClient::GetStats ()
{
  std::cout << " InterruptionTime: " << m_player.m_interruption_time.GetSeconds ()
            << " interruptions: " << m_player.m_interrruptions
            << " avgRate: " << (1.0 * m_player.m_totalRate) / m_player.m_framesPlayed
            << " minRate: " << m_player.m_minRate
            << " AvgDt: " << m_sumDt.GetSeconds () / m_player.m_framesPlayed
            << " changes: " << m_rateChanges << std::endl;
}

void
DashClient::InitializeCSVs ()
{
  NS_LOG_FUNCTION (this); 
  std::string filenameDashStatsByNode = "dashStatsByNode_" + std::to_string(m_id) + ".csv";
  m_outDashStatsByNodeFile.open (filenameDashStatsByNode.c_str ()); m_outDashStatsByNodeFile.setf (std::ios_base::fixed);    
  m_outDashStatsByNodeFile << "timeStamp,Node,newBitRate,oldBitRate,estBitRate,interruptions,InterruptionTime,"
                              "bufferTime,deltaBufferTime,estAvgBufferTime,delay_segmentTime,segmentRate,initBuffer" 
                           << std::endl;                 
  // m_outDashStatsByNodeFile << "timeStamp,Node,newBitRate,oldBitRate,estBitRate,interTime,T,dT,delay,segmentTime,segmentRate" 

  std::string filenameSegStatsByNode = "dashSegmentStatsByNode_" + std::to_string(m_id) + ".csv";
  m_outSegStatsByNodeFile.open (filenameSegStatsByNode.c_str ()); m_outSegStatsByNodeFile.setf (std::ios_base::fixed);    
  m_outSegStatsByNodeFile << "timestamp,segmentId,frameId,node,playedFrames,requestTime,segmentFetchTime,segmentSize,bufferTime,"
                             "deltaBufferTime,estAvgBufferTime,newBitRate,oldBitRate,changes,estBitRate,segmentRate,interruptions,interruptionTime,"
                             "queueLength,queueSize,initBuffer"
                           << std::endl;    
}


// std::stringstream 
std::string
DashClient::GetStatsCsv ()
{
  std::stringstream output;
  output << m_player.m_interruption_time.GetSeconds () << "s," //InterruptionTime
         << m_player.m_interrruptions << "," //Interruptions
         << m_player.m_framesPlayed << "," //framesPlayed
         << (uint32_t) ( (1.0 * m_player.m_totalRate) / m_player.m_framesPlayed / 1000) << "kbps," //avgRateByFrame
         << (uint32_t) ( (1.0 * m_player.m_totalRate) / m_totalSegFetchTime.GetSeconds () / 1000) << "kbps," //avgRateBytes
         << (uint32_t) ( m_player.m_maxRate / 1000 ) << "kbps," //maxRate
         << (uint32_t) ( m_player.m_minminRate / 1000 ) << "kbps," //minminRate
         << (uint32_t) ( m_player.m_minRate / 1000 ) << "kbps," //minRate
         << (uint32_t) GetBitRateEstimate ()/1000 << "kbps," // avgBitRate         
         << m_sumDt.GetSeconds () / m_player.m_framesPlayed << "s," //avgDt
         << round_to(GetBufferEstimate (), 0.01) << "s," // estimated/avg bufferTime                                  
         << m_rateChanges << "," // changes
         << m_player.m_init_buff << "s," // initBuffer
         << m_player.m_frameBuffer.size () << ","//   
         << m_player.m_frameBuffer.sizeInBytes () / 1024 << "KB"  << std::endl; // queueLengthInBytes

  return output.str();         
}


void
DashClient::LogBufferLevel (Time t)
{

  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::LogBufferLevel (Time t)  ");
  NS_LOG_INFO ("                                  dashClient (" << m_id << ")");   
  NS_LOG_INFO ("                                  // t = " << t);  
  NS_LOG_INFO ("                                     m_bufferState[Simulator::Now ()] = t");
  NS_LOG_INFO ("                                                  [" << Simulator::Now ()
                                                                       << "] = " << t);
  NS_LOG_INFO ("                                     // recall to erase buffer (depending on m_windows = " << m_window << ")" );
  NS_LOG_INFO ("                                  // m_start_time = (" << m_player.m_start_time.GetSeconds () << "s)");     
  NS_LOG_INFO ("");      


  m_bufferState[Simulator::Now ()] = t;
  for (auto it = m_bufferState.cbegin (); it != m_bufferState.cend ();)
    {
      if (it->first < (Simulator::Now () - m_window))
        {
          m_bufferState.erase (it++);
        }
      else
        {
          ++it;
        }
    }
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] DashClient::LogBufferLevel () done  ");
  NS_LOG_INFO ("    ------------------------------------------------------");      
  NS_LOG_INFO ("");          
}

double
DashClient::GetBufferEstimate ()
{
  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << " GetBufferEstimate ()  ");
  NS_LOG_INFO ("");      

  double sum = 0;
  int count = 0;
  for (std::map<Time, Time>::iterator it = m_bufferState.begin (); it != m_bufferState.end (); ++it)
    {
      sum += it->second.GetSeconds ();
      count++;
    }
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << " GetBufferEstimate ()   DONE ");
  NS_LOG_INFO ("    ------------------------------------------------------");      
  NS_LOG_INFO ("");      

  return sum / count;

}

double
DashClient::GetBufferDifferential ()
{
  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now () << "] DashClient::GetBufferDifferential  ");
  NS_LOG_INFO ("                                  dashClient (" << m_id << ")");   

  NS_LOG_INFO ("");      

  std::map<Time, Time>::iterator it = m_bufferState.end ();

  if (it == m_bufferState.begin ())
    {
      // Empty buffer
      return 0;
    }
  it--;
  Time last = it->second;

  if (it == m_bufferState.begin ())
    {
      // Only one element
      return 0;
    }
  it--;
  Time prev = it->second;

  NS_LOG_INFO ("                                     return (last - prev).GetSeconds ()");   
  NS_LOG_INFO ("                                            (" << last << "-" << prev << ").GetSeconds ())");   
  NS_LOG_INFO ("                                            (" << (last - prev).GetSeconds () << ")");   
  NS_LOG_INFO ("                                     recall we are calculating based on m_bufferState map <time, time>");   

  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] GetBufferDifferential   DONE ");
  NS_LOG_INFO ("    ------------------------------------------------------");      
  NS_LOG_INFO ("");      


  return (last - prev).GetSeconds ();
}

double
DashClient::GetSegmentFetchTime ()
{
  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] GetSegmentFetchTime  ");
  NS_LOG_INFO ("");      

  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] GetSegmentFetchTime   DONE ");
  NS_LOG_INFO ("    ------------------------------------------------------");      
  NS_LOG_INFO ("");      


  return m_segmentFetchTime.GetSeconds ();
}

void
DashClient::AddBitRate (Time time, double bitrate)
{
  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << " DashClient::AddBitRate (Time time, double bitrate)"  );
  NS_LOG_INFO ("                                  dashClient (" << m_id << ")");   
  NS_LOG_INFO ("                                  time =  (" << time << ")");   
  NS_LOG_INFO ("                                  bitrate =  (" << bitrate << ")");   
  NS_LOG_INFO ("                                  // m_start_time = (" << m_player.m_start_time.GetSeconds () << "s)");   
  NS_LOG_INFO ("");      

  m_bitrates[time] = bitrate;
  double sum = 0;
  int count = 0;
  NS_LOG_INFO ("                                  m_bitrates[time] = bitrate");         
  NS_LOG_INFO ("                                  let calculate sum and count first");         
  NS_LOG_INFO ("                                  // this take in consideration m_window = ( " << m_window << " )");         
  for (auto it = m_bitrates.cbegin (); it != m_bitrates.cend ();)
    {
      if (it->first < (Simulator::Now () - m_window))
        {
          m_bitrates.erase (it++);
        }
      else
        {
          sum += it->second;
          count++;
          ++it;
        }
    }
  NS_LOG_INFO ("                                  now, m_bitrateEstimate = sum / count");       
  m_bitrateEstimate = sum / count;
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << " DashClient::AddBitRate::AddBitRate  DONE"  );
  NS_LOG_INFO ("    ------------------------------------------------------");      
  NS_LOG_INFO ("");      
}

} // Namespace ns3
