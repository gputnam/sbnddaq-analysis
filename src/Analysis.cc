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
#include "Noise.hh"
#include "PeakFinder.hh"

Analysis::Analysis(AnalysisConfig config) : 
  _header_data(), 
  _per_channel_data(config.n_channels),
  _output_file(config.output_file_name.c_str(), "RECREATE")
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

  // clear out containers from last iter
  for (unsigned i = 0; i < _config.n_channels; i++) {
    _per_channel_data[i].waveform.clear();
    _per_channel_data[i].fft_real.clear();
    _per_channel_data[i].fft_imag.clear();
    _per_channel_data[i].peaks.clear();
  }

  auto const& daq_handle = event.getValidHandle<std::vector<artdaq::Fragment>>(_config.daq_tag);
  
  for (auto const& rawfrag: *daq_handle) {
    ProcessFragment(rawfrag);
  }
  // Fill trees
  _t_header_data->Fill();
  _t_channel_data->Fill();
  
  return true;
}

void Analysis::ProcessFragment(const artdaq::Fragment &frag) {

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
    if (waveform.first < _config.n_channels) {

      double max = 0;
      //std::vector<double> waveform_samples;
      for (unsigned raw_value: waveform.second) {
        double adc = (double) raw_value;
        if (adc > max) max = adc;
      
        _per_channel_data[waveform.first].waveform.push_back(adc);
      }

      // Baseline calculation assumes baseline is constant and that the first
      // `n_baseline_samples` of the adc values represent a baseline
      _per_channel_data[waveform.first].baseline = 
          std::accumulate(_per_channel_data[waveform.first].waveform.begin(), _per_channel_data[waveform.first].waveform.begin() + _config.n_baseline_samples, 0.0) 
          / _config.n_baseline_samples;
      
      _per_channel_data[waveform.first].max = max;
      
      // calculate FFTs
      FFT adc_fft(_per_channel_data[waveform.first].waveform);
      int adc_fft_size = adc_fft.size();
      fftw_complex *adc_fft_data = adc_fft.data();
      for (int i = 0; i < adc_fft_size; i++) {
        _per_channel_data[waveform.first].fft_real.push_back(adc_fft_data[i][0]);
        _per_channel_data[waveform.first].fft_imag.push_back(adc_fft_data[i][1]);
      } 

      // get Peaks
      PeakFinder peaks(_per_channel_data[waveform.first].waveform, _per_channel_data[waveform.first].baseline, _config.n_smoothing_samples);
      _per_channel_data[waveform.first].peaks.assign(peaks.Peaks()->begin(), peaks.Peaks()->end());
    }
  }
  // now calculate stuff that depends on stuff between channels
  for (unsigned i = 0; i < _config.n_channels; i++) {
    unsigned last_channel_ind = i == 0 ? _config.n_channels - 1 : i - 1;
    unsigned next_channel_ind = i == _config.n_channels - 1 ? 0 : i + 1;
    Noise last_channel_noise(_per_channel_data[i].waveform, _per_channel_data[last_channel_ind].waveform, _config.n_baseline_samples);
    Noise next_channel_noise(_per_channel_data[i].waveform, _per_channel_data[next_channel_ind].waveform, _config.n_baseline_samples);
    _per_channel_data[i].rms = last_channel_noise.RMS1();
    _per_channel_data[i].last_channel_correlation = last_channel_noise.Correlation();
    _per_channel_data[i].next_channel_correlation = next_channel_noise.Correlation();
  }

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

