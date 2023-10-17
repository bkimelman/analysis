// Tell emacs that this is a C++ source
//  -*- C++ -*-.
#ifndef CALOTREEGENCOMBINE_H
#define CALOTREEGENCOMBINE_H

#include <calobase/TowerInfoContainer.h>  // for TowerInfoContainer, TowerIn...

#include <fun4all/SubsysReco.h>

#include <cassert>
#include <iostream>
#include <string>

class CDBInterface;
class CDBTTree;
class PHCompositeNode;
class TowerInfoContainerv1;

class CaloTreeGenCombine : public SubsysReco
{
 public:
  CaloTreeGenCombine(const std::string &name = "CaloTreeGenCombine");

  ~CaloTreeGenCombine() override;

  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  void CreateNodeTree(PHCompositeNode *topNode);
  

  enum DetectorSystem
  {
    CEMC = 0,
    HCALIN = 1,
    HCALOUT = 2,
    EPD = 3
  };

  void set_detector_type(CaloTreeGenCombine::DetectorSystem dettype)
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

 private:
  TowerInfoContainerv1 *_data_towers = nullptr;
  TowerInfoContainerv1 *_sim_towers = nullptr;

  CaloTreeGenCombine::DetectorSystem m_dettype;

  std::string m_detector;
  TowerInfoContainer::DETECTOR m_DETECTOR;
  std::string m_fieldname;
  std::string m_calibName;
  bool m_overrideCalibName = 0;
  bool m_overrideFieldName = 0;

  CDBInterface *cdb = nullptr;
  CDBTTree *cdbttree = nullptr;
  int m_runNumber;
};

#endif  // CALOTREEGENCOMBINE_H
