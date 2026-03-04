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

#include <ns3/log.h>
#include "nr-mac-rx-trace.h"
#include <ns3/simulator.h>
#include <stdio.h>
#include <fstream>
#include <ns3/string.h>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrMacRxTrace");

NS_OBJECT_ENSURE_REGISTERED (NrMacRxTrace);

std::ofstream NrMacRxTrace::m_rxedGnbMacCtrlMsgsFile;
std::string NrMacRxTrace::m_rxedGnbMacCtrlMsgsFileName;
std::ofstream NrMacRxTrace::m_txedGnbMacCtrlMsgsFile;
std::string NrMacRxTrace::m_txedGnbMacCtrlMsgsFileName;

std::ofstream NrMacRxTrace::m_rxedUeMacCtrlMsgsFile;
std::string NrMacRxTrace::m_rxedUeMacCtrlMsgsFileName;
std::ofstream NrMacRxTrace::m_txedUeMacCtrlMsgsFile;
std::string NrMacRxTrace::m_txedUeMacCtrlMsgsFileName;

NrMacRxTrace::NrMacRxTrace ()
{          
     // SetSimTag (7777);          
}


NrMacRxTrace::~NrMacRxTrace ()
{
  if (m_rxedGnbMacCtrlMsgsFile.is_open ())
    {
      m_rxedGnbMacCtrlMsgsFile.close ();
    }

  if (m_txedGnbMacCtrlMsgsFile.is_open ())
    {
      m_txedGnbMacCtrlMsgsFile.close ();
    }

  if (m_rxedUeMacCtrlMsgsFile.is_open ())
    {
      m_rxedUeMacCtrlMsgsFile.close ();
    }

  if (m_txedUeMacCtrlMsgsFile.is_open ())
    {
      m_txedUeMacCtrlMsgsFile.close ();
    }
}

TypeId
NrMacRxTrace::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrMacRxTrace")
    .SetParent<Object> ()
    .AddConstructor<NrMacRxTrace> ()
    // .AddAttribute ("SimTag",
    //                "simulation tag that will be concatenated to output file names"
    //                "in order to distinguish them, for example: RxPacketTrace-${SimTag}.out. ",
    //                StringValue (""),
    //                MakeStringAccessor (&NrMacRxTrace::SetSimTag),
    //                MakeStringChecker ())     //   
    // .AddAttribute ("SimTag", "tag ta3ak", UintegerValue (7777),                                      
    //                   MakeUintegerAccessor (&NrMacRxTrace::SetSimTag,
    //                                         &NrMacRxTrace::GetSimTag),                                                                                                 
    //                   MakeUintegerChecker<uint32_t> ())                             
  ;
  return tid; 
}

// void
// // NrMacRxTrace::SetSimTag (const int32_t & simTag)
// NrMacRxTrace::SetSimTag (uint32_t simTag)
// {
//   m_simTag = simTag;
// }

// uint32_t
// NrMacRxTrace::GetSimTag () const 
// {
//   return m_simTag;
// }


// void
// NrMacRxTrace::SetSimTag (const std::string &simTag)
// {
//   m_simTag = simTag;
// }

