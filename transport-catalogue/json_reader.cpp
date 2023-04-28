#include "json_reader.h"
#include "json_builder.h"

#include <string>
#include <vector>

using namespace std;

namespace request {

using namespace json;
using namespace transport_catalogue::detail;
using namespace svg;
using namespace routing;

JsonReader::JsonReader(transport_catalogue::TransportCatalogue &transport_catalogue)
    : transport_catalogue_(transport_catalogue) {}

ParsedRequests JsonReader::GetParsedRequests(istream &input) {
  const auto json_input = Load(input).GetRoot();
  const auto &base_requests = json_input.AsMap().at("base_requests"s).AsArray();
  const auto &stat_requests = json_input.AsMap().at("stat_requests"s).AsArray();
  const auto &render_settings = json_input.AsMap().at("render_settings"s).AsMap();
  const auto &routing_settings = json_input.AsMap().at("routing_settings"s).AsMap();
  return {base_requests, stat_requests, render_settings, routing_settings};
}

Bus JsonReader::ParseBusInput(const Dict &request) {
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

Stop JsonReader::ParseStopInput(const Dict &request) {
  Stop stop;
  stop.name = request.at("name"s).AsString();
  stop.coordinates.lat = request.at("latitude"s).AsDouble();
  stop.coordinates.lng = request.at("longitude"s).AsDouble();
  return stop;
}

void JsonReader::AddTransportCatalogueData(const Array &requests) {
  vector<size_t> buses;
  vector<size_t> distances;

  for (size_t i = 0; i < requests.size(); ++i) {
    const auto request = requests[i].AsMap();
    if (request.at("type"s) == BUS) {
      buses.emplace_back(i);
    } else if (request.at("type"s) == STOP) {
      transport_catalogue_.AddStop(ParseStopInput(request));
      if (!request.at("road_distances"s).AsMap().empty()) {
        distances.emplace_back(i);
      }
    }
  }

  for (const auto index : distances) {
    const auto stop_info = requests[index].AsMap();
    for (const auto &[key, value] : stop_info.at("road_distances"s).AsMap()) {
      transport_catalogue_.AddDistance({stop_info.at("name"s).AsString(), key, value.AsInt()});
    }
  }

  for (const auto index : buses) {
    transport_catalogue_.AddBus(ParseBusInput(requests[index].AsMap()));
  }
}

Node JsonReader::GetErrorJson(int id) {
  return Builder{}
      .StartDict()
      .Key("request_id"s).Value(id)
      .Key("error_message"s).Value("not found"s)
      .EndDict()
      .Build();
}

Node JsonReader::GetBusStatJson(int id, const RouteStat &route_stat) {
  return Builder{}
      .StartDict()
      .Key("curvature"s).Value(route_stat.curvature)
      .Key("request_id"s).Value(id)
      .Key("route_length"s).Value(route_stat.route_distance)
      .Key("stop_count"s).Value(static_cast<int>(route_stat.stops_count))
      .Key("unique_stop_count"s).Value(static_cast<int>(route_stat.unique_stops_count))
      .EndDict()
      .Build();
}

Node JsonReader::GetBusStatJson(int id, const optional<RouteStat> &route_stat) {
  return route_stat ? GetBusStatJson(id, *route_stat) : GetErrorJson(id);
}

Node JsonReader::GetStopStatJson(int id, const set<string_view> &stop_stat) {
  Dict result;
  vector<string> stop_stat_{stop_stat.begin(), stop_stat.end()};
  return Builder{}
      .StartDict()
      .Key("buses"s).Value(Array{stop_stat_.begin(), stop_stat_.end()})
      .Key("request_id"s).Value(id)
      .EndDict()
      .Build();
}

Node JsonReader::GetStopStatJson(int id, unique_ptr<set<string_view>> &&stops_stat) {
  return stops_stat ? GetStopStatJson(id, *stops_stat) : GetErrorJson(id);
}

Node JsonReader::GetMapStatJson(int id, const string &map_stat) {
  return Builder{}
      .StartDict()
      .Key("map"s).Value(map_stat)
      .Key("request_id"s).Value(id)
      .EndDict()
      .Build();
}

struct RouteItemJson {
  Builder &json_builder;
  void operator()(const WaitRouteItem &route_item) const {
    json_builder
        .StartDict()
        .Key("stop_name"s).Value(route_item.stop)
        .Key("time"s).Value(route_item.time)
        .Key("type"s).Value(route_item.type)
        .EndDict();
  }
  void operator()(const BusRouteItem &route_item) const {
    json_builder
        .StartDict()
        .Key("bus").Value(route_item.bus)
        .Key("span_count"s).Value(route_item.span_count)
        .Key("time"s).Value(route_item.time)
        .Key("type"s).Value(route_item.type)
        .EndDict();
  }
};

Node JsonReader::GetRouteItems(const vector<RouteItem> &route_items) {
  Builder json_builder;
  json_builder.StartArray();
  for (const auto &item : route_items) {
    visit(RouteItemJson{json_builder}, item);
  }
  json_builder.EndArray();
  return json_builder.Build();
}

Node JsonReader::GetRouteStatJson(int id, const RouteData &route_info) {
  return Builder{}
      .StartDict()
      .Key("request_id"s).Value(id)
      .Key("total_time"s).Value(route_info.total_time)
      .Key("items"s).Value(GetRouteItems(route_info.items))
      .EndDict()
      .Build();
}

Node JsonReader::GetRouteStatJson(int id, const optional<RouteData> &route_info) {
  return route_info ? GetRouteStatJson(id, *route_info) : GetErrorJson(id);
}

vector<Request> JsonReader::GetTransportCatalogueRequests(const Array &requests) {
  vector<Request> result;
  result.reserve(requests.size());
  for (const auto &request : requests) {
    const auto &request_map = request.AsMap();
    Request req;
    req.id = request_map.at("id"s).AsInt();
    req.type = request_map.at("type"s).AsString();
    if (request_map.find("name"s) != request_map.end()) {
      req.name = request_map.at("name"s).AsString();
    }
    if (request_map.find("from"s) != request_map.end()) {
      req.from = request_map.at("from"s).AsString();
    }
    if (request_map.find("to"s) != request_map.end()) {
      req.to = request_map.at("to"s).AsString();
    }
    result.push_back(req);
  }
  return result;
}

Point JsonReader::GetOffset(const Array &offset) {
  return {offset[0].AsDouble(), offset[1].AsDouble()};
}

Color JsonReader::GetColor(const Node &color) {
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

vector<Color> JsonReader::GetColorPalette(const Array &colors) {
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

renderer::RenderSettings JsonReader::GetMapSettings(const Dict &request) {
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

RoutingSettings JsonReader::GetRoutingSettings(const Dict &requests) {
  return {requests.at("bus_velocity"s).AsDouble(),
          requests.at("bus_wait_time"s).AsInt()};
}

}