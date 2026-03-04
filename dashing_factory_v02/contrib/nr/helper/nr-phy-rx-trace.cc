/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *   Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *   Copyright (c) 2015, NYU WIRELESS, Tandon School of Engineering, New York University
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
 *   Author: Marco Miozzo <marco.miozzo@cttc.es>
 *           Nicola Baldo  <nbaldo@cttc.es>
 *
 *   Modified by: Marco Mezzavilla < mezzavilla@nyu.edu>
 *                        Sourjya Dutta <sdutta@nyu.edu>
 *                        Russell Ford <russell.ford@nyu.edu>
 *                        Menglei Zhang <menglei@nyu.edu>
 */



#include <ns3/log.h>
#include "nr-phy-rx-trace.h"
#include <ns3/simulator.h>
#include <stdio.h>
#include <ns3/string.h>
#include <regex>


namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("NrPhyRxTrace");

NS_OBJECT_ENSURE_REGISTERED (NrPhyRxTrace);

std::ofstream NrPhyRxTrace::m_rsrpSinrFile;
std::string NrPhyRxTrace::m_rsrpSinrFileName;

std::ofstream NrPhyRxTrace::m_rxPacketTraceFile;
std::string NrPhyRxTrace::m_rxPacketTraceFilename;
std::string NrPhyRxTrace::m_simTag;

std::ofstream NrPhyRxTrace::m_rxedGnbPhyCtrlMsgsFile;
std::string NrPhyRxTrace::m_rxedGnbPhyCtrlMsgsFileName;
std::ofstream NrPhyRxTrace::m_txedGnbPhyCtrlMsgsFile;
std::string NrPhyRxTrace::m_txedGnbPhyCtrlMsgsFileName;

std::ofstream NrPhyRxTrace::m_rxedUePhyCtrlMsgsFile;
std::string NrPhyRxTrace::m_rxedUePhyCtrlMsgsFileName;
std::ofstream NrPhyRxTrace::m_txedUePhyCtrlMsgsFile;
std::string NrPhyRxTrace::m_txedUePhyCtrlMsgsFileName;
std::ofstream NrPhyRxTrace::m_rxedUePhyDlDciFile;
std::string NrPhyRxTrace::m_rxedUePhyDlDciFileName;


NrPhyRxTrace::NrPhyRxTrace ()
{
}

NrPhyRxTrace::~NrPhyRxTrace ()
{
  if (m_rsrpSinrFile.is_open ())
    {
      m_rsrpSinrFile.close ();
    }

  if (m_rxPacketTraceFile.is_open ())
    {
      m_rxPacketTraceFile.close ();
    }

  if (m_rxedGnbPhyCtrlMsgsFile.is_open ())
    {
      m_rxedGnbPhyCtrlMsgsFile.close ();
    }

  if (m_txedGnbPhyCtrlMsgsFile.is_open ())
    {
      m_txedGnbPhyCtrlMsgsFile.close ();
    }

  if (m_rxedUePhyCtrlMsgsFile.is_open ())
    {
      m_rxedUePhyCtrlMsgsFile.close ();
    }

  if (m_txedUePhyCtrlMsgsFile.is_open ())
    {
      m_txedUePhyCtrlMsgsFile.close ();
    }

  if (m_rxedUePhyDlDciFile.is_open ())
    {
      m_rxedUePhyDlDciFile.close ();
    }
}

TypeId
NrPhyRxTrace::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::NrPhyRxTrace")
    .SetParent<Object> ()
    .AddConstructor<NrPhyRxTrace> ()
    .AddAttribute ("SimTag",
                   "simulation tag that will be concatenated to output file names"
                   "in order to distinguish them, for example: RxPacketTrace-${SimTag}.out. ",
                   StringValue (""),
                   MakeStringAccessor (&NrPhyRxTrace::SetSimTag),
                   MakeStringChecker ())
  ;
  return tid;
}


void
NrPhyRxTrace::SetSimTag (const std::string &simTag)
{
  m_simTag = simTag;
}

