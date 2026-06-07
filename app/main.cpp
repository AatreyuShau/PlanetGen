#include "planetgen/Planet.hpp"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

namespace {

struct Rgb {
  double r{};
  double g{};
  double b{};
};

struct ViewerState {
  double yaw = 0.0;
  double pitch = -0.28;
  double oceanHeight = 0.0;
  double terrainScale = 1.0;
  bool showOcean = true;
  bool dragging = false;
  int lastMouseX = 0;
  int lastMouseY = 0;
};

std::uint32_t pack(Rgb color) {
  const auto channel = [](double value) {
    return static_cast<std::uint32_t>(planetgen::clamp(value, 0.0, 1.0) *
                                      255.0);
  };
  return (channel(color.r) << 16U) | (channel(color.g) << 8U) |
         channel(color.b);
}

Rgb mix(Rgb a, Rgb b, double t) {
  t = planetgen::clamp(t, 0.0, 1.0);
  return {
      a.r + ((b.r - a.r) * t),
      a.g + ((b.g - a.g) * t),
      a.b + ((b.b - a.b) * t),
  };
}

Rgb shade(Rgb base, const planetgen::Vec3 &normal, double ambient = 0.26) {
  const planetgen::Vec3 light = planetgen::normalize({-0.35, 0.55, 0.75});
  const double diffuse = std::max(0.0, planetgen::dot(normal, light));
  const double rim = std::pow(std::max(0.0, 1.0 - normal.z), 2.0) * 0.10;
  const double intensity = ambient + (diffuse * 0.82) + rim;
  return {base.r * intensity, base.g * intensity, base.b * intensity};
}

Rgb terrainColor(const planetgen::TerrainSample &sample, double oceanHeight) {
  const double h = sample.elevation - oceanHeight;
  if (h < 0.012) {
    return mix({0.75, 0.66, 0.39}, {0.24, 0.53, 0.22}, h / 0.012);
  }
  if (sample.mountainMask > 0.55) {
    const double snow = planetgen::smoothstep(0.055, 0.12, sample.elevation);
    return mix({0.32, 0.30, 0.27}, {0.92, 0.92, 0.88}, snow);
  }

  const Rgb dry{0.50, 0.43, 0.25};
  const Rgb lush{0.10, 0.38, 0.16};
  const Rgb biome = mix(dry, lush, sample.moisture);
  return mix({0.20, 0.47, 0.19}, biome, planetgen::smoothstep(0.02, 0.08, h));
}

Rgb oceanColor(const planetgen::TerrainSample &sample, double oceanHeight) {
  const double depth =
      planetgen::clamp((oceanHeight - sample.elevation) / 0.16, 0.0, 1.0);
  return mix({0.04, 0.35, 0.55}, {0.01, 0.05, 0.19}, depth);
}

void rebuildPlanet(planetgen::Planet &planet, const ViewerState &state) {
  planetgen::PlanetSettings settings;
  settings.oceanHeight = state.oceanHeight;
  settings.terrainScale = state.terrainScale;
  settings.seed = 271828U;
  planet = planetgen::Planet(settings);
}

void render(std::vector<std::uint32_t> &pixels, int width, int height,
            planetgen::Planet &planet, const ViewerState &state) {
  const double aspect =
      static_cast<double>(width) / static_cast<double>(height);
  const double invMin = 1.0 / static_cast<double>(std::min(width, height));

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const double sx = ((static_cast<double>(x) + 0.5) -
                         (static_cast<double>(width) * 0.5)) *
                        2.15 * invMin;
      const double sy = ((static_cast<double>(height) * 0.5) -
                         (static_cast<double>(y) + 0.5)) *
                        2.15 * invMin;
      const double radius2 = (sx * sx) + (sy * sy);

      Rgb color{0.005 + (0.015 * (1.0 - (static_cast<double>(y) / height))),
                0.007, 0.018 + (0.030 * (static_cast<double>(y) / height))};

      if (radius2 <= 1.0) {
        const double z = std::sqrt(1.0 - radius2);
        planetgen::Vec3 normal = planetgen::normalize({sx, sy, z});
        const planetgen::Vec3 planetDirection =
            planetgen::rotateYawPitch(normal, state.yaw, state.pitch);
        const planetgen::TerrainSample sample = planet.sample(planetDirection);

        const bool flooded =
            state.showOcean && sample.elevation <= state.oceanHeight;
        const Rgb base = flooded ? oceanColor(sample, state.oceanHeight)
                                 : terrainColor(sample, state.oceanHeight);
        color = shade(base, normal, flooded ? 0.34 : 0.25);

        const double atmosphere = std::pow(std::max(0.0, 1.0 - z), 2.5);
        color = mix(color, {0.23, 0.48, 0.85},
                    atmosphere * (state.showOcean ? 0.28 : 0.10));
      } else if (aspect > 0.0 && ((x * 37 + y * 91) % 997) == 0) {
        color = {0.65, 0.68, 0.82};
      }

      pixels[static_cast<std::size_t>(y * width + x)] = pack(color);
    }
  }
}

