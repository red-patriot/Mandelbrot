#include "Mandelbrot.h"

#include <iostream>
#include <iomanip>
#include <algorithm>

using std::complex;        using std::pair;

Mandelbrot::Mandelbrot(int escape_limit) :
  window(nullptr),
  renderer(nullptr),
  state(NeedsToGeneratePoints),
  x_err(0),
  y_err(0),
  escape_time_limit(escape_limit),
  gathering_new_limits(false) { }

bool Mandelbrot::init(std::complex<double> max, std::complex<double> min,
                      int width, int height) {
  /* Initialize necessary subsystems. */
  // Initialize SDL systems and objects
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    return false;
  }

  window_width = width;
  window_height = height;
  window = SDL_CreateWindow("Mandelbrot",
                            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                            window_width, window_height, 0);
  if (window == nullptr) {
    SDL_Log("Failed to create window: %s", SDL_GetError());
    return false;
  }

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    SDL_Log("Failed to create renderer: %s", SDL_GetError());
    return false;
  }

  // set plot limits
  plot_max = max;
  plot_min = min;

  // plot step depends on the
  plot_resolution = determine_resolution();

  generate_points();
  std::cout << points.size() << '\n';
  
  return true;
}

void Mandelbrot::run_plot() {
  while(state != NotRunning) {
    handle_input();
    update_plot();
    generate_output();
  }
}

void Mandelbrot::handle_input() {
  /* Process user input. */
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      state = NotRunning;
      break;
    case SDL_MOUSEBUTTONDOWN:
      handle_mouse_click(event);
      break;
    default:
      break;
    }   
  }

  const Uint8* keystate = SDL_GetKeyboardState(NULL);
  if (keystate[SDL_SCANCODE_ESCAPE]) {
    state = NotRunning;
  } 
}

void Mandelbrot::update_plot() {
    generate_points();
    calculate_escape_times();
}

void Mandelbrot::generate_output() {
  /* Generate the display. */
  if (state == NeedsToDraw) {
    std::cout << "start_go\n";
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (auto& point : points) {
      render_point(point);
    }

    SDL_RenderPresent(renderer);
    
    state = Running;
    std::cout << "generate_output\n";
  }
}

void Mandelbrot::handle_mouse_click(const SDL_Event& event) {
  /* Respond to a mouse click. */
  switch (event.button.button) {
  case SDL_BUTTON_LEFT:
    gather_new_limits();
    break;
  case SDL_BUTTON_RIGHT:
    // TODO: Make this button cancel setting the new limits
    break;
  case SDL_BUTTON_MIDDLE:
    break;
  }
}

void Mandelbrot::gather_new_limits() {
  int x, y;
  SDL_GetMouseState(&x, &y);
  if (!gathering_new_limits) {
    new_limit_1.real(sdltox(x));
    new_limit_1.imag(sdltoy(y));
    gathering_new_limits = true;
  } else {
    new_limit_2.real(sdltox(x));
    new_limit_2.imag(sdltoy(y));
    gathering_new_limits = false;

    alert_new_limits();    
    set_plot_limits(new_limit_1, new_limit_2);
    reset_plot_resolution();
  }
}

void Mandelbrot::set_plot_limits(std::complex<double> first,
                                 std::complex<double> second) {
  /* Set new plot limits from two points defining the new bounding rectangle.
   * The new points don't have to be ordered, so maxs and mins are determined.
   */
  plot_max.real(std::max(first.real(), second.real()));
  plot_max.imag(std::max(first.imag(), second.imag()));
  plot_min.real(std::min(first.real(), second.real()));
  plot_min.imag(std::min(first.imag(), second.imag()));
}

void Mandelbrot::reset_plot_resolution() {
  /* Set a new plot resolution and ready the plot for recalculation. */
  double new_plot_resolution = determine_resolution();
  points.clear();
  state = NeedsToGeneratePoints;
  escape_time_limit *= 2;
  plot_resolution = new_plot_resolution;
}

double Mandelbrot::determine_resolution() {
  /* Determine the plot resolution based on the window and plot dimensions. */
  return std::min((plot_max.real() - plot_min.real())/window_width,
                  (plot_max.imag() - plot_min.imag())/window_height);
}