void
NrPhyRxTrace::ReportCurrentCellRsrpSinrCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                                 uint16_t cellId, uint16_t rnti, double power, double avgSinr, uint16_t bwpId)
{
  NS_LOG_INFO ("UE" << rnti << "of " << cellId << " over bwp ID " << bwpId << "->Generate RsrpSinrTrace");
  NS_UNUSED (phyStats);
  NS_UNUSED (path);
  //phyStats->ReportInterferenceTrace (imsi, sinr);
  //phyStats->ReportPowerTrace (imsi, power);

  // Called from NrHelper::EnableDlPhyTrace (void)
  // Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/ReportCurrentCellRsrpSinr",
  //                    MakeBoundCallback (&NrPhyRxTrace::ReportCurrentCellRsrpSinrCallback, m_phyStats));



  std::smatch match; int nodeId_path;
  const std::string s = path;
  std::regex rgx_srcNode (".*/NodeList/(\\w+)/*");
  if (std::regex_search(s.begin(), s.end(), match, rgx_srcNode)) nodeId_path = stoi(match[1]);
  // NS_LOG_UNCOND ("---> (regex) nodeId_path" << nodeId_path);


  if (!m_rsrpSinrFile.is_open ())
      {
        std::ostringstream oss;
        oss << "SinrTrace" << "_" << nodeId_path << "_" << m_simTag.c_str() << ".txt";
        m_rsrpSinrFileName = oss.str ();
        m_rsrpSinrFile.open (m_rsrpSinrFileName.c_str ());

        // m_rsrpSinrFile << "Time" << "\t" << "IMSI" <<
        //                             "\t" << "SINR (dB)" << std::endl;
        m_rsrpSinrFile << "timestamp(s)" << "\t" 
		                   << "cellId" <<  "\t" 
		                   << "rnti" <<  "\t" 
		                   << "nodeId" <<  "\t" 
                       << "bwpId" <<   "\t"
                       << "avgSINR" << "\t"
                       << "power" << std::endl;

        if (!m_rsrpSinrFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rsrpSinrFile << Simulator::Now ().GetSeconds () <<
                    "\t" << cellId << "\t" << rnti << "\t" << nodeId_path << "\t" << bwpId <<
                    "\t" << avgSinr << "\t" << power << std::endl;
}

void
NrPhyRxTrace::UlSinrTraceCallback     (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                       uint64_t imsi, SpectrumValue& sinr, SpectrumValue& power)
{
  //NS_LOG_UNCOND ("UE" << imsi << "->Generate UlSinrTrace");
  uint64_t tti_count = Now ().GetMicroSeconds () / 125;
  uint32_t rb_count = 1;
  FILE* log_file;
  char fname[255];
  sprintf (fname, "UE_%llu_UL_SINR_dB.txt", (long long unsigned ) imsi);
  log_file = fopen (fname, "a");
  Values::iterator it = sinr.ValuesBegin ();
  while (it != sinr.ValuesEnd ())
    {
      //fprintf(log_file, "%d\t%d\t%f\t \n", tti_count/2, rb_count, 10*log10(*it));
      fprintf (log_file, "%llu\t%llu\t%d\t%f\t \n",(long long unsigned )tti_count / 8 + 1, (long long unsigned )tti_count % 8 + 1, rb_count, 10 * log10 (*it));
      rb_count++;
      it++;
    }
  fflush (log_file);
  fclose (log_file);
  //phyStats->ReportInterferenceTrace (imsi, sinr);
  //phyStats->ReportPowerTrace (imsi, power);
}

void
NrPhyRxTrace::RxedGnbPhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                              SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                              uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  std::smatch match; int nodeId_path;
  const std::string s = path;
  std::regex rgx_srcNode (".*/NodeList/(\\w+)/*");
  if (std::regex_search(s.begin(), s.end(), match, rgx_srcNode)) nodeId_path = stoi(match[1]);

  if (!m_rxedGnbPhyCtrlMsgsFile.is_open ())
      {
        std::ostringstream oss;
        oss << "RxedGnbPhyCtrlMsgsTrace" << "_" << nodeId << "_" << m_simTag.c_str() << ".txt";
        m_rxedGnbPhyCtrlMsgsFileName = oss.str ();
        m_rxedGnbPhyCtrlMsgsFile.open (m_rxedGnbPhyCtrlMsgsFileName.c_str ());

        m_rxedGnbPhyCtrlMsgsFile << "Time" << "\t" << "Entity"  << "\t" <<
                                    "Frame" << "\t" << "SF" << "\t" << "Slot" <<
                                   "\t" << "VarTTI" << "\t" << "nodeId" << "\t" << "cellId" <<
                                    "\t" << "RNTI" << "\t" << "bwpId" <<
                                    "\t" << "MsgType" << std::endl;

        if (!m_rxedGnbPhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedGnbPhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                              "\t" << "ENB-PHY-Rxed" << "\t" << sfn.GetFrame () <<
                              "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                              "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                              "\t" << nodeId_path << "\t" << nodeId << "\t" << rnti <<
                              "\t" << static_cast<uint32_t> (bwpId) << "\t";


  if (msg->GetMessageType () == NrControlMessage::DL_CQI)
    {
      m_rxedGnbPhyCtrlMsgsFile << "DL_CQI";
    }
  else if (msg->GetMessageType () == NrControlMessage::SR)
    {
      m_rxedGnbPhyCtrlMsgsFile << "SR";
    }
  else if (msg->GetMessageType () == NrControlMessage::BSR)
    {
      m_rxedGnbPhyCtrlMsgsFile << "BSR";
    }
  else if (msg->GetMessageType () == NrControlMessage::RACH_PREAMBLE)
    {
      m_rxedGnbPhyCtrlMsgsFile << "RACH_PREAMBLE";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_HARQ)
    {
      m_rxedGnbPhyCtrlMsgsFile << "DL_HARQ";
    }
  else
    {
      m_rxedGnbPhyCtrlMsgsFile << "Other";
    }
  m_rxedGnbPhyCtrlMsgsFile << std::endl;
}

void
NrPhyRxTrace::TxedGnbPhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                              SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                              uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  std::smatch match; int nodeId_path;
  const std::string s = path;
  std::regex rgx_srcNode (".*/NodeList/(\\w+)/*");
  if (std::regex_search(s.begin(), s.end(), match, rgx_srcNode)) nodeId_path = stoi(match[1]);

  if (!m_txedGnbPhyCtrlMsgsFile.is_open ())
      {
        std::ostringstream oss;
        oss << "TxedGnbPhyCtrlMsgsTrace" << m_simTag.c_str () << ".txt";
        m_txedGnbPhyCtrlMsgsFileName = oss.str ();
        m_txedGnbPhyCtrlMsgsFile.open (m_txedGnbPhyCtrlMsgsFileName.c_str ());

        m_txedGnbPhyCtrlMsgsFile << "Time" << "\t" << "Entity" << "\t" <<
                                    "Frame" << "\t" << "SF" << "\t" << "Slot" <<
                                    "\t" << "VarTTI" << "\t" << "nodeId" << "\t" << "cellId" <<
                                    "\t" << "RNTI" << "\t" << "bwpId" <<
                                    "\t" << "MsgType" << std::endl;

        if (!m_txedGnbPhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_txedGnbPhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                              "\t" << "ENB-PHY-Txed" << "\t" << sfn.GetFrame () <<
                              "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                              "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                              "\t" << nodeId_path << "\t" << nodeId << "\t" << rnti <<
                              "\t" << static_cast<uint32_t> (bwpId) << "\t";

  if (msg->GetMessageType () == NrControlMessage::MIB)
    {
      m_txedGnbPhyCtrlMsgsFile << "MIB";
    }
  else if (msg->GetMessageType () == NrControlMessage::SIB1)
    {
      m_txedGnbPhyCtrlMsgsFile << "SIB1";
    }
  else if (msg->GetMessageType () == NrControlMessage::RAR)
    {
      m_txedGnbPhyCtrlMsgsFile << "RAR";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_DCI)
    {
      m_txedGnbPhyCtrlMsgsFile << "DL_DCI";
    }
  else if (msg->GetMessageType () == NrControlMessage::UL_DCI)
    {
      m_txedGnbPhyCtrlMsgsFile << "UL_UCI";
    }
  else
    {
      m_txedGnbPhyCtrlMsgsFile << "Other";
    }
  m_txedGnbPhyCtrlMsgsFile << std::endl;
}

void
NrPhyRxTrace::RxedUePhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                         SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                         uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  std::smatch match; int nodeId_path;
  const std::string s = path;
  std::regex rgx_srcNode (".*/NodeList/(\\w+)/*");
  if (std::regex_search(s.begin(), s.end(), match, rgx_srcNode)) nodeId_path = stoi(match[1]);

  if (!m_rxedUePhyCtrlMsgsFile.is_open ())
      {
        std::ostringstream oss;
        oss << "RxedUePhyCtrlMsgsTrace" << m_simTag.c_str () << ".txt";
        m_rxedUePhyCtrlMsgsFileName = oss.str ();
        m_rxedUePhyCtrlMsgsFile.open (m_rxedUePhyCtrlMsgsFileName.c_str ());

        m_rxedUePhyCtrlMsgsFile << "Time" << "\t" << "Entity" << "\t" <<
                                   "Frame" << "\t" << "SF" << "\t" << "Slot" <<
                                   "\t" << "VarTTI" << "\t" << "nodeId" << "\t" << "cellId" <<
                                   "\t" << "RNTI" << "\t" << "bwpId" << "\t" <<
                                   "MsgType" << std::endl;

        if (!m_rxedUePhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedUePhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                             "\t" << "UE-PHY-Rxed" << "\t" << sfn.GetFrame ()<<
                             "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                             "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                             "\t" << nodeId_path << "\t" << nodeId << "\t" << rnti <<
                             "\t" << static_cast<uint32_t> (bwpId) << "\t";

  if (msg->GetMessageType () == NrControlMessage::UL_DCI)
    {
      m_rxedUePhyCtrlMsgsFile << "UL_DCI";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_DCI)
    {
      m_rxedUePhyCtrlMsgsFile << "DL_DCI";
    }
  else if (msg->GetMessageType () == NrControlMessage::MIB)
    {
      m_rxedUePhyCtrlMsgsFile << "MIB";
    }
  else if (msg->GetMessageType () == NrControlMessage::SIB1)
    {
      m_rxedUePhyCtrlMsgsFile << "SIB1";
    }
  else if (msg->GetMessageType () == NrControlMessage::RAR)
    {
      m_rxedUePhyCtrlMsgsFile << "RAR";
    }
  else
    {
      m_rxedUePhyCtrlMsgsFile << "Other";
    }
  m_rxedUePhyCtrlMsgsFile << std::endl;
}

void
NrPhyRxTrace::TxedUePhyCtrlMsgsCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                             SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                             uint8_t bwpId, Ptr<const NrControlMessage> msg)
{
  std::smatch match; int nodeId_path;
  const std::string s = path;
  std::regex rgx_srcNode (".*/NodeList/(\\w+)/*");
  if (std::regex_search(s.begin(), s.end(), match, rgx_srcNode)) nodeId_path = stoi(match[1]);

  if (!m_txedUePhyCtrlMsgsFile.is_open ())
      {
        std::ostringstream oss;
        oss << "TxedUePhyCtrlMsgsTrace" << m_simTag.c_str () << ".txt";
        m_txedUePhyCtrlMsgsFileName = oss.str ();
        m_txedUePhyCtrlMsgsFile.open (m_txedUePhyCtrlMsgsFileName.c_str ());

        m_txedUePhyCtrlMsgsFile << "Time" << "\t" << "Entity" << "\t" <<
                                   "Frame" << "\t" << "SF" << "\t" << "Slot" <<
                                   "\t" << "VarTTI" << "\t" << "nodeId" << "\t" << "cellId" <<
                                   "\t" << "RNTI" << "\t" << "bwpId" <<
                                   "\t" << "MsgType" << std::endl;

        if (!m_txedUePhyCtrlMsgsFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_txedUePhyCtrlMsgsFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                     "\t" << "UE-PHY-Txed" << "\t" << sfn.GetFrame () <<
                     "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                     "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                     "\t" << nodeId_path << "\t" << nodeId << "\t" << rnti <<
                     "\t" << static_cast<uint32_t> (bwpId) << "\t";

  if (msg->GetMessageType () == NrControlMessage::RACH_PREAMBLE)
    {
      m_txedUePhyCtrlMsgsFile << "RACH_PREAMBLE";
    }
  else if (msg->GetMessageType () == NrControlMessage::SR)
    {
      m_txedUePhyCtrlMsgsFile << "SR";
    }
  else if (msg->GetMessageType () == NrControlMessage::BSR)
    {
      m_txedUePhyCtrlMsgsFile << "BSR";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_CQI)
    {
      m_txedUePhyCtrlMsgsFile << "DL_CQI";
    }
  else if (msg->GetMessageType () == NrControlMessage::DL_HARQ)
    {
      m_txedUePhyCtrlMsgsFile << "DL_HARQ";
    }
  else
    {
      m_txedUePhyCtrlMsgsFile << "Other";
    }
  m_txedUePhyCtrlMsgsFile << std::endl;
}

void
NrPhyRxTrace::RxedUePhyDlDciCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                          SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                          uint8_t bwpId, uint8_t harqId, uint32_t k1Delay)
{
  std::smatch match; int nodeId_path;
  const std::string s = path;
  std::regex rgx_srcNode (".*/NodeList/(\\w+)/*");
  if (std::regex_search(s.begin(), s.end(), match, rgx_srcNode)) nodeId_path = stoi(match[1]);

  //NS_LOG_UNCOND ("(RxedUePhyDlDciCallback) path_nodeId :" << nodeId_path << ", cellId: " << uint32_t(nodeId) << ", RNTI: " << uint32_t(rnti));

  if (!m_rxedUePhyDlDciFile.is_open ())
      {
        std::ostringstream oss;
        oss << "RxedUePhyDlDciTrace" << m_simTag.c_str () << ".txt";
        //      RxedUePhyDlDciTrace
        m_rxedUePhyDlDciFileName = oss.str ();
        m_rxedUePhyDlDciFile.open (m_rxedUePhyDlDciFileName.c_str ());

        m_rxedUePhyDlDciFile << "Time" << "\t" << "Entity"  << "\t" << "Frame" <<
                                "\t" << "SF" << "\t" << "Slot" << "\t" <<
                                "nodeId" << "\t" << "cellId" << "\t" << "RNTI" << "\t" << "bwpId" <<
                                "\t" << "Harq_ID" << "\t" << "K1_Delay" << std::endl;

        if (!m_rxedUePhyDlDciFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedUePhyDlDciFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                          "\t" << "DL-DCI-Rxed" << "\t" << sfn.GetFrame () <<
                          "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                          "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                          "\t" << nodeId_path << "\t" << nodeId << "\t" << rnti <<
                          static_cast<uint32_t> (bwpId) << "\t" <<
                          static_cast<uint32_t> (harqId) << "\t" <<
                          k1Delay << std::endl;
}

// Ue PHY DL HARQ Feedback Traces. 
void
NrPhyRxTrace::TxedUePhyHarqFeedbackCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                             SfnSf sfn, uint16_t nodeId, uint16_t rnti,
                                             uint8_t bwpId, uint8_t harqId, uint32_t k1Delay)
{
  std::smatch match; int nodeId_path;
  const std::string s = path;
  std::regex rgx_srcNode (".*/NodeList/(\\w+)/*");
  if (std::regex_search(s.begin(), s.end(), match, rgx_srcNode)) nodeId_path = stoi(match[1]);

  //NS_LOG_UNCOND ("(TxedUePhyHarqFeedbackCallback) path_nodeId: " << nodeId_path << ", cellId: " << uint32_t(nodeId) << ", RNTI: " << uint32_t(rnti));

  if (!m_rxedUePhyDlDciFile.is_open ())
      {
        std::ostringstream oss;
        oss << "RxedUePhyDlDciTrace" << m_simTag.c_str () << ".txt";
        //      RxedUePhyDlDciTrace
        m_rxedUePhyDlDciFileName = oss.str ();
        m_rxedUePhyDlDciFile.open (m_rxedUePhyDlDciFileName.c_str ());

        m_rxedUePhyDlDciFile << "Time" << "\t" << "Entity"  << "\t" << "Frame" <<
                                "\t" << "SF" << "\t" << "Slot" << "\t" <<
                                "nodeId" << "\t" << "cellId" << "\t" << "RNTI" << "\t" << "bwpId" <<
                                "\t" << "Harq_ID" << "\t" << "K1_Delay" << std::endl;

        if (!m_rxedUePhyDlDciFile.is_open ())
          {
            NS_FATAL_ERROR ("Could not open tracefile");
          }
      }

  m_rxedUePhyDlDciFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                          "\t" << "HARQ-FD-Txed" << "\t" << sfn.GetFrame () <<
                          "\t" << static_cast<uint32_t> (sfn.GetSubframe ()) <<
                          "\t" << static_cast<uint32_t> (sfn.GetSlot ()) <<
                          "\t" << nodeId_path << "\t" << nodeId << "\t" << rnti <<
                          static_cast<uint32_t> (bwpId) << "\t" <<
                          static_cast<uint32_t> (harqId) << "\t" <<
                          k1Delay << std::endl;
}

void
NrPhyRxTrace::ReportInterferenceTrace (uint64_t imsi, SpectrumValue& sinr)
{
  uint64_t tti_count = Now ().GetMicroSeconds () / 125;
  uint32_t rb_count = 1;
  FILE* log_file;
  char fname[255];
  sprintf (fname, "UE_%llu_SINR_dB.txt", (long long unsigned ) imsi);
  log_file = fopen (fname, "a");
  Values::iterator it = sinr.ValuesBegin ();
  while (it != sinr.ValuesEnd ())
    {
      //fprintf(log_file, "%d\t%d\t%f\t \n", tti_count/2, rb_count, 10*log10(*it));
      fprintf (log_file, "%llu\t%llu\t%d\t%f\t \n",(long long unsigned) tti_count / 8 + 1, (long long unsigned) tti_count % 8 + 1, rb_count, 10 * log10 (*it));
      rb_count++;
      it++;
    }
  fflush (log_file);
  fclose (log_file);
}

void
NrPhyRxTrace::ReportPowerTrace (uint64_t imsi, SpectrumValue& power)
{

  uint32_t tti_count = Now ().GetMicroSeconds () / 125;
  uint32_t rb_count = 1;
  FILE* log_file;
  char fname[255];
  printf (fname, "UE_%llu_ReceivedPower_dB.txt", (long long unsigned) imsi);
  log_file = fopen (fname, "a");
  Values::iterator it = power.ValuesBegin ();
  while (it != power.ValuesEnd ())
    {
      fprintf (log_file, "%llu\t%llu\t%d\t%f\t \n",(long long unsigned) tti_count / 8 + 1,(long long unsigned) tti_count % 8 + 1, rb_count, 10 * log10 (*it));
      rb_count++;
      it++;
    }
  fflush (log_file);
  fclose (log_file);
}

void
NrPhyRxTrace::ReportPacketCountUeCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                               UePhyPacketCountParameter param)
{
  phyStats->ReportPacketCountUe (param);
}
void
NrPhyRxTrace::ReportPacketCountEnbCallback (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                                GnbPhyPacketCountParameter param)
{
  phyStats->ReportPacketCountEnb (param);
}

void
NrPhyRxTrace::ReportDownLinkTBSize (Ptr<NrPhyRxTrace> phyStats, std::string path,
                                        uint64_t imsi, uint64_t tbSize)
{
  phyStats->ReportDLTbSize (imsi, tbSize);
}



void
NrPhyRxTrace::ReportPacketCountUe (UePhyPacketCountParameter param)
{
  FILE* log_file;
  char fname[255];
  sprintf (fname,"UE_%llu_Packet_Trace.txt", (long long unsigned) param.m_imsi);
  log_file = fopen (fname, "a");
  if (param.m_isTx)
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, param.m_noBytes, 0);
    }
  else
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, 0, param.m_noBytes);
    }

  fflush (log_file);
  fclose (log_file);

}

void
NrPhyRxTrace::ReportPacketCountEnb (GnbPhyPacketCountParameter param)
{
  FILE* log_file;
  char fname[255];
  sprintf (fname,"BS_%llu_Packet_Trace.txt",(long long unsigned) param.m_cellId);
  log_file = fopen (fname, "a");
  if (param.m_isTx)
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, param.m_noBytes, 0);
    }
  else
    {
      fprintf (log_file, "%d\t%d\t%d\n", param.m_subframeno, 0, param.m_noBytes);
    }

  fflush (log_file);
  fclose (log_file);
}

