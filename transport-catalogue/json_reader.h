#pragma once

#include "json.h"
//#include "transport_catalogue.h"
#include "request_handler.h"

#include <iostream>
#include <set>
#include <string_view>

namespace request {

transport_catalogue::detail::Bus ParseBusInput(const json::Dict &request);

transport_catalogue::detail::Stop ParseStopInput(const json::Dict &request);

void AddTransportCatalogueData(transport_catalogue::TransportCatalogue &transport_catalogue,
                               const json::Array &requests);

json::Node GetErrorJson(const json::Node &id);

json::Node GetBusStatJson(const json::Node &id,
                          const transport_catalogue::detail::RouteStat &route_stat);

json::Node GetStopStatJson(const json::Node &id, const std::set<std::string_view> &stop_stat);

json::Node GetMapStatJson(const json::Node &id, const std::string &map_stat);

json::Node GetTransportCatalogueStats(const RequestHandler &request_handler,
                                      const json::Array &requests);

renderer::RenderSettings GetMapSettings(const json::Dict &request);

svg::Point GetOffset(const json::Array &offset);

svg::Color GetColor(const json::Node &color);

std::vector<svg::Color> GetColorPalette(const json::Array &colors);

void ProcessJsonRequests(transport_catalogue::TransportCatalogue &transport_catalogue,
                         std::istream &input,
                         std::ostream &output);

}
