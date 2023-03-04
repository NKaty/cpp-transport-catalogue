#include "transport_catalogue.h"

#include <iostream>
#include <algorithm>
#include <iomanip>

using namespace std;

namespace transport_catalogue {
namespace detail {
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

void TransportCatalogue::AddStop(detail::Stop stop) {
  const auto it = stops_list_.insert(stops_list_.begin(), std::move(stop));
  stops_[it->name] = &(*it);
  buses_through_stop_[it->name] = {};
}

void TransportCatalogue::AddBus(detail::Bus bus) {
  const auto it = buses_list_.insert(buses_list_.begin(), std::move(bus));
  transform(
      it->stops_on_route.begin(),
      it->stops_on_route.end(),
      it->stops_on_route.begin(),
      [this, &it](string_view stop) {
        buses_through_stop_[stop].insert(it->name);
        return string_view(stops_[stop]->name);
      }
  );
  it->unique_stops_count =
      set<string_view>(it->stops_on_route.begin(), it->stops_on_route.end()).size();
  it->geo_route_distance = CalculateGeoRouteDistance(*it);
  it->route_distance = CalculateRouteDistance(*it);
  it->curvature = it->route_distance / it->geo_route_distance;
  buses_[it->name] = &(*it);
}

void TransportCatalogue::AddDistance(const detail::StopsDistance &distance) {
  distances_between_stops_.insert({{stops_.at(distance.stop_from), stops_.at(distance.stop_to)},
                                   distance.distance});
}

detail::Bus TransportCatalogue::FindBus(string_view name) const {
  return *buses_.at(name);
}

detail::Stop TransportCatalogue::FindStop(string_view name) const {
  return *stops_.at(name);
}

optional<detail::RouteStat> TransportCatalogue::GetRouteStat(string_view bus_name) const {
  const auto it = buses_.find(bus_name);
  if (it == buses_.end()) {
    return nullopt;
  }
  return detail::RouteStat(*it->second);
}

double TransportCatalogue::CalculateGeoRouteDistance(const detail::Bus &bus) const {
  double route_distance = SumDistances(bus.stops_on_route.begin(),
                                       bus.stops_on_route.end(),
                                       0.,
                                       [this](string_view stop_from, string_view stop_to) {
                                         return ComputeDistance(stops_.at(stop_from)->coordinates,
                                                                stops_.at(stop_to)->coordinates);
                                       });
  return bus.route_type == detail::RouteType::CIRCULAR ? route_distance : route_distance * 2;
}

[[nodiscard]] int TransportCatalogue::CalculateRouteDistance(const detail::Bus &bus) const {
  const auto getter = [this](string_view stop_from, string_view stop_to) {
    const auto it = distances_between_stops_.find({stops_.at(stop_from), stops_.at(stop_to)});
    if (it != distances_between_stops_.end()) {
      return it->second;
    }
    return distances_between_stops_.at({stops_.at(stop_to), stops_.at(stop_from)});
  };
  int route_distance_to = SumDistances(bus.stops_on_route.begin(),
                                       bus.stops_on_route.end(),
                                       0,
                                       getter);
  if (bus.route_type == detail::RouteType::CIRCULAR) {
    return route_distance_to;
  }
  return route_distance_to + SumDistances(bus.stops_on_route.rbegin(),
                                          bus.stops_on_route.rend(),
                                          0,
                                          getter);
}

const set<string_view> *TransportCatalogue::GetBusesThroughStop(string_view stop_name) const {
  const auto it = buses_through_stop_.find(stop_name);
  if (it == buses_through_stop_.end()) {
    return nullptr;
  }
  return &it->second;
}
}