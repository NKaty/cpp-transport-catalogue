#include "json_reader.h"

#include <string>
#include <vector>

using namespace std;

namespace request {

using namespace json;
using namespace transport_catalogue::detail;
using namespace svg;

const string BUS = "Bus"s;
const string STOP = "Stop"s;

Bus ParseBusInput(const Dict &request) {
  Bus bus;
  bus.name = request.at("name"s).AsString();
  bus.route_type = request.at("is_roundtrip"s).AsBool() ? RouteType::CIRCULAR : RouteType::LINEAR;
  const auto &stops = request.at("stops"s).AsArray();
  bus.stops_on_route.reserve(stops.size());
  for (const auto &stop : stops) {
    bus.stops_on_route.emplace_back(stop.AsString());
  }
  return bus;
}

Stop ParseStopInput(const Dict &request) {
  Stop stop;
  stop.name = request.at("name"s).AsString();
  stop.coordinates.lat = request.at("latitude"s).AsDouble();
  stop.coordinates.lng = request.at("longitude"s).AsDouble();
  return stop;
}

void AddTransportCatalogueData(transport_catalogue::TransportCatalogue &transport_catalogue,
                               const Array &requests) {
  vector<size_t> buses;
  vector<size_t> distances;

  for (size_t i = 0; i < requests.size(); ++i) {
    const auto request = requests[i].AsMap();
    if (request.at("type"s) == BUS) {
      buses.emplace_back(i);
    } else if (request.at("type"s) == STOP) {
      transport_catalogue.AddStop(ParseStopInput(request));
      if (!request.at("road_distances"s).AsMap().empty()) {
        distances.emplace_back(i);
      }
    }
  }

  for (const auto index : distances) {
    const auto stop_info = requests[index].AsMap();
    for (const auto &[key, value] : stop_info.at("road_distances"s).AsMap()) {
      transport_catalogue.AddDistance({stop_info.at("name"s).AsString(), key, value.AsInt()});
    }
  }

  for (const auto index : buses) {
    transport_catalogue.AddBus(ParseBusInput(requests[index].AsMap()));
  }
}

Node GetErrorJson(const Node &id) {
  Dict result;
  result["request_id"s] = id;
  result["error_message"s] = "not found"s;
  return result;
}

Node GetBusStatJson(const Node &id, const RouteStat &route_stat) {
  Dict result;
  result["curvature"s] = route_stat.curvature;
  result["request_id"s] = id;
  result["route_length"s] = route_stat.route_distance;
  result["stop_count"s] = static_cast<int>(route_stat.stops_count);
  result["unique_stop_count"s] = static_cast<int>(route_stat.unique_stops_count);
  return result;
}

Node GetStopStatJson(const Node &id, const set<string_view> &stop_stat) {
  Dict result;
  vector<string> stop_stat_{stop_stat.begin(), stop_stat.end()};
  result["buses"s] = Array{stop_stat_.begin(), stop_stat_.end()};
  result["request_id"s] = id;
  return result;
}

Node GetTransportCatalogueStats(const RequestHandler &request_handler, const Array &requests) {
  json::Array result;
  result.reserve(requests.size());
  for (const auto &request : requests) {
    const auto &request_map = request.AsMap();
    const auto id = request_map.at("id"s);
    if (request_map.at("type"s) == BUS) {
      const auto route_stat = request_handler.GetRouteStat(request_map.at("name"s).AsString());
      result.emplace_back(route_stat ? GetBusStatJson(id, *route_stat) : GetErrorJson(id));
    } else if (request_map.at("type"s) == STOP) {
      const auto stops_stat =
          request_handler.GetBusesThroughStop(request_map.at("name"s).AsString());
      result.emplace_back(stops_stat ? GetStopStatJson(id, *stops_stat) : GetErrorJson(id));
    }
  }
  return result;
}

Point GetOffset(const Array &offset) {
  return {offset[0].AsDouble(), offset[1].AsDouble()};
}

Color GetColor(const Node &color) {
  if (color.IsString()) {
    return color.AsString();
  }
  const auto &color_arr = color.AsArray();
  if (color_arr.size() == 3) {
    return Rgb(color_arr[0].AsInt(), color_arr[1].AsInt(), color_arr[2].AsInt());
  }
  return Rgba(color_arr[0].AsInt(),
              color_arr[1].AsInt(),
              color_arr[2].AsInt(),
              color_arr[3].AsDouble());
}

vector<Color> GetColorPalette(const Array &colors) {
  vector<Color> color_palette(colors.size());
  std::transform(
      execution::par,
      colors.begin(), colors.end(),
      color_palette.begin(),
      [&](const Node &node) {
        return GetColor(node);
      });
  return color_palette;
}

renderer::RenderSettings GetMapSettings(const Dict &request) {
  renderer::RenderSettings settings;
  settings.SetWidth(request.at("width"s).AsDouble())
      .SetHeight(request.at("height"s).AsDouble())
      .SetPadding(request.at("padding"s).AsDouble())
      .SetLineWidth(request.at("line_width"s).AsDouble())
      .SetStopRadius(request.at("stop_radius"s).AsDouble())
      .SetBusLabelFontSize(request.at("bus_label_font_size"s).AsInt())
      .SetBusLabelOffset(GetOffset(request.at("bus_label_offset"s).AsArray()))
      .SetStopLabelFontSize(request.at("stop_label_font_size"s).AsInt())
      .SetStopLabelOffset(GetOffset(request.at("stop_label_offset"s).AsArray()))
      .SetUnderlayerColor(GetColor(request.at("underlayer_color"s)))
      .SetUnderlayerWidth(request.at("underlayer_width"s).AsDouble())
      .SetColorPalette(GetColorPalette(request.at("color_palette"s).AsArray()));
  return settings;
}

void ProcessJsonRequests(transport_catalogue::TransportCatalogue &transport_catalogue,
                         istream &input,
                         ostream &output) {
  const auto json_input = Load(input).GetRoot();
  const auto &base_requests = json_input.AsMap().at("base_requests"s).AsArray();
//  const auto &stat_requests = json_input.AsMap().at("stat_requests"s).AsArray();
  const auto &render_settings = json_input.AsMap().at("render_settings"s).AsMap();

  auto map_settings = GetMapSettings(render_settings);
  auto map_render = renderer::MapRenderer(map_settings);
  RequestHandler request_handler(transport_catalogue, map_render);
  AddTransportCatalogueData(transport_catalogue, base_requests);
  request_handler.RenderMap().Render(output);
//  Print(Document(GetTransportCatalogueStats(request_handler, stat_requests)), output);
}

}