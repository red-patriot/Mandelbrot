#include <complex>

#include "Mandelbrot.h"

int main() {
  Mandelbrot m;
  if (!m.init(std::complex<double>(1.0, 1.0), std::complex<double>(-2.0, -1.0))) {
    return -1;
  }

  m.run_plot();
  
  return 0;
}


/*
 * TODO: Add logging/progress bars for plot calculation
 * TODO: Add smooth coloring
 */
