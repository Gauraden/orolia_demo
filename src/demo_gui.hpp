#ifndef DEMO_GUI_HPP
#define DEMO_GUI_HPP

#include "collector/compressor.hpp"

struct GuiSettings {
  GuiSettings();
  bool draw_scales;
};

void CreateWindowWithChart(const Compressor::ShrPtr &comp_ptr,
                           const GuiSettings        &settings);
#endif