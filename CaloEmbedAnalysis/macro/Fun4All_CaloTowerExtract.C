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

#include <calotreegen/caloTowerExtract.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libcaloEmbedAnalysis.so)
#endif

void Fun4All_CaloTowerExtract(const int nEvents = 0, const char *listFileData = "data_DSTs.list", const char *listFileSim = "sim_DSTs.list", const char *listFileEmbed = "allCaloEmbed_DST.list", const char *listFileTruth = "truth_DSTs.list", const char *inName = "caloTowerExtract_eventData.root")
{
  gSystem->Load("libg4dst.so");
  Fun4AllServer *se = Fun4AllServer::instance();
  recoConsts *rc = recoConsts::instance();


  Fun4AllSyncManager *sync = se->getSyncManager();
  sync->MixRunsOk(true);

  Fun4AllInputManager *inData = new Fun4AllDstInputManager("DSTData","DST","TOPData");
  inData->AddListFile(listFileData);
  se->registerInputManager(inData);

  Fun4AllInputManager *inSim = new Fun4AllNoSyncDstInputManager("DSTSim","DST","TOPSim");
  inSim->AddListFile(listFileSim);
  inSim->Verbosity(10);
  se->registerInputManager(inSim);

  Fun4AllInputManager *inEmbed = new Fun4AllNoSyncDstInputManager("DSTEmbed","DST","TOPEmbed");
  inEmbed->AddListFile(listFileEmbed);
  inEmbed->Verbosity(10);
  se->registerInputManager(inEmbed);

  Fun4AllInputManager *inTruth = new Fun4AllNoSyncDstInputManager("DSTTruth","DST","TOPTruth");
  inTruth->AddListFile(listFileTruth);
  inTruth->Verbosity(10);
  se->registerInputManager(inTruth);


  std::string outputName = inName;

  caloTowerExtract *calo = new caloTowerExtract();
  calo->setOutputFileName(outputName);
  se->registerSubsystem(calo,"TOPSim");


  se->run(nEvents);
  se->End();
  se->PrintTimer();
  std::cout << "All done!" << std::endl;

  gSystem->Exit(0);
}
