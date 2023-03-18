#include "map_renderer.h"

#include <cassert>
#include <sstream>

using namespace std;

namespace renderer {

using namespace transport_catalogue::detail;
using namespace svg;
using namespace geo;

RenderSettings &RenderSettings::SetWidth(double width) {
  this->width = width;
  return *this;
}

RenderSettings &RenderSettings::SetHeight(double height) {
  this->height = height;
  return *this;
}

RenderSettings &RenderSettings::SetPadding(double padding) {
  this->padding = padding;
  return *this;
}

RenderSettings &RenderSettings::SetLineWidth(double line_width) {
  this->line_width = line_width;
  return *this;
}

RenderSettings &RenderSettings::SetStopRadius(double stop_radius) {
  this->stop_radius = stop_radius;
  return *this;
}

RenderSettings &RenderSettings::SetBusLabelFontSize(int bus_label_font_size) {
  this->bus_label_font_size = bus_label_font_size;
  return *this;
}

RenderSettings &RenderSettings::SetBusLabelOffset(const svg::Point &bus_label_offset) {
  this->bus_label_offset = bus_label_offset;
  return *this;
}

RenderSettings &RenderSettings::SetStopLabelFontSize(int stop_label_font_size) {
  this->stop_label_font_size = stop_label_font_size;
  return *this;
}

RenderSettings &RenderSettings::SetStopLabelOffset(const Point &stop_label_offset) {
  this->stop_label_offset = stop_label_offset;
  return *this;
}

RenderSettings &RenderSettings::SetUnderlayerColor(Color underlayer_color) {
  this->underlayer_color = std::move(underlayer_color);
  return *this;
}

RenderSettings &RenderSettings::SetUnderlayerWidth(double underlayer_width) {
  this->underlayer_width = underlayer_width;
  return *this;
}

RenderSettings &RenderSettings::SetColorPalette(vector<Color> color_palette) {
  this->color_palette = std::move(color_palette);
  return *this;
}

MapRenderer::MapRenderer(RenderSettings &settings) : settings_(settings) {}

Document MapRenderer::RenderMap(const vector<const Bus *> &buses,
                                const unordered_map<string_view, const Stop *> &stops,
                                const vector<Coordinates> &stop_coords) const {
  Document document;
  SphereProjector sphere_projector{stop_coords.begin(), stop_coords.end(),
                                   settings_.width, settings_.height, settings_.padding};

  RenderBusLines(document, sphere_projector, buses, stops);

  return document;
}

void MapRenderer::RenderBusLines(Document &document,
                                 const SphereProjector &sphere_projector,
                                 const vector<const Bus *> &buses,
                                 const unordered_map<string_view, const Stop *> &stops) const {
  const size_t color_size = settings_.color_palette.size();
  assert(color_size);
  size_t color_index = 0;

  for (const auto bus : buses) {
    if (!bus->stops_on_route.empty()) {
      Polyline line;
      for (const auto stop_name : bus->stops_on_route) {
        Point stop = sphere_projector(stops.at(stop_name)->coordinates);
        line.AddPoint(stop);
      }
      if (bus->route_type == RouteType::LINEAR) {
        for (auto it = bus->stops_on_route.rbegin() + 1; it != bus->stops_on_route.rend(); ++it) {
          Point stop = sphere_projector(stops.at(*it)->coordinates);
          line.AddPoint(stop);
        }
      }

      line.SetStrokeColor(settings_.color_palette[color_index])
          .SetFillColor(NoneColor)
          .SetStrokeWidth(settings_.line_width)
          .SetStrokeLineCap(StrokeLineCap::ROUND)
          .SetStrokeLineJoin(StrokeLineJoin::ROUND);

      document.Add(line);
      color_index = (color_index + 1) % color_size;
    }
  }
}

}