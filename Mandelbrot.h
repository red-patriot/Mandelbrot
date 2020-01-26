#ifndef Mandelbrot_h_INCLUDED
#define Mandelbrot_h_INCLUDED

/* Mandelbrot.h
 * A class to calculate and plot a zoomable Mandelbrot set
 */

#include <SDL2/SDL.h>
#include <complex>
#include <vector>
#include <utility>

class Mandelbrot {
public:
  Mandelbrot(int escape_limit=500);
  ~Mandelbrot() = default;

  bool init(std::complex<long double> max, std::complex<long double> min,
            int width=2800, int height=1860);

  void run_plot();

  enum State {
              Running,
              NeedsToGeneratePoints,
              NeedsToCalculateEscape,
              NeedsToDraw,
              NotRunning
  };

private:  
  SDL_Window* window;
  SDL_Renderer* renderer;
  int window_width;
  int window_height;

  State state;
  
  std::complex<long double> plot_max;
  std::complex<long double> plot_min;
  long double plot_resolution;

  std::vector<std::pair<std::complex<long double>, unsigned int> > points;
  unsigned int escape_time_limit;

  bool gathering_new_limits;
  std::complex<long double> new_limit_1;
  std::complex<long double> new_limit_2;

  // run_plot helpers
  void handle_input();
  void update_plot();
  void generate_output();

  // input handling functions
  void handle_mouse_click(const SDL_Event& event);

  // plot management functions
  void gather_new_limits();
  void set_plot_limits(std::complex<long double> first, std::complex<long double> second);
  void reset_plot_resolution();
  long double determine_resolution();
  void generate_points();
  void calculate_escape_times();
  unsigned int iterate_point(const std::complex<long double>& c);

  // struct to represent an RGB color triple
  struct Color{
    Color(unsigned short rr=0, unsigned short gg=0, unsigned short bb=0) : r(rr), g(gg), b(bb) { }
    unsigned short r, g, b;
  };
  
  // drawing functions
  void render_point(std::pair<std::complex<long double>, unsigned int> point);
  Color calculate_color(const unsigned int escape_time);
  void alert_new_limits();

  // Functions to convert screen to plot coordinates and vice versa
  inline long double xtosdl(long double x);
  inline long double ytosdl(long double y);
  inline long double sdltox(long double sdlx);
  inline long double sdltoy(long double sdly);
};

#endif
