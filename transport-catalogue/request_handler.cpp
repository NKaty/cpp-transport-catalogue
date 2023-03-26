#include "request_handler.h"
#include "json.h"
#include "json_reader.h"

#include <sstream>

using namespace std;

namespace request {

using namespace json;
using namespace renderer;
using namespace transport_catalogue;

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue &db,
                               renderer::MapRenderer &&renderer)
    : db_(db), renderer_(std::move(renderer)) {}

RequestHandler::RequestHandler(transport_catalogue::TransportCatalogue &db)
    : db_(db) {}

[[nodiscard]] RequestHandler::OptinalRouteStat RequestHandler::GetRouteStat(string_view bus_name) const {
  return db_.GetRouteStat(bus_name);
}

[[nodiscard]] unique_ptr<set<string_view>> RequestHandler::GetBusesThroughStop(string_view stop_name) const {
  return db_.GetBusesThroughStop(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
  return renderer_->RenderMap(db_.GetAllBuses(), db_.GetAllStops());
}

void RequestHandler::ProcessJsonRequests(istream &input, ostream &output) {
  const auto request_collections = JsonReader::GetParsedRequests(input);
  JsonReader json_reader(db_);
  renderer_ = MapRenderer(JsonReader::GetMapSettings(request_collections.render_settings));
  json_reader.AddTransportCatalogueData(request_collections.base_requests);
  const auto parsed_requests =
      JsonReader::GetTransportCatalogueRequests(request_collections.stat_requests);
  Array stats;
  for (const auto &req : parsed_requests) {
    if (req.type == JsonReader::BUS) {
      const auto route_stat = GetRouteStat(req.name);
      stats.emplace_back(JsonReader::GetBusStatJson(req.id, route_stat));
    } else if (req.type == JsonReader::STOP) {
      auto stops_stat = GetBusesThroughStop(req.name);
      stats.emplace_back(JsonReader::GetStopStatJson(req.id, std::move(stops_stat)));
    } else if (req.type == JsonReader::MAP) {
      ostringstream buffer;
      RenderMap().Render(buffer);
      stats.emplace_back(JsonReader::GetMapStatJson(req.id, buffer.str()));
    }
  }
  Print(json::Document(stats), output);
}

}
