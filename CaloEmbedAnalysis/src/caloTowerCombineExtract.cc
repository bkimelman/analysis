#include "caloTowerCombineExtract.h"

#include <calobase/TowerInfo.h>  // for TowerInfo
#include <calobase/TowerInfoContainer.h>
#include <calobase/TowerInfoContainerv1.h>
#include <calobase/TowerInfoContainerv2.h>
#include <calobase/TowerInfov1.h>
#include <calobase/TowerInfoDefs.h>
#include <calobase/RawTowerGeom.h>
#include <calobase/RawTowerGeomContainer.h>

#include <mbd/MbdOut.h>

#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4Particlev1.h>
#include <g4main/PHG4Particlev2.h>

#include <globalvertex/GlobalVertexMapv1.h>
#include <globalvertex/GlobalVertex.h>

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

#include "TFile.h"
#include "TTree.h"
//#include "Math/Vector3D.h"
//#include "Math/Vector4D.h"

//using namespace ROOT::Math;

//____________________________________________________________________________..
caloTowerCombineExtract::caloTowerCombineExtract(const std::string &name)
  : SubsysReco(name)
  , m_dettype(caloTowerCombineExtract::HCALOUT)
  , m_detector("HCALOUT")
  , m_DETECTOR(TowerInfoContainer::HCAL)
  , m_fieldname("")
  , m_runNumber(-1)
{
  std::cout << "caloTowerCombineExtract::caloTowerCombineExtract(const std::string &name) Calling ctor" << std::endl;
}

//____________________________________________________________________________..
caloTowerCombineExtract::~caloTowerCombineExtract()
{
  std::cout << "caloTowerCombineExtract::~caloTowerCombineExtract() Calling dtor" << std::endl;
}

