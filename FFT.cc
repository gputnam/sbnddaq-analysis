#include <vector>

#include "fftw3.h"

#include "FFT.hh"

FFT::FFT(std::vector<double>& input) {
  int input_size = input.size();
  double *fft_input = &input[0];
  
  _fft_size = 2 * (input_size / 2 + 1);
  _fft_array = fftw_alloc_complex(input_size);
  
  unsigned flags = FFTW_ESTIMATE;
  
  _plan = fftw_plan_dft_r2c_1d(input_size, fft_input, _fft_array, flags);
  
  fftw_execute(_plan);
}

FFT::~FFT() {
  fftw_free(_fft_array);
}

int FFT::size() {
  return _fft_size;
}

fftw_complex *FFT::data() {
  return _fft_array;
}

