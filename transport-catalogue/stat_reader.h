#pragma once

#include "transport_catalogue.h"

#include <string>

namespace transport_catalogue::stat_parser {
void ParseStatRequests(std::istream &stream, const TransportCatalogue &transport_catalogue);

void PrintBusInfo(std::string_view bus_name, const std::optional<detail::RouteStat> &route_stat);

void PrintStopInfo(std::string_view stop_name, const std::set<std::string_view> *stop_stat);
}
