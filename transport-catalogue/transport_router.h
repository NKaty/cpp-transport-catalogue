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

const int MINUTES_IN_HOUR = 60;
const int METERS_IN_KM = 1000;

struct RoutingSettings {
  double bus_velocity{0.};
  int bus_wait_time{0};
  RoutingSettings() = default;
  RoutingSettings(double bus_velocity, int bus_wait_time)
      : bus_velocity(METERS_IN_KM * bus_velocity / MINUTES_IN_HOUR), bus_wait_time(bus_wait_time) {}
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
  using Vertexes = std::unordered_map<std::string_view, graph::VertexId>;
  using Edges = std::unordered_map<graph::EdgeId, std::pair<BusRouteItem, std::string_view>>;

  TransportRouter(const transport_catalogue::TransportCatalogue &catalogue,
                  RoutingSettings settings);

  TransportRouter(const transport_catalogue::TransportCatalogue &catalogue,
                  RoutingSettings settings,
                  Graph graph,
                  Vertexes router_vertexes,
                  Edges router_edges);

  [[nodiscard]] std::optional<RouteData> BuildRoute(std::string_view from,
                                                    std::string_view to) const;

  [[nodiscard]] const RoutingSettings &GetRoutingSettings() const;

  [[nodiscard]] const Graph &GetGraph() const;

  [[nodiscard]] const std::pair<BusRouteItem,
                                std::string_view> &GetEdge(graph::EdgeId edge_id) const;

  [[nodiscard]] std::optional<graph::VertexId> GetVertexIdByStopName(std::string_view stop_name) const;

 private:
  const transport_catalogue::TransportCatalogue &catalogue_;
  RoutingSettings settings_;
  Vertexes vertexes_;
  Edges edges_;
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
