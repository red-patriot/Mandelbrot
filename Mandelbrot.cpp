#include "Mandelbrot.h"
#include <iostream>

using std::complex;        using std::pair;

Mandelbrot::Mandelbrot(int escape_limit) :
  window(nullptr),
  renderer(nullptr),
  state(NeedsToGeneratePoints),
  escape_time_limit(escape_limit),
  gathering_new_limits(false) { }

bool Mandelbrot::init(std::complex<double> max, std::complex<double> min,
                      double step, int width, int height) {
  /* Initialize nexeccary subsystems. */
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

  // intialize internal values
  plot_max = max;
  plot_min = min;
  plot_resolution = step;

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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    for (auto& point : points) {
      render_point(point);
    }

    SDL_RenderPresent(renderer);
    
    state = Running;
  }
}

void Mandelbrot::handle_mouse_click(const SDL_Event& event) {
  /* Respond to a mouse click. */
  switch (event.button.button) {
  case SDL_BUTTON_LEFT:
    int x, y;
    SDL_GetMouseState(&x, &y);
    std::cout << x << ' ' << y << '\n';
    /*if (!gathering_new_limits) {
      new_limit_1.real(x);
      new_limit_1.imag(y);
      gathering_new_limits = true;
    } else {
      new_limit_2.real(x);
      new_limit_2.imag(y);
      gathering_new_limits = false;

      // Set the new plot limits and generate new points
      
      }*/
    break;
  case SDL_BUTTON_RIGHT:
    break;
  case SDL_BUTTON_MIDDLE:
    break;
  }
}

void Mandelbrot::generate_points() {
  /* Generate a vector of points for the calculation. */
  if (state == NeedsToGeneratePoints) {
    for (double r = plot_min.real(); r <= plot_max.real(); r += plot_resolution) {
      for (double i = plot_min.imag(); i <= plot_max.imag(); i += plot_resolution) {
        complex<double> c{r, i};
        points.push_back(pair<complex<double>, unsigned int>(c, 0));
      }
    }
    state = NeedsToCalculateEscape;
  }
}

void Mandelbrot::calculate_escape_times() {
  /* Calculate the escape time for each point. */
  if (state == NeedsToCalculateEscape) {
    for (auto& point : points) {
      point.second = iterate_point(point.first);
    }
    state = NeedsToDraw;
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
  return e == 0 ? Color(0,0,0) :
    Color{static_cast<unsigned short>((e > 100 ? 7*e : 0)%255),
          static_cast<unsigned short>((e > 50 ? 4*e : 0)%255),
          static_cast<unsigned short>(5*e%255)};
}

double Mandelbrot::xtosdl(double x) {
  return (window_width * x)/(plot_max.real() - plot_min.real())
    - (window_width*plot_min.real())/(plot_max.real() - plot_min.real());
}

double Mandelbrot::ytosdl(double y){
  return (window_height * y)/(plot_min.imag() - plot_max.imag())
    - (window_height*plot_max.imag())/(plot_min.imag() - plot_max.imag());
}
