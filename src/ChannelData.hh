#ifndef _sbnddaq_analysis_WaveformData
#define _sbnddaq_analysis_WaveformData

#include <vector>

#include "PeakFinder.hh"

struct ChannelData {
  double baseline;
  double max;
  double rms;
  double last_channel_correlated_rms;
  double next_channel_correlated_rms;
  std::vector<double> waveform;
  std::vector<double> fft_real;
  std::vector<double> fft_imag;
  std::vector<PeakFinder::Peak> peaks;
  
};

#endif
