#include "serialization.h"

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <transport_router.pb.h>

#include <fstream>

using namespace std;

namespace serialization {

using namespace transport_catalogue;
using namespace detail;
using namespace renderer;
using namespace svg;
using namespace routing;

unordered_map<string_view, int> GetStopIds(const unordered_map<string_view, TransportCatalogue::PtrStop> &stops) {
  const auto sorted_stops = GetSortedUnorderedMapKeys(stops);
  unordered_map<string_view, int> stop_ids;
  const auto len = sorted_stops.size();
  stop_ids.reserve(len);
  for (int id = 0; id < len; ++id) {
    stop_ids.emplace(sorted_stops[id], id);
  }
  return stop_ids;
}

proto_tc::TransportCatalogueData SerializeTransportCatalogue(const TransportCatalogue &catalogue) {
  proto_tc::TransportCatalogueData proto_catalogue_data;
  const auto &stops = catalogue.GetAllStops();
  const auto stop_ids = GetStopIds(stops);
  const auto &buses = catalogue.GetAllBuses();
  const auto &distances = catalogue.GetAllDistances();

  for (const auto &[_, stop] : stops) {
    proto_tc::Stop proto_stop;
    proto_stop.set_name(stop->name);
    proto_stop.mutable_coordinates()->set_lng(stop->coordinates.lng);
    proto_stop.mutable_coordinates()->set_lat(stop->coordinates.lat);
    proto_catalogue_data.mutable_stops()->Add(std::move(proto_stop));
  }

  for (const auto &bus : buses) {
    proto_tc::Bus proto_bus;
    proto_bus.set_name(bus->name);
    proto_bus.set_is_circular(bus->route_type == RouteType::CIRCULAR);
    for (string_view stop : bus->stops_on_route) {
      proto_bus.add_stops_on_route(stop_ids.at(stop));
    }
    proto_catalogue_data.mutable_buses()->Add(std::move(proto_bus));
  }

  for (const auto &[stops_pair, distance] : distances) {
    proto_tc::StopsDistance proto_distance;
    proto_distance.set_stop_from(stop_ids.at(stops_pair.first->name));
    proto_distance.set_stop_to(stop_ids.at(stops_pair.second->name));
    proto_distance.set_distance(distance);
    proto_catalogue_data.mutable_distances()->Add(std::move(proto_distance));
  }

  return proto_catalogue_data;
}

TransportCatalogue DeserializeTransportCatalogue(TransportCatalogue &catalogue,
                                                 const proto_tc::TransportCatalogueData &proto_catalogue_data) {
  for (const auto &proto_stop : proto_catalogue_data.stops()) {
    Stop stop;
    stop.name = proto_stop.name();
    stop.coordinates.lng = proto_stop.coordinates().lng();
    stop.coordinates.lat = proto_stop.coordinates().lat();
    catalogue.AddStop(std::move(stop));
  }

  const auto id_stops = GetSortedUnorderedMapKeys(catalogue.GetAllStops());

  for (const auto &proto_distance : proto_catalogue_data.distances()) {
    auto from = id_stops.at(static_cast<int>(proto_distance.stop_from()));
    auto to = id_stops.at(static_cast<int>(proto_distance.stop_to()));
    catalogue.AddDistance({string(from), string(to), static_cast<int>(proto_distance.distance())});
  }

  for (const auto &proto_bus : proto_catalogue_data.buses()) {
    Bus bus;
    bus.name = proto_bus.name();
    bus.route_type = proto_bus.is_circular() ? RouteType::CIRCULAR : RouteType::LINEAR;
    bus.stops_on_route.reserve(proto_bus.stops_on_route().size());
    for (auto stop_id : proto_bus.stops_on_route()) {
      bus.stops_on_route.emplace_back(id_stops.at(static_cast<int>(stop_id)));
    }
    catalogue.AddBus(std::move(bus));
  }

  return catalogue;
}

proto_tc::Color SerializeColor(const Color &color) {
  proto_tc::Color proto_color;
  if (holds_alternative<monostate>(color)) {
    proto_color.set_is_null(true);
    return proto_color;
  }

  if (holds_alternative<string>(color)) {
    proto_color.set_string_name(get<string>(color));
    return proto_color;
  }

  if (holds_alternative<Rgb>(color)) {
    const auto rgb = get<Rgb>(color);
    proto_tc::Rgba proto_rgba;
    proto_color.mutable_rgba()->set_red(rgb.red);
    proto_color.mutable_rgba()->set_green(rgb.green);
    proto_color.mutable_rgba()->set_blue(rgb.blue);
    proto_color.mutable_rgba()->set_is_rgb(true);
    return proto_color;
  }

  const auto rgba = get<Rgba>(color);
  proto_tc::Rgba proto_rgba;
  proto_color.mutable_rgba()->set_red(rgba.red);
  proto_color.mutable_rgba()->set_green(rgba.green);
  proto_color.mutable_rgba()->set_blue(rgba.blue);
  proto_color.mutable_rgba()->set_is_rgb(false);
  proto_color.mutable_rgba()->set_opacity(rgba.opacity);

  return proto_color;
}

proto_tc::RenderSettings SerializeRenderSettings(const RenderSettings &render_settings) {
  proto_tc::RenderSettings proto_settings;

  proto_settings.set_width(render_settings.GetWidth());
  proto_settings.set_height(render_settings.GetHeight());

  proto_settings.set_padding(render_settings.GetPadding());

  proto_settings.set_stop_radius(render_settings.GetStopRadius());
  proto_settings.set_line_width(render_settings.GetLineWidth());

  proto_settings.set_bus_label_font_size(render_settings.GetBusLabelFontSize());
  const auto bus_label_offset = render_settings.GetBusLabelOffset();
  proto_settings.mutable_bus_label_offset()->set_x(bus_label_offset.x);
  proto_settings.mutable_bus_label_offset()->set_y(bus_label_offset.y);

  proto_settings.set_stop_label_font_size(render_settings.GetStopLabelFontSize());
  const auto stop_label_offset = render_settings.GetStopLabelOffset();
  proto_settings.mutable_stop_label_offset()->set_x(stop_label_offset.x);
  proto_settings.mutable_stop_label_offset()->set_y(stop_label_offset.y);

  *proto_settings.mutable_underlayer_color() =
      std::move(SerializeColor(render_settings.GetUnderlayerColor()));
  proto_settings.set_underlayer_width(render_settings.GetUnderlayerWidth());

  for (const auto &color : render_settings.GetColorPalette()) {
    *proto_settings.add_color_palette() = std::move(SerializeColor(color));
  }

  return proto_settings;
}

Color DeserializeColor(const proto_tc::Color &proto_color) {
  if (proto_color.is_null()) {
    return monostate();
  }

  if (!proto_color.string_name().empty()) {
    return proto_color.string_name();
  }

  bool is_rgb = proto_color.rgba().is_rgb();

  if (is_rgb) {
    Rgb rgb;
    rgb.red = proto_color.rgba().red();
    rgb.green = proto_color.rgba().green();
    rgb.blue = proto_color.rgba().blue();
    return rgb;
  }

  Rgba rgba;
  rgba.red = proto_color.rgba().red();
  rgba.green = proto_color.rgba().green();
  rgba.blue = proto_color.rgba().blue();
  rgba.opacity = proto_color.rgba().opacity();
  return rgba;
}

RenderSettings DeserializeRenderSettings(const proto_tc::RenderSettings &proto_settings) {
  RenderSettings render_settings;

  const auto &bus_label_offset = proto_settings.bus_label_offset();
  const auto &stop_label_offset = proto_settings.stop_label_offset();
  const auto &underlayer_color = proto_settings.underlayer_color();

  vector<Color> color_palette;
  color_palette.reserve(proto_settings.color_palette_size());
  for (const auto &color : proto_settings.color_palette()) {
    color_palette.emplace_back(DeserializeColor(color));
  }

  render_settings.SetWidth(proto_settings.width())
      .SetHeight(proto_settings.height())
      .SetPadding(proto_settings.padding())
      .SetLineWidth(proto_settings.line_width())
      .SetStopRadius(proto_settings.stop_radius())
      .SetBusLabelFontSize(static_cast<int>(proto_settings.bus_label_font_size()))
      .SetBusLabelOffset({bus_label_offset.x(), bus_label_offset.y()})
      .SetStopLabelFontSize(static_cast<int>(proto_settings.stop_label_font_size()))
      .SetStopLabelOffset({stop_label_offset.x(), stop_label_offset.y()})
      .SetUnderlayerColor(DeserializeColor(underlayer_color))
      .SetUnderlayerWidth(proto_settings.underlayer_width())
      .SetColorPalette(std::move(color_palette));

  return render_settings;
}

proto_tc::TransportRouter SerializeTransportRouter(const TransportRouter &transport_router,
                                                   const TransportCatalogue &catalogue) {
  proto_tc::TransportRouter proto_transport_router;
  const auto stop_ids = GetStopIds(catalogue.GetAllStops());

  const auto &routing_settings = transport_router.GetRoutingSettings();
  proto_tc::RoutingSettings proto_settings;
  proto_settings.set_bus_velocity(routing_settings.bus_velocity);
  proto_settings.set_bus_wait_time(routing_settings.bus_wait_time);
  *proto_transport_router.mutable_routing_settings() = proto_settings;
  proto_transport_router.mutable_routing_settings()->set_bus_velocity(routing_settings.bus_velocity);
  proto_transport_router.mutable_routing_settings()->set_bus_wait_time(routing_settings.bus_wait_time);

  const auto &graph = transport_router.GetGraph();
  proto_transport_router.mutable_graph()->set_vertex_count(graph.GetVertexCount());
  for (int id = 0; id < graph.GetEdgeCount(); ++id) {
    proto_tc::Edge proto_edge;
    const auto &graph_edge = graph.GetEdge(id);
    proto_edge.set_from(graph_edge.from);
    proto_edge.set_to(graph_edge.to);
    proto_edge.set_weight(graph_edge.weight);
    *proto_transport_router.mutable_graph()->add_edges() = proto_edge;

    proto_tc::BusRouteItem proto_bus_route_item;
    const auto &[bus_route_item, stop_from] = transport_router.GetEdge(id);
    proto_bus_route_item.set_time(bus_route_item.time);
    proto_bus_route_item.set_bus(bus_route_item.bus);
    proto_bus_route_item.set_span_count(bus_route_item.span_count);
    proto_bus_route_item.set_stop_from(stop_ids.at(stop_from));
    *proto_transport_router.add_edges() = proto_bus_route_item;
  }

  for (const auto &[name, id] : stop_ids) {
    proto_tc::StopVertex proto_stop_vertex;
    proto_stop_vertex.set_stop(id);
    const auto vertexId = transport_router.GetVertexIdByStopName(name);
    if (vertexId.has_value()) {
      proto_stop_vertex.set_vertex(vertexId.value());
      *proto_transport_router.add_vertexes() = proto_stop_vertex;
    }
  }

  return proto_transport_router;
}

TransportRouter DeserializeTransportRouter(const proto_tc::TransportRouter &proto_transport_router,
                                           TransportCatalogue &catalogue) {
  RoutingSettings routing_settings;
  routing_settings.bus_wait_time =
      static_cast<int>(proto_transport_router.routing_settings().bus_wait_time());
  routing_settings.bus_velocity =
      static_cast<int>(proto_transport_router.routing_settings().bus_velocity());

  const auto id_stops = GetSortedUnorderedMapKeys(catalogue.GetAllStops());
  TransportRouter::Graph graph(proto_transport_router.graph().vertex_count());
  TransportRouter::Edges router_edges;
  router_edges.reserve(proto_transport_router.edges_size());
  for (int id = 0; id < proto_transport_router.edges_size(); ++id) {
    const auto &proto_graph_edge = proto_transport_router.graph().edges(id);
    graph.AddEdge({proto_graph_edge.from(), proto_graph_edge.to(), proto_graph_edge.weight()});

    const auto &proto_router_edge = proto_transport_router.edges(id);
    router_edges.emplace(id, make_pair(BusRouteItem(
                                           proto_router_edge.time(),
                                           proto_router_edge.bus(),
                                           static_cast<int>(proto_router_edge.span_count())),
                                       id_stops.at(static_cast<int>(proto_router_edge.stop_from()))));
  }

  TransportRouter::Vertexes router_vertexes;
  router_vertexes.reserve(proto_transport_router.vertexes_size());
  for (const auto &proto_router_vertex : proto_transport_router.vertexes()) {
    router_vertexes.emplace(id_stops.at(static_cast<int>(proto_router_vertex.stop())),
                            proto_router_vertex.vertex());
  }

  return {catalogue,
          routing_settings,
          std::move(graph),
          std::move(router_vertexes),
          std::move(router_edges)};
}

void Serialize(const SerializationSettings &settings,
               const TransportCatalogue &catalogue,
               const RenderSettings &render_settings,
               const TransportRouter &transport_router) {
  proto_tc::TransportCatalogue proto_catalogue;
  *proto_catalogue.mutable_data() = SerializeTransportCatalogue(catalogue);
  *proto_catalogue.mutable_render_settings() = SerializeRenderSettings(render_settings);
  *proto_catalogue.mutable_router() = SerializeTransportRouter(transport_router, catalogue);
  ofstream output(settings.db_path, ios::binary);
  proto_catalogue.SerializeToOstream(&output);
}

pair<RenderSettings, TransportRouter> Deserialize(const SerializationSettings &settings,
                                                  TransportCatalogue &catalogue) {
  ifstream input(settings.db_path, ios::binary);
  proto_tc::TransportCatalogue proto_catalogue;
  proto_catalogue.ParseFromIstream(&input);
  DeserializeTransportCatalogue(catalogue, proto_catalogue.data());
  return {DeserializeRenderSettings(proto_catalogue.render_settings()),
          DeserializeTransportRouter(proto_catalogue.router(), catalogue)};
}

}
