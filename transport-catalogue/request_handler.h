#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <set>
#include <string_view>
#include <optional>

namespace request {

class RequestHandler {
 public:
  using OptinalRouteStat = std::optional<transport_catalogue::detail::RouteStat>;

  explicit RequestHandler(transport_catalogue::TransportCatalogue &db);

  [[nodiscard]] OptinalRouteStat GetRouteStat(std::string_view bus_name) const;

  [[nodiscard]] std::unique_ptr<std::set<std::string_view>> GetBusesThroughStop(std::string_view stop_name) const;

  [[nodiscard]] svg::Document RenderMap(renderer::RenderSettings render_settings) const;

  std::optional<routing::RouteData> BuildRoute(routing::RoutingSettings routing_settings,
                                               std::string_view from,
                                               std::string_view to) const;

  void ProcessMakeBaseRequest(std::istream &input);

  void ProcessRequests(std::istream &input, std::ostream &output);

 private:
  transport_catalogue::TransportCatalogue &db_;
  mutable std::optional<renderer::MapRenderer> renderer_{std::nullopt};
  mutable std::optional<routing::TransportRouter> router_{std::nullopt};
};

}