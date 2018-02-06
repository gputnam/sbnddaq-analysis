#include <vector>
#include <numeric>


#include "Noise.hh"

Noise::Noise(std::vector<double> &inp_wvfm_1, std::vector<double> &inp_wvfm_2, int n_baseline_samples) {
  for (int i = 0; i < n_baseline_samples; i++) {
    wvfm_1.push_back( inp_wvfm_1[i] );
    wvfm_2.push_back( inp_wvfm_2[i] );
    wvfm_diff.push_back( inp_wvfm_1[i] - inp_wvfm_2[i] );
  } 

  double wvfm_1_baseline = 
    std::accumulate(wvfm_1.begin(), wvfm_1.end(), 0.0) / wvfm_1.size();
  double wvfm_2_baseline = 
    std::accumulate(wvfm_2.begin(), wvfm_2.end(), 0.0) / wvfm_1.size();
  double wvfm_diff_baseline = wvfm_1_baseline - wvfm_2_baseline;

  for (int i = 0; i < n_baseline_samples; i++) {
    wvfm_1[i] -= wvfm_1_baseline;
    wvfm_2[i] -= wvfm_2_baseline;
    wvfm_diff[i] -= wvfm_diff_baseline;
  }

}

double Noise::CalcRMS(std::vector<double> &wvfm) {
  double ret = 0;
  for (unsigned i = 0; i < wvfm.size(); i++) {
    ret += wvfm[i] * wvfm[i];
  }
  ret /= wvfm.size();
  return ret;
  
}
