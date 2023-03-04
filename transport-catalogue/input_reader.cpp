#include "input_reader.h"

#include "vector"
#include "iostream"

using namespace std;

namespace transport_catalogue::input_parser {
using namespace detail;

const string_view BUS = "Bus"sv;
const string_view STOP = "Stop"sv;

const size_t STOP_SIZE = 4;
const size_t BUS_SIZE = 3;

string_view Trim(string_view str) {
  const size_t start = str.find_first_not_of(' ');
  const size_t end = str.find_last_not_of(' ');
  return str.substr(start, end - start + 1);
}

Bus ParseBusInput(string_view line) {
  Bus bus;
  const size_t name_start = BUS_SIZE;
  const size_t name_end = line.find(':', BUS_SIZE);
  bus.name = string(Trim(line.substr(name_start, name_end - name_start)));
  bus.route_type = line[line.find_first_of("->"sv)] == '>'
                   ? RouteType::CIRCULAR
                   : RouteType::LINEAR;
  const char delimiter = bus.route_type == RouteType::CIRCULAR ? '>' : '-';
  size_t stop_start = name_end + 1;
  while (stop_start < line.size()) {
    size_t stop_end = line.find(delimiter, stop_start);
    stop_end = stop_end == string_view::npos ? line.size() : stop_end;
    bus.stops_on_route.push_back(Trim(line.substr(stop_start, stop_end - stop_start)));
    stop_start = stop_end + 1;
  }
  return bus;
}

Stop ParseStopInput(string_view line) {
  Stop stop;
  const size_t name_start = STOP_SIZE;
  const size_t name_end = line.find(':', STOP_SIZE);
  stop.name = string(Trim(line.substr(name_start, name_end - name_start)));
  const size_t latitude_start = name_end + 1;
  const size_t latitude_end = line.find(',', latitude_start);
  stop.coordinates.lat =
      stod(string(Trim(line.substr(latitude_start, latitude_end - latitude_start))));
  const size_t longitude_start = latitude_end + 1;
  const size_t longitude_end = line.size();
  stop.coordinates.lng =
      stod(string(Trim(line.substr(longitude_start, longitude_end - longitude_start))));
  return stop;
}

void ParseDistanceInput(string_view line, string_view stop_from, vector<StopsDistance> &distances) {
  size_t number_start = line.find_first_not_of(' ');
  while (number_start < line.size()) {
    size_t stop_name_end = line.find(',', number_start);
    stop_name_end = stop_name_end == string_view::npos ? line.size() : stop_name_end;
    const size_t number_end = line.find('m', number_start);
    const int number = stoi(string(Trim(line.substr(number_start, number_end - number_start))));
    const size_t stop_name_start = line.find("to"sv, number_end + 1) + 2;
    const string_view
        stop_name = Trim(line.substr(stop_name_start, stop_name_end - stop_name_start));
    distances.push_back({string(stop_from), string(stop_name), number});
    number_start = stop_name_end + 1;
  }
}

void ParseInput(istream &stream, TransportCatalogue &transport_catalogue) {
  int line_count = 0;
  stream >> line_count;
  stream.get();
  vector<string> buses;
  vector<StopsDistance> distances;
  string line;
  for (int i = 0; i < line_count; ++i) {
    getline(stream, line);
    const size_t first_letter = line.find_first_not_of(' ');
    if (line.substr(first_letter, BUS_SIZE) == BUS) {
      buses.push_back(line.substr(first_letter));
    } else if (line.substr(first_letter, STOP_SIZE) == STOP) {
      const size_t distances_start = line.find(',', line.find(',') + 1);
      if (distances_start == string_view::npos) {
        transport_catalogue.AddStop(ParseStopInput(line.substr(first_letter)));
      } else {
        auto stop = ParseStopInput(line.substr(first_letter, distances_start - first_letter));
        ParseDistanceInput(line.substr(distances_start + 1), stop.name, distances);
        transport_catalogue.AddStop(std::move(stop));
      }
    }
  }
  for (const auto &distance : distances) {
    transport_catalogue.AddDistance(distance);
  }
  for (const auto &bus : buses) {
    transport_catalogue.AddBus(ParseBusInput(bus));
  }
}
}
