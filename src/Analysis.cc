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

Analysis::Analysis(AnalysisConfig config) : 
  _header_data(), 
  _per_channel_data(config.n_channels),
  _output_file(config.output_file_name, "RECREATE")
{
  _output_file.cd();
  
  _t_header_data = new TTree("nevis_header", "nevis_header");
  _t_header_data->Branch("header_data", &_header_data);
  _t_channel_data = new TTree("channel_data", "channel_data");
  _t_channel_data->Branch("channel_data", &_per_channel_data);   

  _config = config;
  _event_ind = 0;
}

bool Analysis::ProcessEvent(gallery::Event &event) {
  if (_config.n_events >= 0 && _event_ind >= (unsigned)_config.n_events) return false;

  _event_ind ++;

  auto const& daq_handle = event.getValidHandle<std::vector<artdaq::Fragment>>(_config.daq_tag);
  
  for (auto const& rawfrag: *daq_handle) {
    ProcessFragment(rawfrag);
  }
  return true;
}

void Analysis::ProcessFragment(const artdaq::Fragment &frag) {
  // clear out containers from last iter
  for (unsigned i = 0; i < _config.n_channels; i++) {
    _per_channel_data[i].waveform.clear();
    _per_channel_data[i].fft_real.clear();
    _per_channel_data[i].fft_imag.clear();
  }

  sbnddaq::NevisTPCFragment fragment(frag);
  auto fragment_header = fragment.header(); 
  
  _header_data = HeaderData(fragment_header, _config.frame_to_dt); 
  _output_file.cd();
  
  if (_config.verbose) {
    std::cout << "EVENT NUMBER: "  << _header_data.event_number << std::endl;
    std::cout << "FRAME NUMBER: "  << _header_data.frame_number << std::endl;
    std::cout << "CHECKSUM: "  << _header_data.checksum << std::endl;
    std::cout << "ADC WORD COUNT: "  << _header_data.adc_word_count << std::endl;
    std::cout << "TRIG FRAME NUMBER: "  << _header_data.trig_frame_number << std::endl;
  }
  
  std::unordered_map<uint16_t,sbnddaq::NevisTPC_Data_t> waveform_map;
  size_t n_waveforms = fragment.decode_data(waveform_map);
  if (_config.verbose) {
    std::cout << "Decoded data. Found " << n_waveforms << " waveforms." << std::endl;
  }
  for (auto waveform: waveform_map) {
    if (_config.save_waveforms && waveform.first < _config.n_channels) {

      unsigned peak = 0;
      std::vector<double> waveform_samples;
      for (auto adc: waveform.second) {
        if (adc > peak) peak = adc;
      
        waveform_samples.push_back((double) adc);
        _per_channel_data[waveform.first].waveform.push_back(adc);
      }

      // Baseline calculation assumes baseline is constant and that the first
      // `n_baseline_samples` of the adc values represent a baseline
      _per_channel_data[waveform.first].baseline = 
          std::accumulate(waveform_samples.begin(), waveform_samples.begin() + _config.n_baseline_samples, 0.0) / _config.n_baseline_samples;
      
      _per_channel_data[waveform.first].peak = peak;
      
      FFT adc_fft(waveform_samples);
      int adc_fft_size = adc_fft.size();
      fftw_complex *adc_fft_data = adc_fft.data();
      for (int i = 0; i < adc_fft_size; i++) {
        _per_channel_data[waveform.first].fft_real.push_back(adc_fft_data[i][0]);
        _per_channel_data[waveform.first].fft_imag.push_back(adc_fft_data[i][1]);
      } 
    }
  }
  _t_header_data->Fill();
  _t_channel_data->Fill();
  
}

Analysis::~Analysis() {
  if (_config.verbose) {
    std::cout << "Saving..." << std::endl;
  }
  _output_file.cd();
  _t_header_data->Write();
  _t_channel_data->Write();
  _output_file.Close();
}

