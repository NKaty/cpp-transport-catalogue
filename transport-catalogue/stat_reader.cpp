#include "stat_reader.h"

#include "iostream"

using namespace std;

namespace transport_catalogue::stat_parser {
using namespace detail;

const string_view BUS = "Bus"sv;
const string_view STOP = "Stop"sv;

const size_t STOP_SIZE = 4;
const size_t BUS_SIZE = 3;

ostream &ParseStatRequests(istream &is,
                           ostream &os,
                           const TransportCatalogue &transport_catalogue) {
  int line_count = 0;
  is >> line_count;
  is.get();
  string line;
  for (int i = 0; i < line_count; ++i) {
    getline(is, line);
    const size_t first_letter = line.find_first_not_of(' ');
    if (line.substr(first_letter, BUS_SIZE) == BUS) {
      const size_t name_start = line.find_first_not_of(' ', BUS_SIZE + first_letter);
      const size_t name_end = line.find_last_not_of(' ');
      const auto bus_name = line.substr(name_start, name_end - name_start + 1);
      PrintBusInfo(os, bus_name, transport_catalogue.GetRouteStat(bus_name));
    } else if (line.substr(first_letter, STOP_SIZE) == STOP) {
      const size_t name_start = line.find_first_not_of(' ', STOP_SIZE + first_letter);
      const size_t name_end = line.find_last_not_of(' ');
      const auto stop_name = line.substr(name_start, name_end - name_start + 1);
      PrintStopInfo(os, stop_name, transport_catalogue.GetBusesThroughStop(stop_name));
    }
  }
  return os;
}

void PrintBusInfo(ostream &os, string_view bus_name, const optional<RouteStat> &route_stat) {
  if (route_stat) {
    os << *route_stat << endl;
  } else {
    os << "Bus "s << bus_name << ": not found"s << endl;
  }
}

void PrintStopInfo(ostream &os, string_view stop_name, const unique_ptr<set<string_view>> stop_stat) {
  if (!stop_stat) {
    os << "Stop "s << stop_name << ": not found"s << endl;
  } else if (stop_stat->empty()) {
    os << "Stop "s << stop_name << ": no buses"s << endl;
  } else {
    os << "Stop "s << stop_name << ": buses "s << *stop_stat << endl;
  }
}
}
