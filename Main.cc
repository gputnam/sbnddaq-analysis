//some standard C++ includes
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <numeric>
#include <getopt.h>

//some ROOT includes
#include "TInterpreter.h"
#include "TROOT.h"
#include "TH1F.h"
#include "TTree.h"
#include "TFile.h"
#include "TStyle.h"
#include "TSystem.h"
#include "TGraph.h"
#include "TFFTReal.h"

//"art" includes (canvas, and gallery)
#include "canvas/Utilities/InputTag.h"
#include "gallery/Event.h"
#include "gallery/ValidHandle.h"
#include "canvas/Persistency/Common/FindMany.h"
#include "canvas/Persistency/Common/FindOne.h"

#include "sbnddaq-datatypes/Overlays/NevisTPCFragment.hh"
#include "sbnddaq-datatypes/NevisTPC/NevisTPCTypes.hh"
#include "sbnddaq-datatypes/NevisTPC/NevisTPCUtilities.hh"

#include "artdaq-core/Data/Fragment.hh"

#include "HeaderData.hh"
#include "ChannelData.hh"
#include "FFT.hh"
#include "Analysis.hh"

//This way you can be lazy
using namespace art;

int main(int argv, char** argc) {
  if (argv == 0) {
    std::cout << "Pass in some imput files" << std::endl;
    return 1;
  }
  //We have passed the input file as an argument to the function 
  std::vector<std::string> filename;
  for (int i = 1; i < argv; ++i) { 
    std::cout << "FILE : " << argc[i] << std::endl; 
    filename.push_back(std::string(argc[i]));
  }

  // TODO: make these configurable at command line
  Analysis::AnalysisConfig config;
  config.frame_to_dt = 1.6e-3; // units of seconds
  config.output_file_name = (char *)"output.root";
  config.save_waveforms = true;
  config.verbose = false;
  config.n_events = 10;
  config.n_baseline_samples = 20;

  config.daq_tag = art::InputTag("daq","NEVISTPC");

  // TODO: how to detect this?
  // currently only the first 16 channels have anything interesting
  config.n_channels = 16;
  
  Analysis ana(config);

  for (gallery::Event ev(filename) ; !ev.atEnd(); ev.next()) {
   if (!ana.ProcessEvent(ev)) break;
  }
  return 0;
}