void
NrPhyRxTrace::ReportDLTbSize (uint64_t imsi, uint64_t tbSize)
{
  FILE* log_file;
  char fname[255];
  sprintf (fname,"UE_%llu_Tb_Size.txt", (long long unsigned) imsi);
  log_file = fopen (fname, "a");

  fprintf (log_file, "%llu \t %llu\n", (long long unsigned )Now ().GetMicroSeconds (), (long long unsigned )tbSize);
  fprintf (log_file, "%lld \t %llu \n",(long long int) Now ().GetMicroSeconds (), (long long unsigned) tbSize);
  fflush (log_file);
  fclose (log_file);
}

void
NrPhyRxTrace::RxPacketTraceUeCallback (Ptr<NrPhyRxTrace> phyStats, std::string path, RxPacketTraceParams params)
{
  // NrHelper::EnableDlPhyTrace (void)
  // Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/ReportCurrentCellRsrpSinr",
  //                    MakeBoundCallback (&NrPhyRxTrace::ReportCurrentCellRsrpSinrCallback, m_phyStats));
  // Config::Connect ("/NodeList/*/DeviceList/*/ComponentCarrierMapUe/*/NrUePhy/SpectrumPhy/RxPacketTraceUe",
  //                    MakeBoundCallback (&NrPhyRxTrace::RxPacketTraceUeCallback, m_phyStats));
 

  // NS_LOG_UNCOND ("---> NrPhyRxTrace::RxPacketTraceUeCallback::path " << path);
  // example of path : /NodeList/24/DeviceList/0/ComponentCarrierMapUe/0/NrUePhy/SpectrumPhy/RxPacketTraceUe   
  //                    path.substr(9, 12); --> /24/DeviceLi
  //                    path.substr(10, 2); --> 24


  std::smatch match; int nodeId;
  const std::string s = path;
  std::regex rgx_srcNode (".*/NodeList/(\\w+)/*");
  if (std::regex_search(s.begin(), s.end(), match, rgx_srcNode)) nodeId = stoi(match[1]);
  // NS_LOG_UNCOND ("---> (regex) nodeId" << nodeId);


  if (!m_rxPacketTraceFile.is_open ())
    {
      std::ostringstream oss;
      oss << "RxPacketTrace_" << m_simTag.c_str() << ".txt";
      m_rxPacketTraceFilename = oss.str ();
      m_rxPacketTraceFile.open (m_rxPacketTraceFilename.c_str ());

      // NS_LOG_UNCOND ("---> output file (nr-phy-rx-trace.cc)" << oss.str ());
      
      m_rxPacketTraceFile << "Time" << "\t" << "direction" << "\t" <<
                             "frame" << "\t" << "subF" << "\t" << "slot" <<
                             "\t" << "1stSym" << "\t" << "nSymbol" <<
                             "\t" << "cellId" << "\t" << "rnti" <<
                             "\t" << "nodeId" <<                              
                             "\t" << "tbSize" << "\t" << "mcs" <<
                             "\t" << "rv" << "\t" << "SINR(dB)" <<
                             "\t" << "corrupt" << "\t" << "TBler" <<
                             "\t" << "bwpId" << std::endl;

      if (!m_rxPacketTraceFile.is_open ())
        {
          NS_FATAL_ERROR ("Could not open tracefile");
        }
    }

      
  m_rxPacketTraceFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                         "\t" << "DL" <<
                         "\t" << params.m_frameNum <<
                         "\t" << (unsigned)params.m_subframeNum <<
                         "\t" << (unsigned)params.m_slotNum <<
                         "\t" << (unsigned)params.m_symStart <<
                         "\t" << (unsigned)params.m_numSym <<
                         "\t" << params.m_cellId <<
                         "\t" << params.m_rnti <<
                         "\t" << nodeId <<
                         "\t" << params.m_tbSize <<
                         "\t" << (unsigned)params.m_mcs <<
                         "\t" << (unsigned)params.m_rv <<
                         "\t" << 10 * log10 (params.m_sinr) <<
                         "\t" << params.m_corrupt <<
                         "\t" << params.m_tbler <<
                         "\t" << (unsigned)params.m_bwpId << std::endl;

  if (params.m_corrupt)
    {
      NS_LOG_DEBUG ("DL TB error\t" << params.m_frameNum <<
                    "\t" << (unsigned)params.m_subframeNum <<
                    "\t" << (unsigned)params.m_slotNum <<
                    "\t" << (unsigned)params.m_symStart <<
                    "\t" << (unsigned)params.m_numSym <<
                    "\t" << params.m_rnti <<
                    "\t" << params.m_tbSize <<
                    "\t" << (unsigned)params.m_mcs <<
                    "\t" << (unsigned)params.m_rv <<
                    "\t" << params.m_sinr <<
                    "\t" << params.m_tbler <<
                    "\t" << params.m_corrupt <<
                    "\t" << (unsigned)params.m_bwpId);
    }
}
void
NrPhyRxTrace::RxPacketTraceEnbCallback (Ptr<NrPhyRxTrace> phyStats, std::string path, RxPacketTraceParams params)
{
    // Called via --> NrHelper::EnableUlPhyTrace (void)
    //                Config::Connect ("/NodeList/*/DeviceList/*/BandwidthPartMap/*/NrGnbPhy/SpectrumPhy/RxPacketTraceEnb",
    //                                   MakeBoundCallback (&NrPhyRxTrace::RxPacketTraceEnbCallback, m_phyStats));


  
  std::smatch match; int nodeId;
  const std::string s = path;
  std::regex rgx_srcNode (".*/NodeList/(\\w+)/*");
  if (std::regex_search(s.begin(), s.end(), match, rgx_srcNode)) nodeId = stoi(match[1]);
  // NS_LOG_UNCOND ("---> (regex) nodeId" << nodeId);

  if (!m_rxPacketTraceFile.is_open ())
    {
      std::ostringstream oss;
      oss << "RxPacketTrace_" << m_simTag.c_str() << ".txt";
      m_rxPacketTraceFilename = oss.str ();
      m_rxPacketTraceFile.open (m_rxPacketTraceFilename.c_str ());

      m_rxPacketTraceFile << "Time" << "\t" << "direction" << "\t" <<
                             "frame" << "\t" << "subF" << "\t" << "slot" <<
                             "\t" << "1stSym" << "\t" << "nSymbol" <<
                             "\t" << "cellId" << "\t" << "rnti" <<
                             "\t" << "nodeId" << 
                             "\t" << "tbSize" << "\t" << "mcs" <<
                             "\t" << "rv" << "\t" << "SINR(dB)" <<
                             "\t" << "corrupt" << "\t" << "TBler" <<
                             "\t" << "bwpId" << std::endl;

      if (!m_rxPacketTraceFile.is_open ())
        {
          NS_FATAL_ERROR ("Could not open tracefile");
        }
    }
  m_rxPacketTraceFile << Simulator::Now ().GetNanoSeconds () / (double) 1e9 <<
                         "\t" << "UL" <<
                         "\t" << params.m_frameNum <<
                         "\t" << (unsigned)params.m_subframeNum <<
                         "\t" << (unsigned)params.m_slotNum <<
                         "\t" << (unsigned)params.m_symStart <<
                         "\t" << (unsigned)params.m_numSym <<
                         "\t" << params.m_cellId <<
                         "\t" << params.m_rnti <<
                         "\t" << nodeId <<
                         "\t" << params.m_tbSize <<
                         "\t" << (unsigned)params.m_mcs <<
                         "\t" << (unsigned)params.m_rv <<
                         "\t" << 10 * log10 (params.m_sinr) <<
                         "\t" << params.m_corrupt <<
                         "\t" << params.m_tbler <<
                         "\t" << params.m_bwpId << std::endl;

  if (params.m_corrupt)
    {
      NS_LOG_DEBUG ("UL TB error\t" << params.m_frameNum <<
                    "\t" << (unsigned)params.m_subframeNum <<
                    "\t" << (unsigned)params.m_slotNum <<
                    "\t" << (unsigned)params.m_symStart <<
                    "\t" << (unsigned)params.m_numSym <<
                    "\t" << params.m_rnti <<
                    "\t" << params.m_tbSize <<
                    "\t" << (unsigned)params.m_mcs <<
                    "\t" << (unsigned)params.m_rv <<
                    "\t" << params.m_sinr <<
                    "\t" << params.m_tbler <<
                    "\t" << params.m_corrupt <<
                    "\t" << params.m_sinrMin <<
                    "\t" << params.m_bwpId);
    }
}

} /* namespace ns3 */
