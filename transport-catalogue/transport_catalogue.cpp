#include "transport_catalogue.h"

#include <iostream>
#include <algorithm>

using namespace std;

namespace transport_catalogue {

using namespace detail;
using namespace geo;

void TransportCatalogue::AddStop(Stop &&stop) {
  const auto it = stops_list_.insert(stops_list_.begin(), std::move(stop));
  stops_[it->name] = make_shared<Stop>(*it);
}

void TransportCatalogue::AddBus(Bus &&bus) {
  const auto it = buses_list_.insert(buses_list_.begin(), std::move(bus));
  transform(
      it->stops_on_route.begin(),
      it->stops_on_route.end(),
      it->stops_on_route.begin(),
      [this, &it](string_view stop) {
        stops_[stop]->buses_through_stop.insert(it->name);
        return string_view(stops_[stop]->name);
      }
  );
  it->unique_stops_count =
      set<string_view>(it->stops_on_route.begin(), it->stops_on_route.end()).size();
  it->geo_route_distance = CalculateGeoRouteDistance(*it);
  it->route_distance = CalculateRouteDistance(*it);
  it->curvature = it->route_distance / it->geo_route_distance;
  buses_[it->name] = make_shared<Bus>(*it);
}

void TransportCatalogue::AddDistance(const detail::StopsDistance &distance) {
  distances_between_stops_.insert({{stops_.at(distance.stop_from), stops_.at(distance.stop_to)},
                                   distance.distance});
}

const Bus &TransportCatalogue::FindBus(string_view name) const {
  return *buses_.at(name);
}

const Stop &TransportCatalogue::FindStop(string_view name) const {
  return *stops_.at(name);
}

optional<RouteStat> TransportCatalogue::GetRouteStat(string_view bus_name) const {
  const auto it = buses_.find(bus_name);
  if (it == buses_.end()) {
    return nullopt;
  }
  return detail::RouteStat(*it->second);
}

double TransportCatalogue::CalculateGeoRouteDistance(const Bus &bus) const {
  double route_distance = SumDistances(bus.stops_on_route.begin(),
                                       bus.stops_on_route.end(),
                                       0.,
                                       [this](string_view stop_from, string_view stop_to) {
                                         return ComputeDistance(stops_.at(stop_from)->coordinates,
                                                                stops_.at(stop_to)->coordinates);
                                       });
  return bus.route_type == detail::RouteType::CIRCULAR ? route_distance : route_distance * 2;
}

[[nodiscard]] int TransportCatalogue::CalculateRouteDistance(const Bus &bus) const {
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

unique_ptr<set<string_view>> TransportCatalogue::GetBusesThroughStop(string_view stop_name) const {
  const auto it = stops_.find(stop_name);
  if (it == stops_.end()) {
    return nullptr;
  }
  return make_unique<set<string_view>>(it->second->buses_through_stop);
}

vector<TransportCatalogue::PtrBus> TransportCatalogue::GetAllBuses() const {
  vector<PtrBus> buses(buses_.size());
  transform(
      std::execution::par,
      buses_.begin(), buses_.end(),
      buses.begin(),
      [](const auto &item) {
        return item.second;
      });

  sort(
      execution::par,
      buses.begin(), buses.end(),
      [](auto lhs, auto rhs) {
        return lhs->name < rhs->name;
      });
  return buses;
}

const unordered_map<string_view, TransportCatalogue::PtrStop> &TransportCatalogue::GetAllStops() const {
  return stops_;
}

}
