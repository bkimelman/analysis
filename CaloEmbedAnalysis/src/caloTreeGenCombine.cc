#include "caloTreeGenCombine.h"

#include <calobase/TowerInfo.h>  // for TowerInfo
#include <calobase/TowerInfoContainer.h>
#include <calobase/TowerInfoContainerv1.h>
#include <calobase/TowerInfov1.h>

#include <cdbobjects/CDBTTree.h>  // for CDBTTree

#include <ffamodules/CDBInterface.h>

#include <ffaobjects/EventHeader.h>

#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/SubsysReco.h>  // for SubsysReco

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>    // for PHIODataNode
#include <phool/PHNode.h>          // for PHNode
#include <phool/PHNodeIterator.h>  // for PHNodeIterator
#include <phool/PHObject.h>        // for PHObject
#include <phool/getClass.h>
#include <phool/phool.h>
#include <phool/recoConsts.h>

#include <cstdlib>    // for exit
#include <exception>  // for exception
#include <iostream>   // for operator<<, basic_ostream
#include <stdexcept>  // for runtime_error

//____________________________________________________________________________..
CaloTreeGenCombine::CaloTreeGenCombine(const std::string &name)
  : SubsysReco(name)
  , m_dettype(CaloTreeGenCombine::HCALOUT)
  , m_detector("HCALOUT")
  , m_DETECTOR(TowerInfoContainer::HCAL)
  , m_fieldname("")
  , m_runNumber(-1)
{
  std::cout << "CaloTreeGenCombine::CaloTreeGenCombine(const std::string &name) Calling ctor" << std::endl;
}

//____________________________________________________________________________..
CaloTreeGenCombine::~CaloTreeGenCombine()
{
  std::cout << "CaloTreeGenCombine::~CaloTreeGenCombine() Calling dtor" << std::endl;
}

//____________________________________________________________________________..
int CaloTreeGenCombine::InitRun(PHCompositeNode *topNode)
{

  Fun4AllServer *se = Fun4AllServer::instance();

  PHCompositeNode *dataTopNode = se->topNode();

  EventHeader *evtHeader = findNode::getClass<EventHeader>(dataTopNode, "EventHeader");

  if (evtHeader)
  {
    m_runNumber = evtHeader->get_RunNumber();
  }
  else
  {
    m_runNumber = -1;
  }
  std::cout << "at run" << m_runNumber << std::endl;
  if (m_dettype == CaloTreeGenCombine::CEMC)
  {
    m_detector = "CEMC";
    m_DETECTOR = TowerInfoContainer::EMCAL;
  }
  else if (m_dettype == CaloTreeGenCombine::HCALIN)
  {
    m_detector = "HCALIN";
    m_DETECTOR = TowerInfoContainer::HCAL;
  }
  else if (m_dettype == CaloTreeGenCombine::HCALOUT)
  {
    m_detector = "HCALOUT";
    m_DETECTOR = TowerInfoContainer::HCAL;
  }
  else if (m_dettype == CaloTreeGenCombine::EPD)
  {
    m_detector = "EPD";
    m_DETECTOR = TowerInfoContainer::SEPD;
  }


  /*

  PHNodeIterator dataIter(dataTopNode);
  PHNodeIterator simIter(topNode);


  // Looking for the DST node
  PHCompositeNode *dstNode;
  dstNode = dynamic_cast<PHCompositeNode *>(dataIter.findFirst("PHCompositeNode","DST"));
  if (!dstNode)
  {
    std::cout << Name() << "::" << m_detector << "::" << __PRETTY_FUNCTION__
              << "DST Node missing, doing nothing." << std::endl;
    exit(1);
  }
  PHCompositeNode *dstNodeSim;
  dstNodeSim = dynamic_cast<PHCompositeNode *>(simIter.findFirst("PHCompositeNode","DST"));
  if (!dstNodeSim)
  {
    std::cout << Name() << "::" << m_detector << "::" << __PRETTY_FUNCTION__
              << "DSTsim Node missing, doing nothing." << std::endl;
    exit(1);
  }
  */
  try
  {
    CreateNodeTree(topNode);
  }
  catch (std::exception &e)
  {
    std::cout << e.what() << std::endl;
    return Fun4AllReturnCodes::ABORTRUN;
  }
  topNode->print();
  dataTopNode->print();
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int CaloTreeGenCombine::process_event(PHCompositeNode * /*topNode*/)
{
  unsigned int ntowers = _data_towers->size();
  for (unsigned int channel = 0; channel < ntowers; channel++)
  {
    //unsigned int key = _data_towers->encode_key(channel);
    //unsigned int sim_key = _sim_towers->encode_key(channel);
    TowerInfo *caloinfo_data = _data_towers->get_tower_at_channel(channel);
    TowerInfo *caloinfo_sim = _sim_towers->get_tower_at_channel(channel);
    float data_amplitude = caloinfo_data->get_energy();
    float sim_amplitude = caloinfo_sim->get_energy();
    short data_time = caloinfo_data->get_time();
    _data_towers->get_tower_at_channel(channel)->set_energy(data_amplitude + sim_amplitude);
    _data_towers->get_tower_at_channel(channel)->set_time(data_time);
  }

  return Fun4AllReturnCodes::EVENT_OK;
}

void CaloTreeGenCombine::CreateNodeTree(PHCompositeNode *topNode)
{
  std::cout << "creating node" << std::endl;

  Fun4AllServer *se = Fun4AllServer::instance();
  PHCompositeNode *dataTopNode = se->topNode();

  PHNodeIterator dataIter(dataTopNode);
  PHNodeIterator simIter(topNode);

  PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(dataIter.findFirst(
      "PHCompositeNode", "DST"));
  if (!dstNode)
  {
    std::cerr << Name() << "::" << m_detector << "::" << __PRETTY_FUNCTION__
              << "DST Node missing, doing nothing." << std::endl;
    throw std::runtime_error(
        "Failed to find DST node in RawTowerCalibration::CreateNodes");
  }

  PHCompositeNode *dstNodeSim = dynamic_cast<PHCompositeNode *>(simIter.findFirst(
      "PHCompositeNode", "DST"));
  if (!dstNodeSim)
  {
    std::cerr << Name() << "::" << m_detector << "::" << __PRETTY_FUNCTION__
              << "DSTsim Node missing, doing nothing." << std::endl;
    throw std::runtime_error(
        "Failed to find DSTsim node in RawTowerCalibration::CreateNodes");
  }

  std::string DataTowerNodeName = "TOWERINFO_CALIB_" + m_detector;
  _data_towers = findNode::getClass<TowerInfoContainerv1>(dstNode,
                                                           DataTowerNodeName);
  if (!_data_towers)
  {
    std::cerr << Name() << "::" << m_detector << "::" << __PRETTY_FUNCTION__
              << "Tower Calib Node missing, doing nothing." << std::endl;
    throw std::runtime_error(
        "Failed to find Calib Tower node in CaloTreeGenCombine::CreateNodes");
  }


  std::string SimTowerNodeName = "TOWERINFO_CALIB_" + m_detector;
  _sim_towers = findNode::getClass<TowerInfoContainerv1>(dstNodeSim,
                                                           SimTowerNodeName);
  if (!_sim_towers)
  {
    std::cerr << Name() << "::" << m_detector << "::" << __PRETTY_FUNCTION__
              << "Tower Calib Sim Node missing, doing nothing." << std::endl;
    throw std::runtime_error(
        "Failed to find Calib Tower Sim node in CaloTreeGenCombine::CreateNodes");
  }

  return;
}
