#include "request_handler.h"
#include "json_builder.h"
#include "json.h"
#include "json_reader.h"

#include <sstream>

using namespace std;

namespace request {

using namespace json;
using namespace renderer;
using namespace transport_catalogue;
using namespace routing;

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue &db)
    : db_(db) {}

[[nodiscard]] RequestHandler::OptinalRouteStat RequestHandler::GetRouteStat(string_view bus_name) const {
  return db_.GetRouteStat(bus_name);
}

[[nodiscard]] unique_ptr<set<string_view>> RequestHandler::GetBusesThroughStop(string_view stop_name) const {
  return db_.GetBusesThroughStop(stop_name);
}

svg::Document RequestHandler::RenderMap(RenderSettings render_settings) const {
  if (!renderer_.has_value()) {
    renderer_.emplace(MapRenderer(std::move(render_settings)));
  }
  return renderer_->RenderMap(db_.GetAllBuses(), db_.GetAllStops());
}

optional<RouteData> RequestHandler::BuildRoute(RoutingSettings routing_settings,
                                               string_view from,
                                               string_view to) const {
  if (!router_.has_value()) {
    router_.emplace(TransportRouter(db_, routing_settings));
  }
  return router_->BuildRoute(from, to);
}

void RequestHandler::ProcessJsonRequests(istream &input, ostream &output) {
  const auto request_collections = JsonReader::GetParsedRequests(input);
  JsonReader json_reader(db_);
  auto render_settings = JsonReader::GetMapSettings(request_collections.render_settings);
  auto routing_settings = JsonReader::GetRoutingSettings(request_collections.routing_settings);
  json_reader.AddTransportCatalogueData(request_collections.base_requests);
  const auto parsed_requests =
      JsonReader::GetTransportCatalogueRequests(request_collections.stat_requests);
  Builder json_builder;
  json_builder.StartArray();
  for (const auto &req : parsed_requests) {
    if (req.type == JsonReader::BUS) {
      const auto route_stat = GetRouteStat(req.name);
      json_builder.Value(JsonReader::GetBusStatJson(req.id, route_stat));
    } else if (req.type == JsonReader::STOP) {
      auto stops_stat = GetBusesThroughStop(req.name);
      json_builder.Value(JsonReader::GetStopStatJson(req.id, std::move(stops_stat)));
    } else if (req.type == JsonReader::MAP) {
      ostringstream buffer;
      RenderMap(render_settings).Render(buffer);
      json_builder.Value(JsonReader::GetMapStatJson(req.id, buffer.str()));
    } else if (req.type == JsonReader::ROUTE) {
      auto route = BuildRoute(routing_settings, req.from, req.to);
      json_builder.Value(JsonReader::GetRouteStatJson(req.id, route));
    }
  }
  Print(json::Document(json_builder.EndArray().Build()), output);
}

}
