#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"

#include <set>
#include <string_view>

namespace request {

class RequestHandler {
 public:
  using OptinalRouteStat = std::optional<transport_catalogue::detail::RouteStat>;

  RequestHandler(const transport_catalogue::TransportCatalogue &db,
                 const renderer::MapRenderer &renderer);

  [[nodiscard]] OptinalRouteStat GetRouteStat(std::string_view bus_name) const;

  [[nodiscard]] std::unique_ptr<std::set<std::string_view>> GetBusesThroughStop(std::string_view stop_name) const;

  [[nodiscard]] svg::Document RenderMap() const;

 private:
  const transport_catalogue::TransportCatalogue &db_;
  const renderer::MapRenderer &renderer_;
};

}