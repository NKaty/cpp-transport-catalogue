#include "domain.h"

#include <iomanip>

using namespace std;

namespace transport_catalogue::detail {

ostream &operator<<(ostream &os, const RouteStat &route_stat) {
  os << setprecision(6) << "Bus "s << route_stat.bus_name << ": "s << route_stat.stops_count
     << " stops on route, "s << route_stat.unique_stops_count << " unique stops, "s
     << route_stat.route_distance << " route length, "s << route_stat.curvature << " curvature"s;
  return os;
}

size_t Bus::GetStopCount() const {
  return route_type == RouteType::CIRCULAR ? stops_on_route.size() : stops_on_route.size() * 2 - 1;
}

RouteStat::RouteStat(const Bus &bus) : bus_name(bus.name),
                                       stops_count(bus.GetStopCount()),
                                       unique_stops_count(bus.unique_stops_count),
                                       route_distance(bus.route_distance),
                                       curvature(bus.curvature) {
}

}