std::string title(const ViewerState &state) {
  return "PlanetGen - drag rotate | O ocean " +
         std::string(state.showOcean ? "on" : "off") +
         " | ocean=" + std::to_string(state.oceanHeight).substr(0, 5) +
         " | terrain=" + std::to_string(state.terrainScale).substr(0, 4);
}

} // namespace

int main() {
  constexpr int width = 960;
  constexpr int height = 720;

  Display *display = XOpenDisplay(nullptr);
  if (display == nullptr) {
    std::cerr << "PlanetGen viewer needs an X11 display. Build succeeded, but "
                 "DISPLAY is not available.\n";
    return EXIT_FAILURE;
  }

  const int screen = DefaultScreen(display);
  Window window = XCreateSimpleWindow(
      display, RootWindow(display, screen), 0, 0, width, height, 1,
      BlackPixel(display, screen), BlackPixel(display, screen));

  XSelectInput(display, window,
               ExposureMask | KeyPressMask | ButtonPressMask |
                   ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);
  XMapWindow(display, window);

  GC gc = XCreateGC(display, window, 0, nullptr);
  Visual *visual = DefaultVisual(display, screen);
  const int depth = DefaultDepth(display, screen);

  std::vector<std::uint32_t> pixels(static_cast<std::size_t>(width * height));
  XImage *image = XCreateImage(
      display, visual, static_cast<unsigned int>(depth), ZPixmap, 0,
      reinterpret_cast<char *>(pixels.data()), width, height, 32, 0);

  if (image == nullptr) {
    std::cerr << "Failed to create X11 image.\n";
    XFreeGC(display, gc);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
    return EXIT_FAILURE;
  }

  ViewerState state;
  planetgen::Planet planet;
  rebuildPlanet(planet, state);
  bool running = true;
  bool dirty = true;

  std::cout << "PlanetGen controls: drag rotate, O toggle ocean, +/- ocean "
               "height, [] terrain scale, R reset, Q/Esc quit.\n";

  while (running) {
    while (XPending(display) > 0) {
      XEvent event{};
      XNextEvent(display, &event);

      if (event.type == KeyPress) {
        const KeySym key = XLookupKeysym(&event.xkey, 0);
        if (key == XK_Escape || key == XK_q || key == XK_Q) {
          running = false;
        } else if (key == XK_o || key == XK_O) {
          state.showOcean = !state.showOcean;
          dirty = true;
        } else if (key == XK_plus || key == XK_equal) {
          state.oceanHeight += 0.01;
          dirty = true;
        } else if (key == XK_minus || key == XK_underscore) {
          state.oceanHeight -= 0.01;
          dirty = true;
        } else if (key == XK_bracketright) {
          state.terrainScale = std::min(2.5, state.terrainScale + 0.1);
          rebuildPlanet(planet, state);
          dirty = true;
        } else if (key == XK_bracketleft) {
          state.terrainScale = std::max(0.1, state.terrainScale - 0.1);
          rebuildPlanet(planet, state);
          dirty = true;
        } else if (key == XK_r || key == XK_R) {
          state = ViewerState{};
          rebuildPlanet(planet, state);
          dirty = true;
        }
      } else if (event.type == ButtonPress && event.xbutton.button == Button1) {
        state.dragging = true;
        state.lastMouseX = event.xbutton.x;
        state.lastMouseY = event.xbutton.y;
      } else if (event.type == ButtonRelease &&
                 event.xbutton.button == Button1) {
        state.dragging = false;
      } else if (event.type == MotionNotify && state.dragging) {
        const int dx = event.xmotion.x - state.lastMouseX;
        const int dy = event.xmotion.y - state.lastMouseY;
        state.lastMouseX = event.xmotion.x;
        state.lastMouseY = event.xmotion.y;
        state.yaw += static_cast<double>(dx) * 0.012;
        state.pitch = planetgen::clamp(
            state.pitch + (static_cast<double>(dy) * 0.012), -1.45, 1.45);
        dirty = true;
      } else if (event.type == Expose) {
        dirty = true;
      } else if (event.type == DestroyNotify) {
        running = false;
      }
    }

    if (dirty) {
      XStoreName(display, window, title(state).c_str());
      render(pixels, width, height, planet, state);
      XPutImage(display, window, gc, image, 0, 0, 0, 0, width, height);
      XFlush(display);
      dirty = false;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(8));
  }

  image->data = nullptr;
  XDestroyImage(image);
  XFreeGC(display, gc);
  XDestroyWindow(display, window);
  XCloseDisplay(display);
  return EXIT_SUCCESS;
}
