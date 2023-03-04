#pragma once

#include "transport_catalogue.h"

#include <string>

namespace transport_catalogue::input_parser {
std::string_view Trim(std::string_view str);

detail::Bus ParseBusInput(std::string_view line);

detail::Stop ParseStopInput(std::string_view line);

void ParseDistanceInput(std::string_view line,
                        std::string_view stop_from,
                        std::vector<detail::StopsDistance> &distances);

void ParseInput(std::istream &stream, TransportCatalogue &transport_catalogue);
}