void
NrMacRxTrace::RxedGnbMacCtrlMsgsCallback (Ptr<NrMacRxTrace> macStats, std::string path,
                                          SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                          uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  // NS_LOG_UNCOND ("----> path :" << path);	
  if (!m_rxedGnbMacCtrlMsgsFile.is_open ())
      {
        //std::ostringstream oss;
        //oss << "RxedGnbMacCtrlMsgsTrace" << m_simTag.c_str () << ".txt";
        //m_rxedGnbMacCtrlMsgsFileName = oss.str ();

        // NS_LOG_UNCOND ("inside RxedGnbMacCtrlMsgsCallback, macStats->GetSimTag() =" << macStats->m_simTag);                              
        // m_rxedGnbMacCtrlMsgsFileName = "RxedGnbMacCtrlMsgsTrace_" + std::to_string(macStats->GetSimTag()) + ".txt";
        // NS_LOG_UNCOND ("m_rxedGnbMacCtrlMsgsFileName =" << m_rxedGnbMacCtrlMsgsFileName);
        m_rxedGnbMacCtrlMsgsFileName = "RxedGnbMacCtrlMsgsTrace.txt";
        m_rxedGnbMacCtrlMsgsFile.open (m_rxedGnbMacCtrlMsgsFileName.c_str ());
        m_rxedGnbMacCtrlMsgsFile << "Time" << "\t" << "Entity" << "\t" <<
                                    "Frame" << "\t" << "SF" << "\t" << "Slot" <<
                                    "\t" << "VarTTI" << "\t" << "nodeId" <<
                                    "\t" << "RNTI" << "\t" << "bwpId"<<
                                    "\t" << "MsgType" << std::endl;

        if (!m_rxedGnbMacCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedGnbMacCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                              "\t" << "ENB MAC Rxed"  << "\t" << sfn.GetFrame () <<
                              "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                              "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                              "\t" << nodeId << "\t" << rnti <<
                              "\t" << static_cast<uint32_t> (bwpId) << "\t";

  if (msg->GetMessageType () == NrControlMessage::SR)
    {
      m_rxedGnbMacCtrlMsgsFile << "SR";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_CQI)
    {
      m_rxedGnbMacCtrlMsgsFile << "DL_CQI";
    }
  else if (msg->GetMessageType () == NrControlMessage::BSR)
    {
      m_rxedGnbMacCtrlMsgsFile << "BSR";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_HARQ)
    {
      m_rxedGnbMacCtrlMsgsFile << "DL_HARQ";
    }
  else if (msg->GetMessageType () == NrControlMessage::RACH_PREAMBLE)
    {
      m_rxedGnbMacCtrlMsgsFile << "RACH_PREAMBLE";
    }
  else
    {
      m_rxedGnbMacCtrlMsgsFile << "Other";
    }
  m_rxedGnbMacCtrlMsgsFile << std::endl;
}

void
NrMacRxTrace::TxedGnbMacCtrlMsgsCallback (    Ptr<NrMacRxTrace> macStats, std::string path,
                                              SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                              uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  if (!m_txedGnbMacCtrlMsgsFile.is_open ())
      {

        // std::ostringstream oss;
        // oss << "TxedGnbMacCtrlMsgsTrace" << m_simTag.c_str () << ".txt";
        // m_txedGnbMacCtrlMsgsFileName = oss.str ();

	      // m_txedGnbMacCtrlMsgsFileName = "TxedGnbMacCtrlMsgsTrace_"+ std::to_string(m_simTag) + ".txt";
        // NS_LOG_UNCOND ("inside TxedGnbMacCtrlMsgsCallback, macStats->GetSimTag() =" << macStats->m_simTag());                
	      // m_txedGnbMacCtrlMsgsFileName = "TxedGnbMacCtrlMsgsTrace_"+ std::to_string(macStats->GetSimTag()) + ".txt";
        // NS_LOG_UNCOND ("m_txedGnbMacCtrlMsgsFileName =" << m_txedGnbMacCtrlMsgsFileName);        
        m_txedGnbMacCtrlMsgsFileName = "TxedGnbMacCtrlMsgsTrace.txt";
        m_txedGnbMacCtrlMsgsFile.open (m_txedGnbMacCtrlMsgsFileName.c_str ());
        m_txedGnbMacCtrlMsgsFile << "Time" << "\t" << "Entity" << "\t" <<
                                    "Frame" << "\t" << "SF" << "\t" << "Slot" <<
                                    "\t" << "VarTTI" "\t" << "nodeId" <<
                                    "\t" << "RNTI" << "\t" << "bwpId" <<
                                    "\t" << "MsgType" << std::endl;

        if (!m_txedGnbMacCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_txedGnbMacCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                              "\t" << "ENB MAC Txed"  << "\t" << sfn.GetFrame () <<
                              "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                              "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                              "\t" << nodeId << "\t" << rnti <<
                              "\t" << static_cast<uint32_t> (bwpId) << "\t";

  if (msg->GetMessageType () == NrControlMessage::RAR)
    {
      m_txedGnbMacCtrlMsgsFile << "RAR";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_CQI)
    {
      m_txedGnbMacCtrlMsgsFile << "DL_CQI";
    }
  else
    {
      m_txedGnbMacCtrlMsgsFile << "Other";
    }

  m_txedGnbMacCtrlMsgsFile << std::endl;
}

void
NrMacRxTrace::RxedUeMacCtrlMsgsCallback (Ptr<NrMacRxTrace> macStats, std::string path,
                                             SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                             uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  if (!m_rxedUeMacCtrlMsgsFile.is_open ())
      {
        //std::ostringstream oss;
        //oss << "RxedUeMacCtrlMsgsTrace" << m_simTag.c_str () << ".txt";
        //m_rxedUeMacCtrlMsgsFileName = oss.str ();
      
        // NS_LOG_UNCOND ("inside RxedUeMacCtrlMsgsCallback, macStats->GetSimTag() =" << macStats->m_simTag());                      
        // // m_rxedUeMacCtrlMsgsFileName = "RxedUeMacCtrlMsgsTrace_" + std::to_string(m_simTag) + ".txt";
        // m_rxedUeMacCtrlMsgsFileName = "RxedUeMacCtrlMsgsTrace_" + std::to_string(macStats->GetSimTag()) + ".txt";
        // NS_LOG_UNCOND ("m_rxedUeMacCtrlMsgsFileName =" << m_rxedUeMacCtrlMsgsFileName);        
        m_rxedUeMacCtrlMsgsFileName = "RxedUeMacCtrlMsgsTrace.txt";
        m_rxedUeMacCtrlMsgsFile.open (m_rxedUeMacCtrlMsgsFileName.c_str ());
        m_rxedUeMacCtrlMsgsFile << "Time" << "\t" << "Entity" << "\t" <<
                                   "Frame" << "\t" << "SF" << "\t" << "Slot" <<
                                   "\t" << "VarTTI" << "\t" << "nodeId" <<
                                   "\t" << "RNTI" << "\t" << "bwpId" <<
                                   "\t" << "MsgType" << std::endl;

        if (!m_rxedUeMacCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedUeMacCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                             "\t" << "UE  MAC Rxed" << "\t" << sfn.GetFrame () <<
                             "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                             "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                             "\t" << nodeId << "\t" << rnti <<
                             "\t" << static_cast<uint32_t> (bwpId) << "\t";

  if (msg->GetMessageType () == NrControlMessage::UL_DCI)
    {
      m_rxedUeMacCtrlMsgsFile << "UL_DCI";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_DCI)
    {
      m_rxedUeMacCtrlMsgsFile << "DL_DCI";
    }
  else if (msg->GetMessageType () == NrControlMessage::RAR)
    {
      m_rxedUeMacCtrlMsgsFile << "RAR";
    }
  else
    {
      m_rxedUeMacCtrlMsgsFile << "Other";
    }
  m_rxedUeMacCtrlMsgsFile << std::endl;
}

void
NrMacRxTrace::TxedUeMacCtrlMsgsCallback (Ptr<NrMacRxTrace> macStats, std::string path,
                                             SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                             uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  if (!m_txedUeMacCtrlMsgsFile.is_open ())
      {
        //std::ostringstream oss;
        //oss << "TxedUeMacCtrlMsgsTrace" << m_simTag.c_str () << ".txt";
        //m_txedUeMacCtrlMsgsFileName = oss.str ();      

        // NS_LOG_UNCOND ("inside TxedUeMacCtrlMsgsCallback, macStats->GetSimTag() =" << macStats->m_simTag));                      
	      // // m_txedUeMacCtrlMsgsFileName = "TxedUeMacCtrlMsgsTrace_"+ std::to_string(m_simTag) + ".txt";
	      // m_txedUeMacCtrlMsgsFileName = "TxedUeMacCtrlMsgsTrace_"+ std::to_string(macStats->GetSimTag()) + ".txt";
        m_txedUeMacCtrlMsgsFileName = "TxedUeMacCtrlMsgsTrace.txt";
        // NS_LOG_UNCOND ("m_txedUeMacCtrlMsgsFileName =" << m_txedUeMacCtrlMsgsFileName);        
        m_txedUeMacCtrlMsgsFile.open (m_txedUeMacCtrlMsgsFileName.c_str ());
        m_txedUeMacCtrlMsgsFile << "Time" << "\t" << "Entity" << "\t" <<
                                   "Frame" << "\t" << "SF" <<
                                   "\t" << "Slot" << "\t" << "VarTTI" <<
                                   "\t" << "nodeId" << "\t" << "RNTI" <<
                                   "\t" << "bwpId" <<
                                   "\t" << "MsgType" << std::endl;

        if (!m_txedUeMacCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_txedUeMacCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                             "\t" << "UE  MAC Txed" << "\t" << sfn.GetFrame () <<
                             "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                             "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                             "\t" << nodeId << "\t" << rnti <<
                             "\t" << static_cast<uint32_t> (bwpId) << "\t";

  if (msg->GetMessageType () == NrControlMessage::BSR)
    {
      m_txedUeMacCtrlMsgsFile << "BSR";
    }
  else if (msg->GetMessageType () == NrControlMessage::SR)
    {
      m_txedUeMacCtrlMsgsFile << "SR";
    }
  else if (msg->GetMessageType () == NrControlMessage::RACH_PREAMBLE)
    {
      m_txedUeMacCtrlMsgsFile << "RACH_PREAMBLE";
    }
  else
    {
      m_txedUeMacCtrlMsgsFile << "Other";
    }
  m_txedUeMacCtrlMsgsFile << std::endl;
}

} /* namespace ns3 */
