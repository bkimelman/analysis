#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllServer.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllNoSyncDstInputManager.h>

#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllSyncManager.h>
#include <fun4all/Fun4AllServer.h>

#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

#include <calotreegen/caloTreeGenCombine.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libcaloEmbedAnalysis.so)
#endif

  void Fun4All_CaloEmbed(const int nEvents = 0, const char *listFileData = "data_DSTs.list", const char *listFileSim = "dst_calo_cluster.list", const char *inName = "commissioning.root")
{
  gSystem->Load("libg4dst.so");
  Fun4AllServer *se = Fun4AllServer::instance();
  recoConsts *rc = recoConsts::instance();


  Fun4AllSyncManager *sync = se->getSyncManager();
  sync->MixRunsOk(true);

  Fun4AllInputManager *inData = new Fun4AllDstInputManager("DSTData");
  inData->AddListFile(listFileData);
  se->registerInputManager(inData);

  Fun4AllInputManager *inSim = new Fun4AllNoSyncDstInputManager("DSTSim","DST","TOPSim");
  inSim->AddListFile(listFileSim);
  inSim->Verbosity(10);
  se->registerInputManager(inSim);

  CaloTreeGenCombine *calo = new CaloTreeGenCombine(inName);
  calo->set_detector_type(CaloTreeGenCombine::CEMC);
  se->registerSubsystem(calo,"TOPSim");


  CaloTreeGenCombine *caloIH = new CaloTreeGenCombine(inName);
  caloIH->set_detector_type(CaloTreeGenCombine::HCALIN);
  se->registerSubsystem(caloIH,"TOPSim");


  CaloTreeGenCombine *caloOH = new CaloTreeGenCombine(inName);
  caloOH->set_detector_type(CaloTreeGenCombine::HCALOUT);
  se->registerSubsystem(caloOH,"TOPSim");


  Fun4AllDstOutputManager *out = new Fun4AllDstOutputManager("DSTOUT", inName);
  se->registerOutputManager(out);

  se->run(nEvents);
  se->End();
  se->PrintTimer();
  std::cout << "All done!" << std::endl;

  gSystem->Exit(0);
}
