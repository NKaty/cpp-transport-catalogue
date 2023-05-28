#pragma once

#include "geo.h"

#include <string>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>
//#include <execution>

using namespace std::string_literals;

namespace transport_catalogue::detail {

struct PairHash {
  template<typename T>
  size_t operator()(const std::pair<T, T> &p) const {
    return (std::hash<T>()(p.first) * 31) ^ (std::hash<T>()(p.second) * 37);
  }
};

struct Stop {
  std::string name;
  geo::Coordinates coordinates;
  std::set<std::string_view> buses_through_stop;
};

enum class RouteType { CIRCULAR, LINEAR };

struct Bus {
  std::string name;
  std::vector<std::string_view> stops_on_route;
  RouteType route_type;
  size_t unique_stops_count = 0;
  double geo_route_distance = 0.;
  int route_distance = 0;
  double curvature = 0.;
  [[nodiscard]] size_t GetStopCount() const;
};

struct RouteStat {
  RouteStat() = default;
  explicit RouteStat(const Bus &bus);
  std::string_view bus_name;
  size_t stops_count = 0;
  size_t unique_stops_count = 0;
  int route_distance = 0.;
  double curvature = 0.;
};

struct StopsDistance {
  std::string stop_from;
  std::string stop_to;
  int distance = 0;
};

std::ostream &operator<<(std::ostream &os, const RouteStat &route_stat);

template<typename Value>
std::ostream &operator<<(std::ostream &os, const std::set<Value> &collection) {
  bool first = true;
  for (const auto item : collection) {
    if (first) {
      os << item;
      first = false;
    } else {
      os << " "s << item;
    }
  }
  return os;
}

template<typename Key, typename Value>
std::vector<Key> GetSortedUnorderedMapKeys(const std::unordered_map<Key, Value> &container) {
  std::vector<Key> keys(container.size());
  std::transform(
//      std::execution::par,
      container.begin(),
      container.end(),
      keys.begin(),
      [](const auto &item) {
        return item.first;
      });
  std::sort(
//      std::execution::par,
      keys.begin(),
      keys.end()
  );
  return keys;
}

}