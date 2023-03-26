#include "map_renderer.h"

#include <cassert>
#include <sstream>
#include <execution>

using namespace std;

namespace renderer {

using namespace transport_catalogue::detail;
using namespace svg;
using namespace geo;

RenderSettings &RenderSettings::SetWidth(const double width) {
  this->width = width;
  return *this;
}

RenderSettings &RenderSettings::SetHeight(const double height) {
  this->height = height;
  return *this;
}

RenderSettings &RenderSettings::SetPadding(const double padding) {
  this->padding = padding;
  return *this;
}

RenderSettings &RenderSettings::SetLineWidth(const double line_width) {
  this->line_width = line_width;
  return *this;
}

RenderSettings &RenderSettings::SetStopRadius(const double stop_radius) {
  this->stop_radius = stop_radius;
  return *this;
}

RenderSettings &RenderSettings::SetBusLabelFontSize(const int bus_label_font_size) {
  this->bus_label_font_size = bus_label_font_size;
  return *this;
}

RenderSettings &RenderSettings::SetBusLabelOffset(const svg::Point &bus_label_offset) {
  this->bus_label_offset = bus_label_offset;
  return *this;
}

RenderSettings &RenderSettings::SetStopLabelFontSize(const int stop_label_font_size) {
  this->stop_label_font_size = stop_label_font_size;
  return *this;
}

RenderSettings &RenderSettings::SetStopLabelOffset(const Point &stop_label_offset) {
  this->stop_label_offset = stop_label_offset;
  return *this;
}

RenderSettings &RenderSettings::SetUnderlayerColor(Color &&underlayer_color) {
  this->underlayer_color = std::move(underlayer_color);
  return *this;
}

RenderSettings &RenderSettings::SetUnderlayerWidth(const double underlayer_width) {
  this->underlayer_width = underlayer_width;
  return *this;
}

RenderSettings &RenderSettings::SetColorPalette(vector<Color> &&color_palette) {
  this->color_palette = std::move(color_palette);
  return *this;
}

MapRenderer::MapRenderer(RenderSettings &&settings) : settings_(std::move(settings)) {}

vector<Coordinates> MapRenderer::GetStopCoords(const unordered_map<string_view, shared_ptr<Stop>> &stops) {
  vector<Coordinates> stop_coords;
  stop_coords.reserve(stops.size());
  for (const auto &[_, stop] : stops) {
    if (!stop->buses_through_stop.empty()) {
      stop_coords.emplace_back(stop->coordinates);
    }
  }
  return stop_coords;
}

vector<string_view> MapRenderer::GetStopNames(const unordered_map<string_view, shared_ptr<Stop>> &stops) {
  vector<string_view> stop_names(stops.size());
  std::transform(
      execution::par,
      stops.begin(),
      stops.end(),
      stop_names.begin(),
      [](const auto &item) {
        return item.first;
      });
  std::sort(execution::par, stop_names.begin(), stop_names.end());
  return stop_names;
}

Document MapRenderer::RenderMap(const vector<shared_ptr<Bus>> &buses,
                                const unordered_map<string_view, shared_ptr<Stop>> &stops) const {
  Document document;
  const auto &stop_names = GetStopNames(stops);
  const auto &stop_coords = GetStopCoords(stops);
  SphereProjector sphere_projector{stop_coords.begin(), stop_coords.end(),
                                   settings_.width, settings_.height, settings_.padding};

  RenderBusLines(document, sphere_projector, buses, stops);
  RenderBusNames(document, sphere_projector, buses, stops);
  RenderStopCircles(document, sphere_projector, stops, stop_names);
  RenderStopNames(document, sphere_projector, stops, stop_names);

  return document;
}

void MapRenderer::RenderBusLines(Document &document,
                                 const SphereProjector &sphere_projector,
                                 const vector<shared_ptr<Bus>> &buses,
                                 const unordered_map<string_view, shared_ptr<Stop>> &stops) const {
  const size_t color_size = settings_.color_palette.size();
  assert(color_size);
  size_t color_index = 0;

  for (const auto &bus : buses) {
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

void MapRenderer::RenderBusNames(Document &document,
                                 const SphereProjector &sphere_projector,
                                 const vector<shared_ptr<Bus>> &buses,
                                 const unordered_map<string_view, shared_ptr<Stop>> &stops) const {
  const size_t color_size = settings_.color_palette.size();
  assert(color_size);
  size_t color_index = 0;
  for (const auto &bus : buses) {
    vector<string_view> final_stops;
    final_stops.reserve(2);
    if (!bus->stops_on_route.empty()) {
      final_stops.emplace_back(bus->stops_on_route.front());
      if (bus->route_type == RouteType::LINEAR) {
        const auto last_stop = bus->stops_on_route.back();
        if (final_stops[0] != last_stop) {
          final_stops.emplace_back(last_stop);
        }
      }
      for (const auto final_stop : final_stops) {
        Point stop_point = sphere_projector(stops.at(final_stop)->coordinates);
        document.Add(Text()
                         .SetData(bus->name)
                         .SetFillColor(settings_.underlayer_color)
                         .SetStrokeColor(settings_.underlayer_color)
                         .SetStrokeWidth(settings_.underlayer_width)
                         .SetStrokeLineCap(StrokeLineCap::ROUND)
                         .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                         .SetPosition(stop_point)
                         .SetOffset(settings_.bus_label_offset)
                         .SetFontSize(settings_.bus_label_font_size)
                         .SetFontFamily("Verdana"s)
                         .SetFontWeight("bold"s));
        document.Add(Text()
                         .SetData(bus->name)
                         .SetFillColor(settings_.color_palette[color_index])
                         .SetPosition(stop_point)
                         .SetOffset(settings_.bus_label_offset)
                         .SetFontSize(settings_.bus_label_font_size)
                         .SetFontFamily("Verdana"s)
                         .SetFontWeight("bold"s));
      }
      color_index = (color_index + 1) % color_size;
    }
  }
}

void MapRenderer::RenderStopCircles(Document &document,
                                    const SphereProjector &sphere_projector,
                                    const unordered_map<string_view, shared_ptr<Stop>> &stops,
                                    const vector<string_view> &stop_names) const {
  for (const auto stop_name : stop_names) {
    const auto &stop = stops.at(stop_name);
    if (!stop->buses_through_stop.empty()) {
      document.Add(Circle()
                       .SetCenter(sphere_projector(stop->coordinates))
                       .SetRadius(settings_.stop_radius)
                       .SetFillColor("white"s));
    }
  }
}

void MapRenderer::RenderStopNames(Document &document,
                                  const SphereProjector &sphere_projector,
                                  const unordered_map<string_view, shared_ptr<Stop>> &stops,
                                  const vector<string_view> &stop_names) const {
  for (const auto stop_name : stop_names) {
    const auto &stop = stops.at(stop_name);
    if (!stop->buses_through_stop.empty()) {
      Point stop_point = sphere_projector(stops.at(stop_name)->coordinates);
      document.Add(Text()
                       .SetData(stop->name)
                       .SetFillColor(settings_.underlayer_color)
                       .SetStrokeColor(settings_.underlayer_color)
                       .SetStrokeWidth(settings_.underlayer_width)
                       .SetStrokeLineCap(StrokeLineCap::ROUND)
                       .SetStrokeLineJoin(StrokeLineJoin::ROUND)
                       .SetPosition(stop_point)
                       .SetOffset(settings_.stop_label_offset)
                       .SetFontSize(settings_.stop_label_font_size)
                       .SetFontFamily("Verdana"s));
      document.Add(Text()
                       .SetData(stop->name)
                       .SetFillColor("black"s)
                       .SetPosition(stop_point)
                       .SetOffset(settings_.stop_label_offset)
                       .SetFontSize(settings_.stop_label_font_size)
                       .SetFontFamily("Verdana"s));
    }
  }
}
}