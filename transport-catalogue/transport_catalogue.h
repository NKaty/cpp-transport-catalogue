#pragma once

#include "domain.h"

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

class TransportCatalogue {
 public:
  void AddStop(detail::Stop &&stop);

  void AddBus(detail::Bus &&bus);

  void AddDistance(const detail::StopsDistance &distance);

  [[nodiscard]] const detail::Bus &FindBus(std::string_view name) const;

  [[nodiscard]] const detail::Stop &FindStop(std::string_view name) const;

  [[nodiscard]] std::optional<detail::RouteStat> GetRouteStat(std::string_view bus_name) const;

  [[nodiscard]] const std::set<std::string_view> *GetBusesThroughStop(std::string_view stop_name) const;

  [[nodiscard]] std::vector<const detail::Bus *> GetAllBuses() const;

  [[nodiscard]] const std::unordered_map<std::string_view, const detail::Stop *> &GetAllStops() const;

  [[nodiscard]] std::vector<geo::Coordinates> GetStopCoords() const;

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
