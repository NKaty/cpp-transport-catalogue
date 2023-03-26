#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <set>
#include <string_view>
#include <optional>

namespace request {

class RequestHandler {
 public:
  using OptinalRouteStat = std::optional<transport_catalogue::detail::RouteStat>;

  RequestHandler(transport_catalogue::TransportCatalogue &db,
                 renderer::MapRenderer &&renderer);

  explicit RequestHandler(transport_catalogue::TransportCatalogue &db);

  [[nodiscard]] OptinalRouteStat GetRouteStat(std::string_view bus_name) const;

  [[nodiscard]] std::unique_ptr<std::set<std::string_view>> GetBusesThroughStop(std::string_view stop_name) const;

  [[nodiscard]] svg::Document RenderMap() const;

  void ProcessJsonRequests(std::istream &input,
                           std::ostream &output);

 private:
  transport_catalogue::TransportCatalogue &db_;
  std::optional<renderer::MapRenderer> renderer_{std::nullopt};
};

}