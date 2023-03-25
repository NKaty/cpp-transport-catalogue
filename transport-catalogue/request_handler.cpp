#include "request_handler.h"
#include "json.h"
#include "json_reader.h"

#include <sstream>

using namespace std;

namespace request {

using namespace json;
using namespace renderer;
using namespace transport_catalogue;

RequestHandler::RequestHandler(const transport_catalogue::TransportCatalogue &db,
                               const renderer::MapRenderer &renderer)
    : db_(db), renderer_(renderer) {}

[[nodiscard]] RequestHandler::OptinalRouteStat RequestHandler::GetRouteStat(string_view bus_name) const {
  return db_.GetRouteStat(bus_name);
}

[[nodiscard]] unique_ptr<set<string_view>> RequestHandler::GetBusesThroughStop(string_view stop_name) const {
  return db_.GetBusesThroughStop(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
  return renderer_.RenderMap(db_.GetAllBuses(), db_.GetAllStops());
}

void ProcessJsonRequests(TransportCatalogue &transport_catalogue, istream &input, ostream &output) {
  const auto json_input = Load(input).GetRoot();
  const auto &base_requests = json_input.AsMap().at("base_requests"s).AsArray();
  const auto &stat_requests = json_input.AsMap().at("stat_requests"s).AsArray();
  const auto &render_settings = json_input.AsMap().at("render_settings"s).AsMap();

  JsonReader json_reader(transport_catalogue);
  auto map_settings = JsonReader::GetMapSettings(render_settings);
  auto map_render = MapRenderer(map_settings);
  RequestHandler request_handler(transport_catalogue, map_render);
  json_reader.AddTransportCatalogueData(base_requests);
  const auto parsed_requests = JsonReader::GetTransportCatalogueRequests(stat_requests);
  Array stats;
  for (const auto &req : parsed_requests) {
    if (req.type == JsonReader::BUS) {
      const auto route_stat = request_handler.GetRouteStat(req.name);
      stats.emplace_back(JsonReader::GetBusStatJson(req.id, route_stat));
    } else if (req.type == JsonReader::STOP) {
      auto stops_stat = request_handler.GetBusesThroughStop(req.name);
      stats.emplace_back(JsonReader::GetStopStatJson(req.id, std::move(stops_stat)));
    } else if (req.type == JsonReader::MAP) {
      ostringstream buffer;
      request_handler.RenderMap().Render(buffer);
      stats.emplace_back(JsonReader::GetMapStatJson(req.id, buffer.str()));
    }
  }
  Print(json::Document(stats), output);
}

}
