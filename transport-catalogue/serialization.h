#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include <transport_catalogue.pb.h>
#include <transport_router.pb.h>

#include <filesystem>

namespace serialization {

struct SerializationSettings {
  std::filesystem::path db_path;
};

void Serialize(const SerializationSettings &settings,
               const transport_catalogue::TransportCatalogue &catalogue,
               const renderer::RenderSettings &render_settings,
               const routing::TransportRouter &transport_router);

std::pair<renderer::RenderSettings,
          routing::TransportRouter> Deserialize(const SerializationSettings &settings,
                                                transport_catalogue::TransportCatalogue &catalogue);

}
