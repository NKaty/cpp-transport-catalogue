#pragma once

#include "json.h"
#include "request_handler.h"
#include "json_builder.h"
#include "serialization.h"

#include <iostream>
#include <set>
#include <string_view>
#include <string>
#include <unordered_map>
#include <map>
#include <memory>

namespace request {

struct Request {
  int id;
  std::string type;
  std::string name;
  std::string from;
  std::string to;
};

//struct ParsedRequests {
//  std::vector<json::Node> base_requests;
//  std::vector<json::Node> stat_requests;
//  std::map<std::string, json::Node> render_settings;
//  std::map<std::string, json::Node> routing_settings;
//  std::map<std::string, json::Node> serialization_settings;
//};

struct ParsedBaseRequests {
  std::vector<json::Node> base_requests;
  std::map<std::string, json::Node> render_settings;
  std::map<std::string, json::Node> routing_settings;
  std::map<std::string, json::Node> serialization_settings;
};

struct ParsedStatRequests {
  std::vector<json::Node> stat_requests;
  std::map<std::string, json::Node> serialization_settings;
};

class JsonReader {
 public:
  explicit JsonReader(transport_catalogue::TransportCatalogue &transport_catalogue);

//  static ParsedRequests GetParsedRequests(std::istream &input);

  static ParsedBaseRequests GetParsedBaseRequests(std::istream &input);

  static ParsedStatRequests GetParsedStatRequests(std::istream &input);

  void AddTransportCatalogueData(const json::Array &requests);

  static json::Node GetBusStatJson(int id,
                                   const std::optional<transport_catalogue::detail::RouteStat> &route_stat);

  static json::Node GetStopStatJson(int id,
                                    std::unique_ptr<std::set<std::string_view>> &&stops_stat);

  static json::Node GetMapStatJson(int id, const std::string &map_stat);

  static std::vector<Request> GetTransportCatalogueRequests(const json::Array &requests);

  static renderer::RenderSettings GetMapSettings(const json::Dict &request);

  static routing::RoutingSettings GetRoutingSettings(const json::Dict &requests);

  static serialization::SerializationSettings GetSerializationSettings(const json::Dict &requests);

  static json::Node GetRouteStatJson(int id, const std::optional<routing::RouteData> &route_info);

  inline static const std::string BUS = "Bus"s;
  inline static const std::string STOP = "Stop"s;
  inline static const std::string MAP = "Map"s;
  inline static const std::string ROUTE = "Route"s;

 private:
  transport_catalogue::TransportCatalogue &transport_catalogue_;

  static transport_catalogue::detail::Bus ParseBusInput(const json::Dict &request);

  static transport_catalogue::detail::Stop ParseStopInput(const json::Dict &request);

  static json::Node GetErrorJson(int id);

  static json::Node GetBusStatJson(int id,
                                   const transport_catalogue::detail::RouteStat &route_stat);

  static json::Node GetStopStatJson(int id, const std::set<std::string_view> &stop_stat);

  static json::Node GetRouteItems(const std::vector<routing::RouteItem> &route_items);

  static json::Node GetRouteStatJson(int id, const routing::RouteData &route_info);

  static svg::Point GetOffset(const json::Array &offset);

  static svg::Color GetColor(const json::Node &color);

  static std::vector<svg::Color> GetColorPalette(const json::Array &colors);
};

}
