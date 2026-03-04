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

#include "http-parser.h"
#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/socket.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "http-header.h"
#include "mpeg-header.h"
#include "dash-client.h"

NS_LOG_COMPONENT_DEFINE ("HttpParser");

namespace ns3 {

HttpParser::HttpParser () : m_app (NULL), m_lastmeasurement ("0s")
{
  NS_LOG_FUNCTION (this);
}

HttpParser::~HttpParser ()
{
  NS_LOG_FUNCTION (this);
}

void
HttpParser::SetApp (DashClient *app)
{
  NS_LOG_FUNCTION (this << app);
  m_app = app;
}

void
HttpParser::ReadSocket (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now () << "] HttpParser::ReadSocket (socket)  ");
  NS_LOG_INFO ("                               from node" << socket->GetNode ()->GetId ());
  NS_LOG_INFO ("                               dashClient (" << m_app->m_id << ")");         
  NS_LOG_INFO ("                               // Extract pkt from socket and add it into the end of m_pending_packet");         
  NS_LOG_INFO ("                               // ... call TryToPushToPlayer ...");         
  NS_LOG_INFO ("");             


  NS_LOG_INFO ("                               m_pending_packet ?");
  if (m_pending_packet)
    {
      NS_LOG_INFO ("                                   Yes it is -> TryToPushToPlayer ()");      
      TryToPushToPlayer ();
    }
  else
  {
      NS_LOG_INFO ("                                   No It is not -> do nothing (noo TryToPushToPlayer)");      
  }

  NS_LOG_INFO ("                              Reminder ... ");          
  NS_LOG_INFO ("                              // we are inside HttpParser::ReadSocket (socket) ... ");          
  NS_LOG_INFO ("                              // dashClient (" << m_app->m_id << ")");
  NS_LOG_INFO ("                              ");          
  NS_LOG_INFO ("                                m_pending_p && size (m_pending_p) > m_pending_message_size ?");
  NS_LOG_INFO ("                                // i.e. is our play buffer is likely full ??");
  if (m_pending_packet && m_pending_packet->GetSize () >= m_pending_message_size)
    {
      NS_LOG_INFO ("                                   Yes it is -> ");      
      NS_LOG_INFO ("                                   -> Not reading socket, our play buffer is likely full ");      
      NS_LOG_INFO ("Not reading socket, our play buffer is likely full ");
      // Send dummy packet to prevent deadlock
      // due to TCP "Silly Window"
      // See also: https://www.nsnam.org/bugzilla/show_bug.cgi?id=1565
      m_app->SendBlank();
      return;
    }
  else
  {
      NS_LOG_INFO ("                                   No It is not (is buffer full??) -> do nothing ");      
  }  

  if (socket->GetRxAvailable () <= 0)
    {
      NS_LOG_INFO ("Not reading socket, nothing to read");
      return;
    }

  NS_LOG_INFO ("                               Ptr<Packet> pkt = socket->Recv ();");
  Ptr<Packet> pkt = socket->Recv ();
  NS_LOG_INFO ("                                           pkt size = " << pkt->GetSize () << " bytes");  

  if (pkt == NULL)
    {
      NS_LOG_INFO ("Received NULL packet");
      return;
    }

  if (m_pending_packet == NULL)
    {
      NS_LOG_INFO ("                               m_pending_packet = pkt (m_pending_packet is NULL)");
      m_pending_packet = pkt;
    }
  else
    {
      NS_LOG_INFO ("                               m_pending_packet->AddAtEnd (pkt) (m_pending_packet is not NULL)");
      m_pending_packet->AddAtEnd (pkt);
    }

  NS_LOG_INFO ("                               TryToPushToPlayer ( -finale- )");
  TryToPushToPlayer ();

  NS_LOG_INFO ("    [" << Simulator::Now () << "] HttpParser::ReadSocket (socket)  ");  
  NS_LOG_INFO ("    ------------------------------------------------------");
}

