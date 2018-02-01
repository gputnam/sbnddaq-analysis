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

  art::InputTag daqTag("daq","NEVISTPC");

  // TODO: make these configurable at command line
  double frame_to_dt = 1.6e-3; // units of seconds
  TFile* output = new TFile("output.root","RECREATE");
  bool save_waveforms = true;
  bool verbose = false;
  int n_events = 10;
  unsigned n_baseline_samples = 20;

  // TODO: how to detect this?
  // currently only the first 16 channels have anything interesting
  size_t n_channels = 16;

  output->cd();

  TTree t_header_data("nevis_header", "nevis_header");
  HeaderData header_data;
  t_header_data.Branch("header_data", &header_data);

  TTree t_channel_data("channel_data", "channel_data");
  // NOTE: The way things are currently set up, if you push to per_channel_data
  // after this loop, the code will SEGFAULT. So don't do that.
  std::vector<ChannelData> per_channel_data(n_channels);
  t_channel_data.Branch("channel_data", &per_channel_data);

  int event_ind = 0;
  for (gallery::Event ev(filename) ; !ev.atEnd() && event_ind < n_events; ev.next()) {
    event_ind ++;

    auto const& daq_handle = ev.getValidHandle<std::vector<artdaq::Fragment>>(daqTag);

    // NOTE: Currently the code assumes there is only one frament per event. 
    // It does not try to do any maching of fragment id -> channel id.
    // This will have to be implemented later.
    for(auto const& rawfrag : *daq_handle){
      sbnddaq::NevisTPCFragment fragment(rawfrag);
      auto fragment_header = fragment.header(); 
      
      header_data = HeaderData(fragment_header, frame_to_dt); 
      output->cd();

      if (verbose) {
	std::cout << "EVENT NUMBER: "  << header_data.event_number << std::endl;
	std::cout << "FRAME NUMBER: "  << header_data.frame_number << std::endl;
	std::cout << "CHECKSUM: "  << header_data.checksum << std::endl;
	std::cout << "ADC WORD COUNT: "  << header_data.adc_word_count << std::endl;
	std::cout << "TRIG FRAME NUMBER: "  << header_data.trig_frame_number << std::endl;
      }

      std::unordered_map<uint16_t,sbnddaq::NevisTPC_Data_t> waveform_map;
      size_t n_waveforms = fragment.decode_data(waveform_map);
      if (verbose) {
        std::cout << "Decoded data. Found " << n_waveforms << " waveforms." << std::endl;
      }
      for (auto waveform: waveform_map) {
        if (save_waveforms && waveform.first < n_channels) {
          unsigned peak = 0;
          std::vector<double> waveform_samples;
          for (auto adc: waveform.second) {
            if (adc > peak) peak = adc;

            waveform_samples.push_back((double) adc);
            per_channel_data[waveform.first].waveform.push_back(adc);
          }
          // Baseline calculation assumes baseline is constant and that the first
          // `n_baseline_samples` of the adc values represent a baseline
          per_channel_data[waveform.first].baseline = 
            std::accumulate(waveform_samples.begin(), waveform_samples.begin() + n_baseline_samples, 0.0) / n_baseline_samples;

          per_channel_data[waveform.first].peak = peak;

          FFT adc_fft(waveform_samples);
          int adc_fft_size = adc_fft.size();
          fftw_complex *adc_fft_data = adc_fft.data();
          for (int i = 0; i < adc_fft_size; i++) {
            per_channel_data[waveform.first].fft_real.push_back(adc_fft_data[i][0]);
            per_channel_data[waveform.first].fft_imag.push_back(adc_fft_data[i][1]);
          } 
        }
      }  // iterate over fragments
      t_header_data.Fill();
      t_channel_data.Fill();

      // clear out containers for next iter
      for (unsigned i = 0; i < n_channels; i++) {
        per_channel_data[i].waveform.clear();
        per_channel_data[i].fft_real.clear();
        per_channel_data[i].fft_imag.clear();
      }
    }// Iterate through fragments
  }// Iterate through events

  if (verbose) {
    std::cout << "Saving..." << std::endl;
  }
  output->cd();
  t_header_data.Write();
  t_channel_data.Write();
  output->Close();
  return 0;
}
