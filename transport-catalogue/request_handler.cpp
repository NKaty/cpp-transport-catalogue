#include "request_handler.h"

using namespace std;

namespace request {

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

}
