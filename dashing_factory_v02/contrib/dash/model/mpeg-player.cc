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

#include "ns3/log.h" 
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "http-header.h"
#include "mpeg-header.h"
#include "mpeg-player.h"
#include "dash-client.h"
#include <cmath>

NS_LOG_COMPONENT_DEFINE ("MpegPlayer");
namespace ns3 {

FrameBuffer::FrameBuffer (uint32_t &capacity) : m_capacity (capacity)
{
}

bool
FrameBuffer::push (Ptr<Packet> frame)
{
  NS_LOG_FUNCTION (this);
  if (m_size_in_bytes + frame->GetSerializedSize () <= m_capacity)
    {
      m_queue.push (frame);
      NS_LOG_INFO ("pushing packet with size = " << frame->GetSize () << " serialized = "
                                                 << frame->GetSerializedSize ());

      m_size_in_bytes += frame->GetSerializedSize ();
      return true;
    }
  else
    {
      return false;
    }
}

Ptr<Packet>
FrameBuffer::pop ()
{
  NS_LOG_FUNCTION (this);
  Ptr<Packet> frame = m_queue.front ();
  m_queue.pop ();
  NS_LOG_INFO ("popping packet with size = " << frame->GetSize ()
                                             << " serialized = " << frame->GetSerializedSize ());

  m_size_in_bytes -= frame->GetSerializedSize ();
  return frame;
}

int
FrameBuffer::size ()
{
  NS_LOG_FUNCTION (this);
  return m_queue.size ();
}

int
FrameBuffer::sizeInBytes ()
{
  NS_LOG_FUNCTION (this);
  return m_size_in_bytes;
}

bool
FrameBuffer::empty ()
{
  NS_LOG_FUNCTION (this);
  return m_queue.empty ();
}

 


MpegPlayer::MpegPlayer (Ptr<DashClient> dashClient, uint32_t &capacity)
    : m_state (MPEG_PLAYER_NOT_STARTED),
      m_interrruptions (0),    
      m_totalRate (0),    
      m_minRate (100000000),
      m_maxRate (0),      
      m_minminRate (100000000),
      m_framesPlayed (0),      
      m_frameBuffer (capacity),
      m_init_buff (0),
      m_firstPlay (false),
      m_firstBuff (Seconds (0)),                                  
      m_bufferDelay ("0s"),
      m_dashClient (dashClient)
{
  NS_LOG_FUNCTION (this);
}

MpegPlayer::~MpegPlayer ()
{
  NS_LOG_FUNCTION (this);
}

int
MpegPlayer::GetQueueSize ()
{
  return m_frameBuffer.size ();
}

Time
MpegPlayer::GetRealPlayTime (Time playTime)
{

NS_LOG_INFO ("    ------------------------------------------------------");
NS_LOG_INFO ("    [" << Simulator::Now () << "] MpegPlayer::GetRealPlayTime (Time playTime)  ");
NS_LOG_INFO ("                               dClient = (" << m_dashClient->m_id << ")");
NS_LOG_INFO ("                               m_state : [" << m_state << "]");
NS_LOG_INFO ("                               // I think calculating real playtime from a specefied playTime (" << playTime.GetSeconds () << "s)");
NS_LOG_INFO ("                                  i.e. how much we had realy consumed until <playTime>");
NS_LOG_INFO ("                                       this take into consideration paused time, etc");
NS_LOG_INFO ("                                                               why ALLAH a3lem :/");
NS_LOG_INFO ("");               


  // NS_LOG_INFO (" Start: " << m_start_time.GetSeconds ()
     NS_LOG_INFO ("                               Start: " << m_start_time.GetSeconds () << "s " 
                          << " Inter: " << m_interruption_time.GetSeconds ()
                          << " playtime: " << playTime.GetSeconds ()
                          << " now: " << Simulator::Now ().GetSeconds () << " actual: "
                          << (m_start_time + m_interruption_time + playTime).GetSeconds ());
     NS_LOG_INFO ("                               //(m_start_time + m_interruption_time + playTime).GetSeconds ())");
     NS_LOG_INFO ("");
     NS_LOG_INFO ("                               return (" << (m_start_time + m_interruption_time +
                                                                (m_state == MPEG_PLAYER_PAUSED ? (Simulator::Now () - m_lastpaused) : Seconds (0)) +
                                                                playTime - Simulator::Now ()).GetSeconds () << "s)");


NS_LOG_INFO ("    [" << Simulator::Now () << "] MpegPlayer::GetRealPlayTime (DONE)  ");
NS_LOG_INFO ("    ------------------------------------------------------");

  return m_start_time + m_interruption_time +
         (m_state == MPEG_PLAYER_PAUSED ? (Simulator::Now () - m_lastpaused) : Seconds (0)) +
         playTime - Simulator::Now ();
}

bool
MpegPlayer::ReceiveFrame (Ptr<Packet> message)
{
  NS_LOG_FUNCTION (this << message);
  NS_LOG_INFO ("Received Frame " << m_state);

  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now () << "] MpegPlayer::ReceiveFrame (Ptr<Packet> message)  ");
  NS_LOG_INFO ("                               dClient = (" << m_dashClient->m_id << ")");
  NS_LOG_INFO ("                               m_state : " << m_state);
  NS_LOG_INFO ("                               // push msg into m_frameBuffer, update m_state, call playFrame ");
  NS_LOG_INFO ("");               

