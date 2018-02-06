#ifndef _sbnddaq_analysis_Analysis
#define _sbnddaq_analysis_Analysis
#include <vector>

#include "canvas/Utilities/InputTag.h"

class Analysis {
public:
  struct AnalysisConfig {
    public:
    std::string output_file_name;
    double frame_to_dt;
    bool verbose;
    int n_events;
    unsigned n_baseline_samples;
    size_t n_channels;
    art::InputTag daq_tag;
  };

  Analysis() {}
  Analysis(AnalysisConfig config);
  ~Analysis();
  bool ProcessEvent(gallery::Event &event);
  void ProcessFragment(const artdaq::Fragment &frag);

  inline HeaderData *HeaderDataRef() { return &_header_data; }
  inline std::vector<ChannelData> *ChannelDataRef() { return &_per_channel_data; }

protected:
  HeaderData _header_data;
  std::vector<ChannelData> _per_channel_data;
  TFile _output_file;
  TTree *_t_header_data;
  TTree *_t_channel_data;
  AnalysisConfig _config;
  unsigned _event_ind;
};

#endif
