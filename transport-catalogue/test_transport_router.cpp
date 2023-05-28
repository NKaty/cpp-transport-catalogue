#include "testing_library.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "geo.h"
#include "transport_router.h"

using namespace std;

using namespace transport_catalogue;
using namespace detail;
using namespace geo;
using namespace routing;

namespace {

void AddCircularAndLinearBuses(TransportCatalogue &tc) {
  const Coordinates biryulyovo_coords{55.574371, 37.6517};
  const Coordinates universam_coords{55.587655, 37.645687};
  const Coordinates rasskazovka_coords{55.595579, 37.605757};
  const Coordinates tolstopaltsevo_coords{55.611087, 37.20829};
  const Coordinates marushkino_coords{55.595884, 37.209755};
  tc.AddStop({"Biryulyovo"s, biryulyovo_coords});
  tc.AddStop({"Universam"s, universam_coords});
  tc.AddStop({"Rasskazovka"s, rasskazovka_coords});
  tc.AddStop({"Tolstopaltsevo"s, tolstopaltsevo_coords});
  tc.AddStop({"Marushkino"s, marushkino_coords});
  tc.AddDistance({"Biryulyovo"s, "Universam", 2400});
  tc.AddDistance({"Rasskazovka"s, "Universam", 5600});
  tc.AddDistance({"Biryulyovo"s, "Rasskazovka", 7500});
  tc.AddDistance({"Tolstopaltsevo"s, "Marushkino", 3900});
  tc.AddDistance({"Marushkino"s, "Tolstopaltsevo", 3000});
  tc.AddDistance({"Marushkino"s, "Rasskazovka", 9900});
  tc.AddDistance({"Marushkino"s, "Marushkino", 100});
  tc.AddBus({"828"s, {"Biryulyovo"sv, "Universam"sv, "Rasskazovka"sv, "Biryulyovo"sv},
             RouteType::CIRCULAR});
  tc.AddBus({"750"s, {"Tolstopaltsevo"sv, "Marushkino"sv, "Marushkino"sv, "Rasskazovka"sv},
             RouteType::LINEAR});
}

void TestBuildRoute() {
  TransportCatalogue tc;
  AddCircularAndLinearBuses(tc);
  TransportRouter tr(tc, RoutingSettings{30, 2});
  {
    auto route = tr.BuildRoute("Tolstopaltsevo"sv, "Marushkino"sv);
    ASSERT_EQUAL(route->total_time, 9.8);
    ASSERT_EQUAL(route->items.size(), 2);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).type, "Wait"s);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).stop, "Tolstopaltsevo"s);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).time, 2);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).type, "Bus"s);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).bus, "750"s);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).time, 7.8);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).span_count, 1);
  }
  {
    auto route = tr.BuildRoute("Marushkino"sv, "Tolstopaltsevo"sv);
    ASSERT_EQUAL(route->total_time, 8);
    ASSERT_EQUAL(route->items.size(), 2);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).type, "Wait"s);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).stop, "Marushkino"s);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).time, 2);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).type, "Bus"s);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).bus, "750"s);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).time, 6);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).span_count, 1);
  }
  {
    auto route = tr.BuildRoute("Universam"sv, "Tolstopaltsevo"sv);
    ASSERT_EQUAL(route->total_time, 41.2);
    ASSERT_EQUAL(route->items.size(), 4);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).type, "Wait"s);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).stop, "Universam"s);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[0]).time, 2);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).type, "Bus"s);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).bus, "828"s);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).time, 11.2);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[1]).span_count, 1);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[2]).type, "Wait"s);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[2]).stop, "Rasskazovka"s);
    ASSERT_EQUAL(get<WaitRouteItem>(route->items[2]).time, 2);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[3]).type, "Bus"s);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[3]).bus, "750"s);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[3]).time, 26);
    ASSERT_EQUAL(get<BusRouteItem>(route->items[3]).span_count, 3);
  }
}

void TestGetRoutingSettings() {
  TransportCatalogue tc;
  AddCircularAndLinearBuses(tc);
  TransportRouter tr(tc, RoutingSettings{30, 2});
  const auto settings = tr.GetRoutingSettings();
  ASSERT_EQUAL(settings.bus_velocity, 500);
  ASSERT_EQUAL(settings.bus_wait_time, 2);
}

void TestGetGraph() {
  TransportCatalogue tc;
  AddCircularAndLinearBuses(tc);
  TransportRouter tr(tc, RoutingSettings{30, 2});
  const auto &graph = tr.GetGraph();
  ASSERT_EQUAL(graph.GetVertexCount(), 5);
}

void TestGetEdge() {
  TransportCatalogue tc;
  AddCircularAndLinearBuses(tc);
  TransportRouter tr(tc, RoutingSettings{30, 2});
  const auto &edge0 = tr.GetEdge(0);
  ASSERT_EQUAL(edge0.first.time, 7.8);
  ASSERT_EQUAL(edge0.first.bus, "750"sv);
  ASSERT_EQUAL(edge0.first.span_count, 1);
  ASSERT_EQUAL(edge0.second, "Tolstopaltsevo"sv);
  const auto &edge10 = tr.GetEdge(10);
  ASSERT_EQUAL(edge10.first.time, 19.8);
  ASSERT_EQUAL(edge10.first.bus, "750"sv);
  ASSERT_EQUAL(edge10.first.span_count, 1);
  ASSERT_EQUAL(edge10.second, "Marushkino"sv);
}

void TestGetVertexIdByStopName() {
  TransportCatalogue tc;
  AddCircularAndLinearBuses(tc);
  TransportRouter tr(tc, RoutingSettings{30, 2});
  const auto &graph = tr.GetGraph();
  ASSERT_EQUAL(tr.GetVertexIdByStopName("Biryulyovo"sv).value(), 3);
  ASSERT_EQUAL(tr.GetVertexIdByStopName("Marushkino"sv).value(), 1);
  ASSERT(!tr.GetVertexIdByStopName("Marush"sv).has_value());
}

}

void TransportRouterRunTest() {
  TestBuildRoute();
  TestGetRoutingSettings();
  TestGetGraph();
  TestGetEdge();
  TestGetVertexIdByStopName();
}
