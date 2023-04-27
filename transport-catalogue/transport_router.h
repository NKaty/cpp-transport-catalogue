#pragma once

#include "domain.h"
#include "router.h"
#include "graph.h"
#include "transport_catalogue.h"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>
#include <variant>
#include <memory>

namespace routing {

#define DECLARE_DOUBLE_PARAM(Name) \
  struct Name {                    \
    double value;                  \
    explicit Name(double v)        \
        : value(v) {               \
    }                              \
    operator double() const {      \
        return value;              \
    }                              \
  }

DECLARE_DOUBLE_PARAM(MeterPerMinute);
DECLARE_DOUBLE_PARAM(KmPerHour);

struct RoutingSettings {
  MeterPerMinute bus_velocity{0.};
  int bus_wait_time{0};
  RoutingSettings(MeterPerMinute bus_velocity, int bus_wait_time)
      : bus_velocity(bus_velocity), bus_wait_time(bus_wait_time) {}
  RoutingSettings(KmPerHour bus_velocity, int bus_wait_time)
      : bus_velocity(1000. * bus_velocity / 60.), bus_wait_time(bus_wait_time) {}
};

struct WaitRouteItem {
  std::string type = "Wait"s;
  double time = 0.;
  std::string stop;
  WaitRouteItem() = default;
  WaitRouteItem(double time, std::string_view stop) : time(time), stop(stop) {}
};

struct BusRouteItem {
  std::string type = "Bus"s;
  double time = 0.;
  std::string bus;
  int span_count = 0;
  BusRouteItem() = default;
  BusRouteItem(double time, std::string bus, int span_count)
      : time(time), bus(std::move(bus)), span_count(span_count) {}
};

using RouteItem = std::variant<WaitRouteItem, BusRouteItem>;

struct RouteData {
  double total_time = 0.;
  std::vector<RouteItem> items;
};

class TransportRouter {
 public:
  using Graph = graph::DirectedWeightedGraph<double>;
  using Router = graph::Router<double>;

  TransportRouter(const transport_catalogue::TransportCatalogue &catalogue,
                  RoutingSettings settings);

  [[nodiscard]] std::optional<RouteData> BuildRoute(std::string_view from,
                                                    std::string_view to) const;

 private:
  const transport_catalogue::TransportCatalogue &catalogue_;
  RoutingSettings settings_;
  std::unordered_map<std::string_view, graph::VertexId> vertexes_;
  std::unordered_map<graph::EdgeId, std::pair<BusRouteItem, std::string_view>> edges_;
  std::unique_ptr<Graph> graph_;
  Router router_;

  std::unique_ptr<TransportRouter::Graph> BuildGraph();

  graph::VertexId AddVertex(std::string_view stop);

  void AddEdge(Graph &graph,
               const std::shared_ptr<transport_catalogue::detail::Bus> &bus,
               double &weight,
               int prev_index,
               int from_index,
               int to_index);
};

}