void
HttpParser::TryToPushToPlayer ()
{
  NS_LOG_FUNCTION (this);

  NS_LOG_INFO ("    ------------------------------------------------------");
  NS_LOG_INFO ("    [" << Simulator::Now () << "] HttpParser::TryToPushToPlayer ()  ");
  NS_LOG_INFO ("                               dashClient (" << m_app->m_id << ")");       
  NS_LOG_INFO ("                               // start from  m_pending_packet");
  NS_LOG_INFO ("");
  NS_LOG_INFO ("                               while (true) ->");
  while (true)
    {
      MPEGHeader mpeg_header;
      HTTPHeader http_header;
      uint32_t headersize = mpeg_header.GetSerializedSize () + http_header.GetSerializedSize ();

      if (m_pending_packet->GetSize () < headersize)
        {
          NS_LOG_INFO ("### Headers incomplete ");
          return;
        }

      NS_LOG_INFO ("                                   extract http_header / mpeg_header (from m_pending_packet)");
      Ptr<Packet> headerPacket = m_pending_packet->Copy ();
      headerPacket->RemoveHeader (http_header);
      headerPacket->RemoveHeader (mpeg_header);

      m_pending_message_size = headersize + mpeg_header.GetSize ();

      NS_LOG_INFO ("                                   Total size is " << m_pending_packet->GetSize () << " (m_pending_packet->GetSize ()) ");
      NS_LOG_INFO ("                                   Pending message size is " << m_pending_message_size << " (m_pending_message_size) ");
      NS_LOG_INFO ("                                   // m_pending_message_size = headersize + mpeg_header.GetSize ()");
      // NS_LOG_INFO ("Total size is " << m_pending_packet->GetSize () << " pending message is "

      if (m_pending_packet->GetSize () < m_pending_message_size)
        {
          NS_LOG_INFO ("### Packet incomplete ");
          return;
        }
      NS_LOG_INFO ("                                   Ptr<Packet> message = m_pending_packet-> CreateFragment (0, " 
                                                      << m_pending_message_size << ")");
      Ptr<Packet> message = m_pending_packet->CreateFragment (0, m_pending_message_size);
      uint32_t total_size = m_pending_packet->GetSize ();
      NS_LOG_INFO ("                                   if m_app->MessageReceived (*message) ?");
      if (m_app->MessageReceived (*message))
        {
          NS_LOG_INFO ("                                   Reminder ... ");          
          NS_LOG_INFO ("                                   // we are inside TryToPushToPlayer ... ");          
          NS_LOG_INFO ("                                   // dashClient (" << m_app->m_id << ")");
          NS_LOG_INFO ("                                   previously we asked if m_app->MessageReceived (*message) ?");          
          NS_LOG_INFO ("                                       ### Yes -> Message received by mpeg_player");          
          NS_LOG_INFO ("                                       ### Ptr<Packet> remainder = m_pending_packet->CreateFragment (" 
                                                                 << m_pending_message_size << "," << total_size - m_pending_message_size << ")");         
          NS_LOG_INFO ("                                       ###     m_pending_message_size = " << m_pending_message_size );          
          NS_LOG_INFO ("                                       ###     total_size = " << total_size );          
          NS_LOG_INFO ("                                       ### m_pending_packet is now remainder;");          
          NS_LOG_INFO ("### Message received by mpeg_player ");
          Ptr<Packet> remainder = m_pending_packet->CreateFragment (
              m_pending_message_size, total_size - m_pending_message_size);
          m_pending_packet = remainder;
        }
      else
        {
          NS_LOG_INFO ("                                       ### No -> Player Buffer is full");                    
          NS_LOG_INFO ("### Player Buffer is full ");
          return;
        }
    }

  NS_LOG_INFO ("    [" << Simulator::Now () << "] HttpParser::TryToPushToPlayer (DONE)  ");  
  NS_LOG_INFO ("    ------------------------------------------------------");
}

} // namespace ns3
