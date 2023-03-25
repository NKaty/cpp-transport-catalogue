#pragma once

#include "json.h"
#include "request_handler.h"

#include <iostream>
#include <set>
#include <string_view>
#include <unordered_map>
#include <memory>

namespace request {

struct Request {
  int id;
  std::string type;
  std::string name;
};

class JsonReader {
 public:
  explicit JsonReader(transport_catalogue::TransportCatalogue &transport_catalogue);

  void AddTransportCatalogueData(const json::Array &requests);

  static json::Node GetBusStatJson(int id,
                                   const std::optional<transport_catalogue::detail::RouteStat> &route_stat);

  static json::Node GetStopStatJson(int id,
                                    std::unique_ptr<std::set<std::string_view>> &&stops_stat);

  static json::Node GetMapStatJson(int id, const std::string &map_stat);

  static std::vector<Request> GetTransportCatalogueRequests(const json::Array &requests);

  static renderer::RenderSettings GetMapSettings(const json::Dict &request);

  inline static const std::string BUS = "Bus"s;
  inline static const std::string STOP = "Stop"s;
  inline static const std::string MAP = "Map"s;

 private:
  transport_catalogue::TransportCatalogue &transport_catalogue_;

  static transport_catalogue::detail::Bus ParseBusInput(const json::Dict &request);

  static transport_catalogue::detail::Stop ParseStopInput(const json::Dict &request);

  static json::Node GetErrorJson(int id);

  static json::Node GetBusStatJson(int id,
                                   const transport_catalogue::detail::RouteStat &route_stat);

  static json::Node GetStopStatJson(int id, const std::set<std::string_view> &stop_stat);

  static svg::Point GetOffset(const json::Array &offset);

  static svg::Color GetColor(const json::Node &color);

  static std::vector<svg::Color> GetColorPalette(const json::Array &colors);
};

}
