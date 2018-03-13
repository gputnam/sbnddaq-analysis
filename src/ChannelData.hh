#ifndef _sbnddaq_analysis_WaveformData
#define _sbnddaq_analysis_WaveformData

#include <vector>

#include "PeakFinder.hh"

class ChannelData {
public:
  double baseline;
  double max;
  double rms;
  double last_channel_correlation;
  double next_channel_correlation;
  std::vector<double> waveform;
  std::vector<double> fft_real;
  std::vector<double> fft_imag;
  std::vector<PeakFinder::Peak> peaks;
  
};

#endif
