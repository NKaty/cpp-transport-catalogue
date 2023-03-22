#pragma once

#include "transport_catalogue.h"

#include <string>
#include <memory>

namespace transport_catalogue::stat_parser {
std::ostream &ParseStatRequests(std::istream &is,
                                std::ostream &os,
                                const TransportCatalogue &transport_catalogue);

void PrintBusInfo(std::ostream &os,
                  std::string_view bus_name,
                  const std::optional<detail::RouteStat> &route_stat);

void PrintStopInfo(std::ostream &os,
                   std::string_view stop_name,
                   std::unique_ptr<std::set<std::string_view>> stop_stat);
}
