//some standard C++ includes
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <numeric>
#include <fstream>

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
#include "artdaq-core/Data/Fragment.hh"

// other shared libraries
#include <json/json.h>

// sbnddaq stuff
#include "sbnddaq-datatypes/Overlays/NevisTPCFragment.hh"
#include "sbnddaq-datatypes/NevisTPC/NevisTPCTypes.hh"
#include "sbnddaq-datatypes/NevisTPC/NevisTPCUtilities.hh"

// local stuff
#include "HeaderData.hh"
#include "ChannelData.hh"
#include "FFT.hh"
#include "Analysis.hh"

using namespace art;

int main(int argv, char** argc) {
  if (argv < 3) {
    std::cout << "Usage: config_file [input_files]" << std::endl;
    return 1;
  }
  // parse configuration
  std::ifstream configstream(argc[1], std::ifstream::binary);
  Json::Value user_config;
  Json::Reader reader;
  bool r = reader.parse(configstream, user_config);
  if (!r) {
    std::cout << "Error parsing configuration file" << std::endl;
    return 1;
  }

  // get input files
  std::vector<std::string> filename;
  for (int i = 2; i < argv; ++i) { 
    std::cout << "FILE : " << argc[i] << std::endl; 
    filename.push_back(std::string(argc[i]));
  }

  Analysis::AnalysisConfig config;
  config.frame_to_dt = (double) user_config.get("frame_to_dt", 1.6e-3 /* units of seconds */).asFloat();
  config.output_file_name = (char *) user_config.get("output_file_name", "output.root").asCString();
  config.verbose = user_config.get("verbose", true).asBool();
  config.n_events = user_config.get("n_events", 10).asUInt();
  config.n_baseline_samples = user_config.get("n_baseline_samples", 20).asUInt();

  // TODO: should this be in config file?
  config.daq_tag = art::InputTag("daq","NEVISTPC");

  // TODO: how to detect this?
  config.n_channels = user_config.get("n_channels", 16 /* currently only the first 16 channels have data */).asUInt();
  
  Analysis ana(config);

  for (gallery::Event ev(filename) ; !ev.atEnd(); ev.next()) {
   if (!ana.ProcessEvent(ev)) break;
   // @ANDY:
   // I think this is where you'd want to put any redis code.
   // You can access the channel data (stored as a vector<ChannelData>)
   // by calling ana.ChannelDataRef()
   // And yo ucan access the header data (stored as a HeaderData)
   // by calling ana.HeaderDataRef()
  }
  return 0;
}
