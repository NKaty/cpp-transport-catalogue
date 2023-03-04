#include "stat_reader.h"

#include "iostream"

using namespace std;

namespace transport_catalogue::stat_parser {
using namespace detail;

const string_view BUS = "Bus"sv;
const string_view STOP = "Stop"sv;

const size_t STOP_SIZE = 4;
const size_t BUS_SIZE = 3;

void ParseStatRequests(istream &stream, const TransportCatalogue &transport_catalogue) {
  int line_count = 0;
  stream >> line_count;
  stream.get();
  string line;
  for (int i = 0; i < line_count; ++i) {
    getline(stream, line);
    const size_t first_letter = line.find_first_not_of(' ');
    if (line.substr(first_letter, BUS_SIZE) == BUS) {
      const size_t name_start = line.find_first_not_of(' ', BUS_SIZE + first_letter);
      const size_t name_end = line.find_last_not_of(' ');
      const auto bus_name = line.substr(name_start, name_end - name_start + 1);
      PrintBusInfo(bus_name, transport_catalogue.GetRouteStat(bus_name));
    } else if (line.substr(first_letter, STOP_SIZE) == STOP) {
      const size_t name_start = line.find_first_not_of(' ', STOP_SIZE + first_letter);
      const size_t name_end = line.find_last_not_of(' ');
      const auto stop_name = line.substr(name_start, name_end - name_start + 1);
      PrintStopInfo(stop_name, transport_catalogue.GetBusesThroughStop(stop_name));
    }
  }
}

void PrintBusInfo(string_view bus_name, const optional<RouteStat> &route_stat) {
  if (route_stat) {
    cout << *route_stat << endl;
  } else {
    cout << "Bus "s << bus_name << ": not found"s << endl;
  }
}

void PrintStopInfo(string_view stop_name, const set<string_view> *stop_stat) {
  if (!stop_stat) {
    cout << "Stop "s << stop_name << ": not found"s << endl;
  } else if (stop_stat->empty()) {
    cout << "Stop "s << stop_name << ": no buses"s << endl;
  } else {
    cout << "Stop "s << stop_name << ": buses "s << *stop_stat << endl;
  }
}
}