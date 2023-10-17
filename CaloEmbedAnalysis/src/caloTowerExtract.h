// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef CALOTOWEREXTRACT_H
#define CALOTOWEREXTRACT_H

#include <calobase/TowerInfoContainer.h>  // for TowerInfoContainer, TowerIn...
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4main/PHG4Particle.h>

#include <calobase/RawTowerGeom.h>
#include <calobase/RawTowerGeomContainer.h>

#include <globalvertex/GlobalVertexMapv1.h>

#include <fun4all/SubsysReco.h>

#include <cassert>
#include <iostream>
#include <string>

#include "TFile.h"
#include "TTree.h"
//#include "Math/Vector3D.h"
//#include "Math/Vector4D.h"

class CDBInterface;
class CDBTTree;
class PHCompositeNode;
class TowerInfoContainerv1;
class PHG4TruthInfoContainer;
class PHG4Particle;
class RawTowerGeomContainer;

class caloTowerExtract : public SubsysReco
{
 public:
  caloTowerExtract(const std::string &name = "caloTowerExtract");

  ~caloTowerExtract() override;

  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  int End(PHCompositeNode *topNode) override;
  void CreateNodeTree(PHCompositeNode *topNode);
  

  enum DetectorSystem
  {
    CEMC = 0,
    HCALIN = 1,
    HCALOUT = 2,
    EPD = 3
  };

  void set_detector_type(caloTowerExtract::DetectorSystem dettype)
  {
    m_dettype = dettype;
    return;
  }
  void setCalibName(const std::string &name)
  {
    m_calibName = name;
    m_overrideCalibName = 1;
    return;
  }
  void setFieldName(const std::string &name)
  {
    m_fieldname = name;
    m_overrideFieldName = 1;
    return;
  }

  void setOutputFileName(std::string fileName){ m_outputFileName = fileName; };

 private:
  TowerInfoContainerv1 *_data_towers[3] = {nullptr,nullptr,nullptr};
  TowerInfoContainerv1 *_sim_towers[3] = {nullptr,nullptr,nullptr};
  TowerInfoContainerv1 *_embed_towers[3] = {nullptr,nullptr,nullptr};
  PHG4TruthInfoContainer *_truth_info = nullptr;
  TowerInfoContainerv1 *_data_MBD = nullptr;
  GlobalVertexMapv1 *_data_vtx_map = nullptr;

  RawTowerGeomContainer *tower_geom = nullptr;
  RawTowerGeomContainer *tower_geomIH = nullptr;
  RawTowerGeomContainer *tower_geomOH = nullptr;

  caloTowerExtract::DetectorSystem m_dettype;

  std::string m_detector;
  TowerInfoContainer::DETECTOR m_DETECTOR;
  std::string m_fieldname;
  std::string m_calibName;
  bool m_overrideCalibName = 0;
  bool m_overrideFieldName = 0;

  CDBInterface *cdb = nullptr;
  CDBTTree *cdbttree = nullptr;
  int m_runNumber;

  std::string m_outputFileName = "caloTowerExtractTree.root";
  TFile *m_outfile = nullptr;
  TTree *jetRecoTree = nullptr;
  
  /*
  std::vector<ROOT::Math::PxPyPzEVector> dataTowers;
  std::vector<ROOT::Math::PxPyPzEVector> simTowers;
  std::vector<ROOT::Math::PxPyPzEVector> embedTowers;
  std::vector<ROOT::Math::PxPyPzEVector> truthTracks;
  */

  std::vector<double> eventData;
  std::vector<std::vector<double>> dataTowers;
  std::vector<std::vector<double>> simTowers;
  std::vector<std::vector<double>> embedTowers;
  std::vector<std::vector<double>> truthTracks;
};

#endif  // CALOTOWEREXTRACT_H
