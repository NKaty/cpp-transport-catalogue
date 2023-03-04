#pragma once

#include "geo.h"

#include <string>
#include <unordered_map>
#include <vector>
#include <set>
#include <deque>
#include <optional>
#include <functional>
#include <execution>

using namespace std::string_literals;

namespace transport_catalogue {
namespace detail {
struct PairHash {
  template<typename T>
  size_t operator()(const std::pair<T, T> &p) const {
    return (std::hash<T>()(p.first) * 31) ^ (std::hash<T>()(p.second) * 37);
  }
};

struct Stop {
  std::string name;
  geo::Coordinates coordinates;
};

enum class RouteType { CIRCULAR, LINEAR };

struct Bus {
  std::string name;
  std::vector<std::string_view> stops_on_route;
  size_t unique_stops_count = 0;
  RouteType route_type;
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
  double route_distance = 0.;
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
}

class TransportCatalogue {
 public:
  void AddStop(detail::Stop stop);

  void AddBus(detail::Bus bus);

  void AddDistance(const detail::StopsDistance &distance);

  [[nodiscard]] detail::Bus FindBus(std::string_view name) const;

  [[nodiscard]] detail::Stop FindStop(std::string_view name) const;

  [[nodiscard]] std::optional<detail::RouteStat> GetRouteStat(std::string_view bus_name) const;

  [[nodiscard]] const std::set<std::string_view> *GetBusesThroughStop(std::string_view stop_name) const;

 private:
  std::deque<detail::Stop> stops_list_;
  std::deque<detail::Bus> buses_list_;
  std::unordered_map<std::string_view, const detail::Stop *> stops_;
  std::unordered_map<std::string_view, const detail::Bus *> buses_;
  std::unordered_map<std::string_view, std::set<std::string_view>> buses_through_stop_;
  std::unordered_map<std::pair<const detail::Stop *, const detail::Stop *>, int, detail::PairHash>
      distances_between_stops_;

  template<typename F, typename T, typename I>
  [[nodiscard]] T SumDistances(I begin,
                               I end,
                               T start_value,
                               F distance_getter) const;

  [[nodiscard]] double CalculateGeoRouteDistance(const detail::Bus &bus) const;

  [[nodiscard]] int CalculateRouteDistance(const detail::Bus &bus) const;
};

template<typename F, typename T, typename I>
[[nodiscard]] T TransportCatalogue::SumDistances(I begin,
                                                 I end,
                                                 T start_value,
                                                 F distance_getter) const {
  return std::transform_reduce(
      begin,
      prev(end),
      next(begin),
      start_value,
      std::plus{},
      distance_getter
  );
}
}