  Ptr<Packet> msg = message->Copy ();

  NS_LOG_INFO ("                               !m_frameBuffer.push (msg) ???");
  NS_LOG_INFO ("                               <------ WE ARE BUFFERING a msg just received NOW");
  if (!m_frameBuffer.push (msg))
    {
      NS_LOG_INFO ("                                   Yes it is, return False");      
      return false;
    }
  else {
      NS_LOG_INFO ("                                   No it is not, do nothing, continu ...");      
  }

  NS_LOG_INFO ("                               m_state == MPEG_PLAYER_PAUSED ???? ");
  if (m_state == MPEG_PLAYER_PAUSED)
    {
      NS_LOG_INFO ("                               YESS --> Play resumed ");
      NS_LOG_INFO ("                                        m_state = MPEG_PLAYER_PLAYING ");
      NS_LOG_INFO ("Play resumed");
      m_state = MPEG_PLAYER_PLAYING;
      NS_LOG_INFO ("                                        m_interruption_time += (Simulator::Now () - m_lastpaused); ");      
      m_interruption_time += (Simulator::Now () - m_lastpaused);
      NS_LOG_INFO ("                                        PlayFrame (); ");            
      PlayFrame ();
    }
  else {
    NS_LOG_INFO ("                               No it is not, next question :");
    NS_LOG_INFO ("                               // aslan, m_state ? --> is " << m_state);
    NS_LOG_INFO ("                               m_state == MPEG_PLAYER_NOT_STARTED ???? ");    
    if (m_state == MPEG_PLAYER_NOT_STARTED)
     {
       NS_LOG_INFO ("                               Yes (MPEG_PLAYER_NOT_STARTED) ");
       NS_LOG_INFO ("                               --> Play started after 1sec");
       NS_LOG_INFO ("Play started");
       NS_LOG_INFO ("                                        m_state = MPEG_INITIAL_BUFFERING ");      
       m_state = MPEG_INITIAL_BUFFERING;
       if (m_firstPlay == false) {
           m_firstBuff = Simulator::Now ();
           NS_LOG_INFO ("                                        ********************************");             
           NS_LOG_INFO ("                                        FIRST BUFFERING EVENT = " << m_firstBuff.GetSeconds () << "s");             
           NS_LOG_INFO ("                                        m_firstPlay = " << m_firstPlay);             
           NS_LOG_INFO ("                                        ********************************");             
       }
       NS_LOG_INFO ("                                        Schedule (Sec(1), &PlayFrame ");            
       Simulator::Schedule (Seconds (1), &MpegPlayer::PlayFrame, this);

     }
    else {
       NS_LOG_INFO ("                               No (mstate != MPEG_PLAYER_NOT_STARTED) --> continu");
       NS_LOG_INFO ("                               // aslan m_state = " << m_state);
    }
  }
  NS_LOG_INFO ("    [" << Simulator::Now () << "] MpegPlayer::ReceiveFrame () DONE  ");
  NS_LOG_INFO ("    ------------------------------------------------------");    
  return true;
}

