#ifndef _sbnddaq_analysis_WaveformData
#define _sbnddaq_analysis_WaveformData

struct ChannelData {
  double baseline;
  unsigned peak;
  std::vector<unsigned> waveform;
  std::vector<double> fft_real;
  std::vector<double> fft_imag;
};

#endif
