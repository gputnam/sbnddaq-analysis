#ifndef _sbnddaq_analysis_PeakFinder
#define _sbnddaq_analysis_PeakFinder
#include <vector>
#include <float.h>

// Reinventing the wheel: search for a bunch of peaks in a set of data
// 
// Implementation: searches for points above some threshold (requiring a 
// good basline) and tries to make peaks for them.
class PeakFinder {
public:
  struct Peak {
    double amplitude;
    unsigned width;
    double baseline;

    Peak() {
      amplitude = -DBL_MAX;
      width = 0;      
      baseline = 0;
    }
  };

  PeakFinder(std::vector<double> waveform, double baseline, double threshold=100., unsigned n_smoothing_points=5);
  inline std::vector<Peak> *Peaks() { return &_peaks; }
private:
  std::vector<double> _smoothed_waveform;
  std::vector<Peak> _peaks;

};
#endif