//____________________________________________________________________________..
int caloTowerCombineExtract::InitRun(PHCompositeNode *topNode)
{

  Fun4AllServer *se = Fun4AllServer::instance();

  PHCompositeNode *dataTopNode = se->topNode("TOPData");

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

  m_outfile = new TFile(m_outputFileName.c_str(), "RECREATE");

  jetRecoTree = new TTree("jetRecoTree","jetRecoTree");

  jetRecoTree->Branch("eventData",&eventData);
  jetRecoTree->Branch("dataTowers",&dataTowers);
  //  jetRecoTree->Branch("dataTowers","std::vector<ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > * >",&dataTowersTmp);
  jetRecoTree->Branch("simTowers",&simTowers);
  jetRecoTree->Branch("embedTowers",&embedTowers);
  jetRecoTree->Branch("truthTracks",&truthTracks);

  jetRecoTree->Print();

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int caloTowerCombineExtract::process_event(PHCompositeNode* topNode)
{


  std::cout << "working on event " << jetRecoTree->GetEntries() << std::endl;
  /*
  std::vector<ROOT::Math::PxPyPzEVector> dataTowers;
  std::vector<ROOT::Math::PxPyPzEVector> simTowers;
  std::vector<ROOT::Math::PxPyPzEVector> embedTowers;
  std::vector<ROOT::Math::PxPyPzEVector> truthTracks;
  
  jetRecoTree->SetBranchAddress("dataTowers",&dataTowers);
  jetRecoTree->SetBranchAddress("simTowers",&simTowers);
  jetRecoTree->SetBranchAddress("embedTowers",&embedTowers);
  jetRecoTree->SetBranchAddress("truthTracks",&truthTracks);
  */

  
  double totalMBDEnergy = 0.0;
  /*
  unsigned int ntowers_MBD = _data_MBD->size();
  for (unsigned int channel = 0; channel < ntowers_MBD; channel++)
    {

      if(TowerInfoDefs::get_mbd_type(TowerInfoDefs::encode_mbd(channel)) == 1){
	totalMBDEnergy += _data_MBD->get_tower_at_channel(channel)->get_energy();
      }
    }
  */
  totalMBDEnergy = _data_MBD->get_q(0) + _data_MBD->get_q(1);

  //  std::cout << "totalMBDEnergy=" << totalMBDEnergy << "   vtx pointer: " << _data_vtx_map->get(0) << "   vtxZ=" << _data_vtx_map->get(0)->get_z() << std::endl;
  std::cout << "totalMBDEnergy=" << totalMBDEnergy << "   vtx pointer: " << _data_vtx_map->get(0) << std::endl;

  double z_vtx = 0.0;
  if(!_data_vtx_map->get(0)){
    //return Fun4AllReturnCodes::ABORTEVENT;
    z_vtx = -999.999;
  }
  else z_vtx = _data_vtx_map->get(0)->get_z();

  eventData.clear();
  eventData.push_back(z_vtx);
  eventData.push_back(totalMBDEnergy);
  


  dataTowers.clear();
  simTowers.clear();
  embedTowers.clear();
  truthTracks.clear();

  std::cout << "cleared vectors" << std::endl;

  for(int caloIndex=0; caloIndex<3; caloIndex++){

    std::cout << "event " << jetRecoTree->GetEntries() << " working on caloIndex " << caloIndex;
    if(caloIndex == 0) std::cout << " CEMC" << std::endl;
    else if(caloIndex == 1) std::cout << " HCALIN" << std::endl;
    else if(caloIndex == 2) std::cout << " HCALOUT" << std::endl;
    RawTowerDefs::keytype keyData = 0;
    RawTowerDefs::keytype keySim = 0;

    unsigned int ntowers = _data_towers[caloIndex]->size();
    for (unsigned int channel = 0; channel < ntowers; channel++)
      {
	//std::cout << "working on channel " << channel << std::endl;
	unsigned int data_key = _data_towers[caloIndex]->encode_key(channel);
	unsigned int sim_key = _sim_towers[caloIndex]->encode_key(channel);

	//std::cout << "got keys" << std::endl;

	int ieta_data = _data_towers[caloIndex]->getTowerEtaBin(data_key);
	int iphi_data = _data_towers[caloIndex]->getTowerPhiBin(data_key);
	int ieta_sim = _sim_towers[caloIndex]->getTowerEtaBin(sim_key);
	int iphi_sim = _sim_towers[caloIndex]->getTowerPhiBin(sim_key);

	//std::cout << "got phi and eta bins" << std::endl;

	if(caloIndex == 0){
	  //std::cout << "getting CEMC keys" << std::endl;
	  keyData = RawTowerDefs::encode_towerid(RawTowerDefs::CalorimeterId::CEMC, ieta_data, iphi_data);
	  keySim = RawTowerDefs::encode_towerid(RawTowerDefs::CalorimeterId::CEMC, ieta_sim, iphi_sim);
	}else if(caloIndex == 1){
	  //std::cout << "getting HCALIN keys" << std::endl;
	  keyData = RawTowerDefs::encode_towerid(RawTowerDefs::CalorimeterId::HCALIN, ieta_data, iphi_data);
	  keySim = RawTowerDefs::encode_towerid(RawTowerDefs::CalorimeterId::HCALIN, ieta_sim, iphi_sim);
	}else if(caloIndex == 2){
	  //std::cout << "getting HCALOUT keys" << std::endl;
	  keyData = RawTowerDefs::encode_towerid(RawTowerDefs::CalorimeterId::HCALOUT, ieta_data, iphi_data);
	  keySim = RawTowerDefs::encode_towerid(RawTowerDefs::CalorimeterId::HCALOUT, ieta_sim, iphi_sim);
	}
	
	//std::cout << "got calo keys" << std::endl;

	TowerInfo *caloinfo_data = _data_towers[caloIndex]->get_tower_at_channel(channel);
	TowerInfo *caloinfo_sim = _sim_towers[caloIndex]->get_tower_at_channel(channel);

	//	std::cout << "got tower infos" << std::endl;

	float data_E = caloinfo_data->get_energy();
	float sim_E = caloinfo_sim->get_energy();
	float embed_E = data_E + sim_E;

	//	std::cout << "got energies" << std::endl;

	float data_phi = 0.0;
	float sim_phi = 0.0;
	float embed_phi = 0.0;

	float data_eta = 0.0;
	float sim_eta = 0.0;
	float embed_eta = 0.0;

	if(caloIndex == 0){

	  //	  std::cout << "in CEMC, getting eta and phi values" << std::endl;

	  data_phi = tower_geom->get_tower_geometry(keyData)->get_phi();
	  data_eta = tower_geom->get_tower_geometry(keyData)->get_eta();

	  //	  std::cout << "got data" << std::endl;

	  sim_phi = tower_geom->get_tower_geometry(keySim)->get_phi();
	  sim_eta = tower_geom->get_tower_geometry(keySim)->get_eta();

	  //	  std::cout << "got sim" << std::endl;

	  embed_phi = data_phi;
	  embed_eta = data_eta;

	  //	  std::cout << "got embed" << std::endl;

	}else if(caloIndex == 1){
	  //	  std::cout << "in HCALIN, getting eta and phi values" << std::endl;

	  data_phi = tower_geomIH->get_tower_geometry(keyData)->get_phi();
	  data_eta = tower_geomIH->get_tower_geometry(keyData)->get_eta();

	  //	  std::cout << "got data" << std::endl;

	  sim_phi = tower_geomIH->get_tower_geometry(keySim)->get_phi();
	  sim_eta = tower_geomIH->get_tower_geometry(keySim)->get_eta();

	  //	  std::cout << "got sim" << std::endl;

	  embed_phi = data_phi;
	  embed_eta = data_eta;

	  //	  std::cout << "got embed" << std::endl;

	}else if(caloIndex == 2){

	  //	  std::cout << "in HCALOUT, getting eta and phi values" << std::endl;

	  data_phi = tower_geomOH->get_tower_geometry(keyData)->get_phi();
	  data_eta = tower_geomOH->get_tower_geometry(keyData)->get_eta();

	  sim_phi = tower_geomOH->get_tower_geometry(keySim)->get_phi();
	  sim_eta = tower_geomOH->get_tower_geometry(keySim)->get_eta();

	  embed_phi = data_phi;
	  embed_eta = data_eta;
	}

	//	std::cout << "got eta and phi values" << std::endl;


	float data_pt = data_E / cosh(data_eta);
	float sim_pt = sim_E / cosh(sim_eta);
	float embed_pt = embed_E / cosh(embed_eta);

	float data_px = data_pt * cos(data_phi);
	float sim_px = sim_pt * cos(sim_phi);
	float embed_px = embed_pt * cos(embed_phi);

	float data_py = data_pt * sin(data_phi);
	float sim_py = sim_pt * sin(sim_phi);
	float embed_py = embed_pt * sin(embed_phi);

	float data_pz = data_pt * sinh(data_eta);
	float sim_pz = sim_pt * sinh(sim_eta);
	float embed_pz = embed_pt * sinh(embed_eta);


	//	std::cout << "got momenta components" << std::endl;


	std::vector<double> dataTower = {data_px, data_py, data_pz, data_E, 1.0*caloIndex};
	std::vector<double> simTower = {sim_px, sim_py, sim_pz, sim_E, 1.0*caloIndex};
	std::vector<double> embedTower = {embed_px, embed_py, embed_pz, embed_E, 1.0*caloIndex};

	//ROOT::Math::PxPyPzEVector dataTower(data_px, data_py, data_pz, data_E);
	//ROOT::Math::PxPyPzEVector simTower(sim_px, sim_py, sim_pz, sim_E);
	//ROOT::Math::PxPyPzEVector embedTower(embed_px, embed_py, embed_pz, embed_E);

	if(data_E > 0.3) dataTowers.push_back(dataTower);
	//	std::cout << "pushed back dataTower" << std::endl;
	if(sim_E > 0.3) simTowers.push_back(simTower);
	//	std::cout << "pushed back simTower" << std::endl;
	if(embed_E > 0.3) embedTowers.push_back(embedTower);
	//	std::cout << "pushed back embedTower" << std::endl;
	
      }
  }


  std::cout << "done with calo stuff" << std::endl;
  PHG4TruthInfoContainer::ConstRange primary_range = _truth_info->GetPrimaryParticleRange();
  
  std::cout << "got range for particles from truth" << std::endl;

  for(PHG4TruthInfoContainer::ConstIterator particle_iter = primary_range.first; particle_iter != primary_range.second; ++particle_iter){

    //    std::cout << "working on particle " << particle_iter->first << std::endl;

    PHG4Particle *part = particle_iter->second;

    //    std::cout << "got particle " << part << std::endl;
    
    //ROOT::Math::PxPyPzEVector truthTrack(part->get_px(), part->get_py(), part->get_pz(), part->get_e());
    std::vector<double> truthTrack = {part->get_px(), part->get_py(), part->get_pz(), part->get_e()};
    //    std::cout << "made particle 4Vector " << std::endl;
    truthTracks.push_back(truthTrack);
    //    std::cout << "pushed back truthTrack" << std::endl;
    
  }  

  std::cout << "finished particle loop" << std::endl;


  jetRecoTree->Fill();

  std::cout << "filled jetRecoTree: " << jetRecoTree->GetEntries() << std::endl;

  return Fun4AllReturnCodes::EVENT_OK;
}

int caloTowerCombineExtract::End(PHCompositeNode *topNode)
{

  m_outfile->cd();
  jetRecoTree->Write();

  m_outfile->Close();

  return Fun4AllReturnCodes::EVENT_OK;

}

void caloTowerCombineExtract::CreateNodeTree(PHCompositeNode *topNode)
{
  std::cout << "creating node" << std::endl;

  Fun4AllServer *se = Fun4AllServer::instance();
  PHCompositeNode *dataTopNode = se->topNode("TOPData");
  PHCompositeNode *truthTopNode = se->topNode("TOPTruth");


  tower_geom = findNode::getClass<RawTowerGeomContainer>(dataTopNode, "TOWERGEOM_CEMC");
  if(!tower_geom){
    std::cerr << Name() << "::" << __PRETTY_FUNCTION__
	      << "tower geom CEMC missing, doing nothing." << std::endl;
    throw std::runtime_error(
			     "Failed to find TOWERGEOM_CEMC node");
  }

  tower_geomIH = findNode::getClass<RawTowerGeomContainer>(dataTopNode, "TOWERGEOM_HCALIN");
  if(!tower_geomIH){
    std::cerr << Name() << "::" << __PRETTY_FUNCTION__
	      << "tower geom HCALIN missing, doing nothing." << std::endl;
    throw std::runtime_error(
			     "Failed to find TOWERGEOM_HCALIN node");
  }

  tower_geomOH = findNode::getClass<RawTowerGeomContainer>(dataTopNode, "TOWERGEOM_HCALOUT");
  if(!tower_geomOH){
    std::cerr << Name() << "::" << __PRETTY_FUNCTION__
	      << "tower geom HCALOUT missing, doing nothing." << std::endl;
    throw std::runtime_error(
			     "Failed to find TOWERGEOM_HCALOUT node");
  }

  PHNodeIterator dataIter(dataTopNode);
  PHNodeIterator simIter(topNode);
  PHNodeIterator truthIter(truthTopNode);


  //data top node first

  PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(dataIter.findFirst(
      "PHCompositeNode", "DST"));
  if (!dstNode)
  {
    std::cerr << Name() << "::" << m_detector << "::" << __PRETTY_FUNCTION__
              << "DST Node missing, doing nothing." << std::endl;
    throw std::runtime_error(
        "Failed to find DST node in RawTowerCalibration::CreateNodes");
  }

  PHCompositeNode *mbdNode = dynamic_cast<PHCompositeNode *>(dataIter.findFirst("PHCompositeNode", "MBD"));
  if(!mbdNode){
    std::cerr << Name() << "::" <<  __PRETTY_FUNCTION__
              << "MBD Node missing, doing nothing." << std::endl;
    throw std::runtime_error(
        "Failed to find MBD node in RawTowerCalibration::CreateNodes");    
  }

  _data_MBD = findNode::getClass<MbdOut>(mbdNode, "MbdOut");
  //  _data_MBD = findNode::getClass<TowerInfoContainerv1>(dstNode,"TOWERS_MBD");
  if(!_data_MBD){
    std::cerr << Name() << "::" <<  __PRETTY_FUNCTION__
              << "MbdOut Node missing, doing nothing." << std::endl;
    //<< "TOWERS_MBD Node missing, doing nothing." << std::endl;
    throw std::runtime_error(
			     //"Failed to find TOWERS_MBD node in RawTowerCalibration::CreateNodes");
        "Failed to find MbdOut node in RawTowerCalibration::CreateNodes");
  }

  _data_vtx_map = findNode::getClass<GlobalVertexMapv1>(dstNode,"GlobalVertexMap");
  if(!_data_vtx_map){
    std::cerr << Name() << "::" <<  __PRETTY_FUNCTION__
              << "GlobalVertexMap Node missing, doing nothing." << std::endl;
    throw std::runtime_error(
        "Failed to find GlobalVertexMap node in RawTowerCalibration::CreateNodes");
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

  PHCompositeNode *dstNodeTruth = dynamic_cast<PHCompositeNode *>(truthIter.findFirst(
      "PHCompositeNode", "DST"));
  if (!dstNodeTruth)
  {
    std::cerr << Name() << "::" << m_detector << "::" << __PRETTY_FUNCTION__
              << "DSTtruth Node missing, doing nothing." << std::endl;
    throw std::runtime_error(
        "Failed to find DSTtruth node in RawTowerCalibration::CreateNodes");
  }



  for(int i=0; i<3; i++){

    std::string detector = "CEMC";
    if(i==1) detector = "HCALIN";
    else if(i==2) detector = "HCALOUT";

    
    //data
    std::string DataTowerNodeName = "TOWERINFO_CALIB_" + detector;
    _data_towers[i] = findNode::getClass<TowerInfoContainerv2>(dstNode,
							       DataTowerNodeName);
    if (!_data_towers[i])
      {
	std::cerr << Name() << "::" << detector << "::" << __PRETTY_FUNCTION__
		  << "Tower Calib Node missing, doing nothing." << std::endl;
	throw std::runtime_error(
				 "Failed to find Calib Tower node in caloTowerCombineExtract::CreateNodes");
      }


    //sim
    std::string SimTowerNodeName = "TOWERINFO_CALIB_" + detector;
    _sim_towers[i] = findNode::getClass<TowerInfoContainerv1>(dstNodeSim,
                                                           SimTowerNodeName);
    if (!_sim_towers[i])
      {
	std::cerr << Name() << "::" << detector << "::" << __PRETTY_FUNCTION__
		  << "Tower Calib Sim Node missing, doing nothing." << std::endl;
	throw std::runtime_error(
				 "Failed to find Calib Tower Sim node in caloTowerCombineExtract::CreateNodes");
      }

  }//end loop over calorimeter types

  
  //truth
  std::string TruthContainerNodeName = "G4TruthInfo";
  _truth_info = findNode::getClass<PHG4TruthInfoContainer>(dstNodeTruth,
							      TruthContainerNodeName);
  if (!_truth_info)
    {
      std::cerr << Name() << "::" << __PRETTY_FUNCTION__
		<< "Tower Calib Truth Node missing, doing nothing." << std::endl;
      throw std::runtime_error(
			       "Failed to find Calib Tower Truth node in caloTowerCombineExtract::CreateNodes");
    }






  return;
}
