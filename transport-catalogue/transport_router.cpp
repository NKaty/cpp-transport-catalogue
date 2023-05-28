#include "transport_router.h"

#include <numeric>

using namespace std;

namespace routing {

using namespace graph;
using namespace transport_catalogue;
using namespace transport_catalogue::detail;

TransportRouter::TransportRouter(const TransportCatalogue &catalogue, RoutingSettings settings)
    : catalogue_(catalogue), settings_(settings), graph_(BuildGraph()), router_(*graph_) {}

TransportRouter::TransportRouter(const TransportCatalogue &catalogue,
                                 RoutingSettings settings,
                                 Graph graph,
                                 Vertexes router_vertexes,
                                 Edges router_edges)
    : catalogue_(catalogue),
      settings_(settings),
      graph_(std::make_unique<Graph>(std::move(graph))),
      vertexes_(std::move(router_vertexes)),
      edges_(std::move(router_edges)),
      router_(*graph_) {}

unique_ptr<TransportRouter::Graph> TransportRouter::BuildGraph() {
  Graph graph(catalogue_.GetAllStops().size());
  for (const auto &bus : catalogue_.GetAllBuses()) {
    const int stop_count = static_cast<int>(bus->stops_on_route.size());
    for (int i_fwd = 0, i_bwd = stop_count - 1; i_fwd < stop_count; ++i_fwd, --i_bwd) {
      double weight_fwd = 0., weight_bwd = 0.;
      for (int j_fwd = i_fwd + 1, j_bwd = i_bwd - 1; j_fwd < stop_count; ++j_fwd, --j_bwd) {
        AddEdge(graph, bus, weight_fwd, j_fwd - 1, i_fwd, j_fwd);
        if (bus->route_type == detail::RouteType::LINEAR) {
          AddEdge(graph, bus, weight_bwd, j_bwd + 1, i_bwd, j_bwd);
        }
      }
    }
  }
  return std::make_unique<Graph>(graph);
}

void TransportRouter::AddEdge(Graph &graph,
                              const shared_ptr<Bus> &bus,
                              double &weight,
                              int prev_index,
                              int from_index,
                              int to_index) {
  auto from = bus->stops_on_route[from_index];
  auto to = bus->stops_on_route[to_index];
  auto prev = bus->stops_on_route[prev_index];
  auto span_count = std::abs(to_index - from_index);
  std::optional<int> distance = catalogue_.GetDistanceBetweenStops(prev, to);

  if (distance.has_value()) {
    weight += static_cast<double>(*distance) / settings_.bus_velocity;
    graph::EdgeId edge_id = graph.AddEdge({AddVertex(from), AddVertex(to),
                                           weight + settings_.bus_wait_time});
    edges_[edge_id] = make_pair(BusRouteItem(weight, bus->name, span_count), from);
  }
}

graph::VertexId TransportRouter::AddVertex(string_view stop) {
  auto it = vertexes_.find(stop);
  if (it != vertexes_.end()) {
    return it->second;
  }
  vertexes_[stop] = vertexes_.size();
  return vertexes_[stop];
}

optional<RouteData> TransportRouter::BuildRoute(string_view from, string_view to) const {
  const auto it_from = vertexes_.find(from);
  const auto it_to = vertexes_.find(to);
  if (it_from == vertexes_.end() || it_to == vertexes_.end()) {
    return nullopt;
  }

  auto route = router_.BuildRoute(it_from->second, it_to->second);
  if (route) {
    RouteData route_data;
    route_data.total_time = route->weight;
    route_data.items.reserve(route->edges.size() * 2);
    for_each(
        route->edges.begin(), route->edges.end(),
        [&](graph::EdgeId edge_id) {
          const auto [bus_route_item, stop_name] = edges_.at(edge_id);
          WaitRouteItem wait_route_item(settings_.bus_wait_time, stop_name);
          route_data.items.emplace_back(wait_route_item);
          route_data.items.emplace_back(bus_route_item);
        });
    return route_data;
  }

  return nullopt;
}

const RoutingSettings &TransportRouter::GetRoutingSettings() const {
  return settings_;
}

const TransportRouter::Graph &TransportRouter::GetGraph() const {
  return *graph_;
}

const pair<BusRouteItem, string_view> &TransportRouter::GetEdge(EdgeId edge_id) const {
  return edges_.at(edge_id);
}

optional<graph::VertexId> TransportRouter::GetVertexIdByStopName(string_view stop_name) const {
  const auto it = vertexes_.find(stop_name);
  if (it != vertexes_.end()) {
    return vertexes_.at(stop_name);
  }
  return nullopt;
}

}