void
MpegPlayer::Start (void)
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] MpegPlayer::start ()  ");
  NS_LOG_INFO ("                               dClient = (" << m_dashClient->m_id << ")");
  NS_LOG_INFO ("                               m_state = " << m_state);
  NS_LOG_INFO ("");  
  NS_LOG_INFO ("                               m_state ==> MPEG_PLAYER_PLAYING");             
  m_state = MPEG_PLAYER_PLAYING;
  m_interruption_time = Seconds (0);
  NS_LOG_INFO ("    [" << Simulator::Now () << "] MpegPlayer::start () DONE  ");
  NS_LOG_INFO ("    ------------------------------------------------------");      
}

void
MpegPlayer::PlayFrame (void)
{
  NS_LOG_FUNCTION (this);
  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now ().GetSeconds () << "] MpegPlayer::PlayFrame ()  ");
  NS_LOG_INFO ("                               dClient = (" << m_dashClient->m_id << ")");
  NS_LOG_INFO ("                               m_state = " << m_state);
  NS_LOG_INFO ("");     


  if (m_state == MPEG_PLAYER_DONE)
    {
      NS_LOG_INFO ("                               m_state == MPEG_PLAYER_DONE --> zidd");      
      return;
    }
  else if (m_state == MPEG_INITIAL_BUFFERING)
    {
      NS_LOG_INFO ("                               m_state = MPEG_INITIAL_BUFFERING --> ");
      NS_LOG_INFO ("                                   m_start_time before affectation = " << m_start_time << "s");
      NS_LOG_INFO ("                                   m_start_time = Now | m_state = MPEG_PLAYER_PLAYING");
      m_start_time = Simulator::Now ();
      NS_LOG_INFO ("                                   m_start_time = " << m_start_time.GetSeconds () << "s" );      
      m_state = MPEG_PLAYER_PLAYING;
      if (m_firstPlay == false) {
          m_init_buff = (Simulator::Now () - m_firstBuff).GetSeconds ();
          m_firstPlay = true;   
          NS_LOG_INFO ("                                        ********************************");             
          NS_LOG_INFO ("                                        FIRST PLAYING EVENT = " << Simulator::Now ().GetSeconds () << "s");             
          NS_LOG_INFO ("                                        m_init_buff = " << m_init_buff << "s");             
          NS_LOG_INFO ("                                        m_firstPlay = " << m_firstPlay);             
          NS_LOG_INFO ("                                        ********************************");                              
      }
    }
  if (m_frameBuffer.empty ())
    {
      NS_LOG_INFO (Simulator::Now ().GetSeconds () << " No frames to play");
      NS_LOG_INFO ("                               m_frameBuffer.empty () --> m_state = MPEG_PLAYER_PAUSED; ");
      NS_LOG_INFO ("                                                          m_lastpaused = Now");      
      NS_LOG_INFO ("                                                          " << Simulator::Now ().GetSeconds () << " No frames to play"); 
      NS_LOG_INFO ("                                                          m_interrruptions++ --> return ");      
      m_state = MPEG_PLAYER_PAUSED;
      m_lastpaused = Simulator::Now ();
      m_interrruptions++;
      return;
    }

  NS_LOG_INFO ("                               Extract msg --> Ptr<Packet> message");
  NS_LOG_INFO ("                                               m_frameBuffer.size = " << m_frameBuffer.size ());
  NS_LOG_INFO ("                                               Ptr<Packet> message = m_frameBuffer.pop (); ");
  NS_LOG_INFO ("                                               (new) m_frameBuffer.size = " << m_frameBuffer.size ());  
  Ptr<Packet> message = m_frameBuffer.pop ();

  MPEGHeader mpeg_header;
  HTTPHeader http_header;

  NS_LOG_INFO ("                                               extract http/mpeg header ... ");
  message->RemoveHeader (http_header);
  message->RemoveHeader (mpeg_header);

  NS_LOG_INFO ("                                               increment m_totalRate ... ");
  NS_LOG_INFO ("                                               // Discard the first segment for the minRate... ");
  NS_LOG_INFO ("                                               // calculation, as it is always the minimum rate... ");
  NS_LOG_INFO ("                                               // recall we are talking about segment : [" 
                                                                    << http_header.GetSegmentId () << "]");
  m_totalRate += http_header.GetResolution (); // resolution is in bps
  NS_LOG_INFO ("                                               httpheader.GetSegmentId () > 0 ???");  
  m_minminRate = http_header.GetResolution () < m_minminRate ?
                 http_header.GetResolution () : 
                 m_minminRate;

  NS_LOG_INFO ("                                                 // m_maxRate = max between (" << http_header.GetResolution ()
                                                                                                 << "), and (" <<  m_maxRate 
                                                                                                 << ") ...");                                                                                                                          
  m_maxRate = http_header.GetResolution () > m_maxRate ?
              http_header.GetResolution () : 
              m_maxRate;  
  NS_LOG_INFO ("                                                 //           = " << m_maxRate);

  if (http_header.GetSegmentId () > 0) // Discard the first segment for the minRate
    {                                  // calculation, as it is always the minimum rate
      NS_LOG_INFO ("                                                    YES it is, calculate m_minRate (and minminBitrate)"); 
      NS_LOG_INFO ("                                                 // m_minRate = min between (http_header.GetResol), and (old m_minRate) ...");           
      NS_LOG_INFO ("                                                 // m_minRate = min between (" << http_header.GetResolution ()
                                                                                                     << "), and (" <<  m_minRate 
                                                                                                     << ") ...");           
      m_minRate = http_header.GetResolution () < m_minRate ?
                  http_header.GetResolution () : 
                  m_minRate;
      NS_LOG_INFO ("                                                 // m_minRate = " << m_minRate << "bps");                             
    }
  else
    {
      NS_LOG_INFO ("                                                NO it is not (indeed segmentId = 0)");              
    }     
  NS_LOG_INFO ("                                               m_framesPlayed = " << m_framesPlayed << ", increment now ++;");    
  m_framesPlayed++;

  /*std::cerr << "res= " << http_header.GetResolution() << " tot="
     << m_totalRate << " played=" << m_framesPlayed << std::endl;*/

  // Time b_t = GetRealPlayTime (mpeg_header.GetPlaybackTime ()) + Seconds(m_frameBuffer.size() * MPEG_TIME_BETWEEN_FRAMES);

  // if (m_bufferDelay > Time ("0s") && b_t < m_bufferDelay && m_dashClient)
  //   {
  //     NS_LOG_INFO("Requesting frame due low buffer time, b_t = " <<  b_t << " m_bufferDelay = " <<  m_bufferDelay);
  //     m_dashClient->RequestSegment ();
  //     m_bufferDelay = Seconds (0);
  //     // m_dashClient = NULL;
  //   }

  // NS_LOG_INFO (Simulator::Now ().GetSeconds ()

  NS_LOG_INFO (    "                                               " 
                     "/~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
  NS_LOG_INFO (    "                                               | "
                     " >> PLAYING FRAME  [ player" << m_dashClient->m_id 
                      << " at " << Simulator::Now ().GetSeconds () << "s]");
  NS_LOG_INFO (    "                                               |    " 
                     " VidId: " << http_header.GetVideoId () 
                  << " SegId: " <<   http_header.GetSegmentId () 
                  << " Res: " << http_header.GetResolution () 
                  << " FrameId: " << mpeg_header.GetFrameId ());
  NS_LOG_INFO (    "                                               |    " 
                  << " PlayTime: " << mpeg_header.GetPlaybackTime ().GetSeconds ()
                  << " Type: " << (char) mpeg_header.GetType () 
                  << " Size: " << mpeg_header.GetSize ()
                  << " interTime: " << m_interruption_time.GetSeconds ()
                  << " queueLength: " << m_frameBuffer.size ());
  NS_LOG_INFO (    "                                               " 
                     "\\~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");

  /*   >> atsp << <   << " playtime: " << mpeg_header.GetPlaybackTime() );
     << " target: " << (m_start_time + m_interruption_time + mpeg_header.GetPlaybackTime()).GetSeconds()
     << " now: " << Simulator::Now().GetSeconds()
     << std::endl;
     */
  NS_LOG_INFO ("                                               Schedule (msec (20), &PlayFrame");        
  Simulator::Schedule (MilliSeconds (20), &MpegPlayer::PlayFrame, this);

  NS_LOG_INFO ("                                               // There may be space now to read a new packet from the socket");        
  // There may be space now to read a new packet from the socket
  if (m_dashClient)
    {
      NS_LOG_INFO ("                                               Schedule (msec (0), &CheckBuffer");              
      Simulator::Schedule (MilliSeconds (0), &DashClient::CheckBuffer, m_dashClient);
    }
  NS_LOG_INFO ("    [" << Simulator::Now () << "] MpegPlayer::PlayFrame () DONE  ");
  NS_LOG_INFO ("    ------------------------------------------------------");    

}

} // namespace ns3