void Mandelbrot::generate_points() {
  /* Generate a vector of points for the calculation. */
  if (state == NeedsToGeneratePoints) {
    std::cout << "start_gp\n";
    for (double r = plot_min.real(); r <= plot_max.real(); r += plot_resolution) {
      for (double i = plot_min.imag(); i <= plot_max.imag(); i += plot_resolution) {
        complex<double> c{r, i};
        points.push_back(pair<complex<double>, unsigned int>(c, 0));
      }
    }
    state = NeedsToCalculateEscape;
    std::cout << "generate_points\n";
  }
}

void Mandelbrot::calculate_escape_times() {
  /* Calculate the escape time for each point. */
  if (state == NeedsToCalculateEscape) {
    std::cout << "start_cet\n";
    for (auto& point : points) {
      point.second = iterate_point(point.first);
    }
    state = NeedsToDraw;
    std::cout << "calculate_escape_times\n";
  }
}

unsigned int Mandelbrot::iterate_point(const std::complex<double>& c) {
  /* Iterate over a single point to determine how long it takes to excape. */
  complex<double> _c{c.real(), c.imag()};
  complex<double> z{0.0, 0.0};
  unsigned int count = 1;

  // check if the point is within the cardiod or period-2 bulb first
  double x{c.real()}, y{c.imag()};
  double q = (x-.25)*(x-.25) + y*y;
  if ((q*(q + (x-.25))) <= (.25*y*y) || ((x+1)*(x+1) + y*y) <= .0625) {
    return 0;
  }

  while (count < escape_time_limit) {
    z = z*z + _c;
    // check if z has escaped the bounds of the set
    if (z.real()*z.real() + z.imag()*z.imag() >= 4.0){
      break;
    }
    ++count;
  }
  
  return count == escape_time_limit ? 0 : count;
}

void Mandelbrot::render_point(std::pair<std::complex<double>, unsigned int> point) {
  /* Render a point on the plot. */
  Color c = calculate_color(point.second);
  SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255);
  SDL_RenderDrawPoint(renderer, xtosdl(point.first.real()),
                      ytosdl(point.first.imag()));
}

Mandelbrot::Color Mandelbrot::calculate_color(const unsigned int e) {
  /* Calculate the color of a point based on its escape_time. */
  return e == 0 ? Color(0,0,0) : Color(175, 175, 175);
  /*    Color{static_cast<unsigned short>((e > 200 ? 10*e : 0)%255),
          static_cast<unsigned short>((e > 100 ? 14*e : 0)%255),
          static_cast<unsigned short>(7*e%255)};*/
}

void Mandelbrot::alert_new_limits() {
  /* Draw a rectangle at the new limits to show them. */
  SDL_Rect new_plot_area;
  new_plot_area.x = xtosdl(std::min(new_limit_1.real(), new_limit_2.real()));
  new_plot_area.y = ytosdl(std::max(new_limit_1.imag(), new_limit_2.imag()));
  new_plot_area.w = fabs(xtosdl(new_limit_1.real()) - xtosdl(new_limit_2.real()));
  new_plot_area.h = fabs(ytosdl(new_limit_1.imag()) - ytosdl(new_limit_2.imag()));

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderDrawRect(renderer, &new_plot_area);
  SDL_RenderPresent(renderer);
}

inline double Mandelbrot::xtosdl(double x) {
  double m = (window_width - 2*x_err)/(plot_max.real() - plot_min.real());
  return m*x - m*plot_min.real() + x_err;
}

inline double Mandelbrot::ytosdl(double y){
  double m = (window_height - 2*y_err)/(plot_min.imag() - plot_max.imag());
  return m*y -m*plot_max.imag() + y_err;
}

inline double Mandelbrot::sdltox(double sdlx) {
  double m = (plot_max.real() - plot_min.real())/(window_width - 2*x_err);
  return m*sdlx - m*x_err + plot_min.real();
}

inline double Mandelbrot::sdltoy(double sdly) {
  double m = (plot_min.imag() - plot_max.imag())/(window_height - 2*y_err);
  return m*sdly - m*y_err + plot_max.imag();
}
