#include "testing_library.h"
#include "transport_catalogue.h"
#include "domain.h"
#include "geo.h"

using namespace std;

using namespace transport_catalogue;
using namespace detail;
using namespace geo;

namespace {

void AddCircularBus(TransportCatalogue &tc) {
  const Coordinates biryulyovo_coords{55.574371, 37.6517};
  const Coordinates universam_coords{55.587655, 37.645687};
  const Coordinates rasskazovka_coords{55.595579, 37.605757};
  tc.AddStop({"Biryulyovo"s, biryulyovo_coords});
  tc.AddStop({"Universam"s, universam_coords});
  tc.AddStop({"Rasskazovka"s, rasskazovka_coords});
  tc.AddDistance({"Biryulyovo"s, "Universam", 2400});
  tc.AddDistance({"Rasskazovka"s, "Universam", 5600});
  tc.AddDistance({"Biryulyovo"s, "Rasskazovka", 7500});
  vector<string_view> stops{"Biryulyovo"sv, "Universam"sv, "Rasskazovka"sv, "Biryulyovo"sv};
  tc.AddBus({"828"s, stops, RouteType::CIRCULAR});
}

void AddLinearBus(TransportCatalogue &tc) {
  const Coordinates tolstopaltsevo_coords{55.611087, 37.20829};
  const Coordinates marushkino_coords{55.595884, 37.209755};
  const Coordinates rasskazovka_coords{55.632761, 37.333324};
  tc.AddStop({"Tolstopaltsevo"s, tolstopaltsevo_coords});
  tc.AddStop({"Marushkino"s, marushkino_coords});
  tc.AddStop({"Rasskazovka"s, rasskazovka_coords});
  tc.AddDistance({"Tolstopaltsevo"s, "Marushkino", 3900});
  tc.AddDistance({"Marushkino"s, "Tolstopaltsevo", 3700});
  tc.AddDistance({"Marushkino"s, "Rasskazovka", 9900});
  tc.AddDistance({"Marushkino"s, "Marushkino", 100});
  vector<string_view> stops{"Tolstopaltsevo"sv, "Marushkino"sv, "Marushkino"sv, "Rasskazovka"sv};
  tc.AddBus({"750"s, stops, RouteType::LINEAR});
}

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
  tc.AddDistance({"Marushkino"s, "Rasskazovka", 9900});
  tc.AddDistance({"Marushkino"s, "Marushkino", 100});
  tc.AddBus({"828"s, {"Biryulyovo"sv, "Universam"sv, "Rasskazovka"sv, "Biryulyovo"sv},
             RouteType::CIRCULAR});
  tc.AddBus({"750"s, {"Tolstopaltsevo"sv, "Marushkino"sv, "Marushkino"sv, "Rasskazovka"sv},
             RouteType::LINEAR});
}

void TestAddStop() {
  TransportCatalogue tc;
  const Coordinates marushkino_coords{55.595884, 37.209755};
  const Coordinates universam_coords{55.587655, 37.645687};
  {
    ASSERT_EQUAL(tc.GetAllStops().size(), 0u);
  }
  {
    tc.AddStop({"Marushkino"s, marushkino_coords});
    ASSERT_EQUAL(tc.GetAllStops().size(), 1u);
    const auto stop = tc.FindStop("Marushkino"s);
    ASSERT_EQUAL(stop.name, "Marushkino"s);
    ASSERT_EQUAL(stop.coordinates.lng, marushkino_coords.lng);
    ASSERT_EQUAL(stop.coordinates.lat, marushkino_coords.lat);
    ASSERT_EQUAL(stop.buses_through_stop.size(), 0u);
  }
  {
    tc.AddStop({"Universam"s, universam_coords});
    ASSERT_EQUAL(tc.GetAllStops().size(), 2u);
    const auto stop = tc.FindStop("Universam"s);
    ASSERT_EQUAL(stop.name, "Universam"s);
    ASSERT_EQUAL(stop.coordinates.lng, universam_coords.lng);
    ASSERT_EQUAL(stop.coordinates.lat, universam_coords.lat);
    ASSERT_EQUAL(stop.buses_through_stop.size(), 0u);
  }
}

void TestAddCircularBus() {
  {
    TransportCatalogue tc;
    const Coordinates biryulyovo_coords{55.574371, 37.6517};
    const Coordinates universam_coords{55.587655, 37.645687};
    const Coordinates rossoshanskaya_coords{55.595579, 37.605757};
    tc.AddStop({"Biryulyovo"s, biryulyovo_coords});
    tc.AddStop({"Universam"s, universam_coords});
    tc.AddStop({"Rossoshanskaya"s, rossoshanskaya_coords});
    tc.AddDistance({"Biryulyovo"s, "Universam", 2400});
    tc.AddDistance({"Rossoshanskaya"s, "Universam", 5600});
    tc.AddDistance({"Biryulyovo"s, "Rossoshanskaya", 7500});
    vector<string_view> stops{"Biryulyovo"sv, "Universam"sv, "Rossoshanskaya"sv, "Biryulyovo"sv};
    tc.AddBus({"828"s, stops, RouteType::CIRCULAR});
    const auto bus = tc.FindBus("828"s);
    ASSERT_EQUAL(bus.name, "828"s);
    ASSERT_EQUAL(bus.stops_on_route, stops);
    ASSERT_EQUAL(static_cast<int>(bus.route_type), static_cast<int>(RouteType::CIRCULAR));
    ASSERT_EQUAL(bus.unique_stops_count, 3);
    ASSERT_EQUAL(bus.GetStopCount(), 4);
    ASSERT_EQUAL(bus.route_distance, 15500);
    ASSERT(abs(bus.curvature - 1.95908) < 1e-5);
  }
  {
    TransportCatalogue tc;
    const Coordinates biryulyovo_coords{55.574371, 37.6517};
    const Coordinates universam_coords{55.587655, 37.645687};
    const Coordinates rossoshanskaya_coords{55.595579, 37.605757};
    tc.AddStop({"Biryulyovo"s, biryulyovo_coords});
    tc.AddStop({"Universam"s, universam_coords});
    tc.AddStop({"Rossoshanskaya"s, rossoshanskaya_coords});
    tc.AddDistance({"Biryulyovo"s, "Universam", 2400});
    tc.AddDistance({"Rossoshanskaya"s, "Universam", 5600});
    tc.AddDistance({"Universam"s, "Rossoshanskaya", 6000});
    tc.AddDistance({"Biryulyovo"s, "Rossoshanskaya", 7500});
    vector<string_view> stops{"Biryulyovo"sv, "Universam"sv, "Rossoshanskaya"sv, "Biryulyovo"sv};
    tc.AddBus({"828"s, stops, RouteType::CIRCULAR});
    const auto bus = tc.FindBus("828"s);
    ASSERT_EQUAL(bus.name, "828"s);
    ASSERT_EQUAL(bus.stops_on_route, stops);
    ASSERT_EQUAL(static_cast<int>(bus.route_type), static_cast<int>(RouteType::CIRCULAR));
    ASSERT_EQUAL(bus.unique_stops_count, 3);
    ASSERT_EQUAL(bus.GetStopCount(), 4);
    ASSERT_EQUAL(bus.route_distance, 15900);
    ASSERT(abs(bus.curvature - 2.00964) < 1e-5);
  }
}

void TestAddLinearBus() {
  {
    TransportCatalogue tc;
    const Coordinates tolstopaltsevo_coords{55.611087, 37.20829};
    const Coordinates marushkino_coords{55.595884, 37.209755};
    const Coordinates rasskazovka_coords{55.632761, 37.333324};
    tc.AddStop({"Tolstopaltsevo"s, tolstopaltsevo_coords});
    tc.AddStop({"Marushkino"s, marushkino_coords});
    tc.AddStop({"Rasskazovka"s, rasskazovka_coords});
    tc.AddDistance({"Tolstopaltsevo"s, "Marushkino", 3900});
    tc.AddDistance({"Marushkino"s, "Rasskazovka", 9900});
    tc.AddDistance({"Marushkino"s, "Marushkino", 100});
    vector<string_view>
        stops{"Tolstopaltsevo"sv, "Marushkino"sv, "Marushkino"sv, "Rasskazovka"sv};
    tc.AddBus({"750"s, stops, RouteType::LINEAR});
    const auto bus = tc.FindBus("750"s);
    ASSERT_EQUAL(bus.name, "750"s);
    ASSERT_EQUAL(bus.stops_on_route, stops);
    ASSERT_EQUAL(static_cast<int>(bus.route_type), static_cast<int>(RouteType::LINEAR));
    ASSERT_EQUAL(bus.unique_stops_count, 3);
    ASSERT_EQUAL(bus.GetStopCount(), 7);
    ASSERT_EQUAL(bus.route_distance, 27800);
    ASSERT(abs(bus.curvature - 1.32764) < 1e-5);
  }
  {
    {
      TransportCatalogue tc;
      const Coordinates tolstopaltsevo_coords{55.611087, 37.20829};
      const Coordinates marushkino_coords{55.595884, 37.209755};
      const Coordinates rasskazovka_coords{55.632761, 37.333324};
      tc.AddStop({"Tolstopaltsevo"s, tolstopaltsevo_coords});
      tc.AddStop({"Marushkino"s, marushkino_coords});
      tc.AddStop({"Rasskazovka"s, rasskazovka_coords});
      tc.AddDistance({"Tolstopaltsevo"s, "Marushkino", 3900});
      tc.AddDistance({"Marushkino"s, "Rasskazovka", 9900});
      tc.AddDistance({"Rasskazovka"s, "Marushkino", 9500});
      tc.AddDistance({"Marushkino"s, "Marushkino", 100});
      vector<string_view>
          stops{"Tolstopaltsevo"sv, "Marushkino"sv, "Marushkino"sv, "Rasskazovka"sv};
      tc.AddBus({"750"s, stops, RouteType::LINEAR});
      const auto bus = tc.FindBus("750"s);
      ASSERT_EQUAL(bus.name, "750"s);
      ASSERT_EQUAL(bus.stops_on_route, stops);
      ASSERT_EQUAL(static_cast<int>(bus.route_type), static_cast<int>(RouteType::LINEAR));
      ASSERT_EQUAL(bus.unique_stops_count, 3);
      ASSERT_EQUAL(bus.GetStopCount(), 7);
      ASSERT_EQUAL(bus.route_distance, 27400);
      ASSERT(abs(bus.curvature - 1.30853) < 1e-5);
    }
  }
}

void TestGetBusesThroughStop() {
  TransportCatalogue tc;
  AddCircularAndLinearBuses(tc);
  set<string_view> marushkino_buses{"750"sv};
  ASSERT_EQUAL(*tc.GetBusesThroughStop("Marushkino"s), marushkino_buses);
  set<string_view> rasskazovka_buses{"750"sv, "828"sv};
  ASSERT_EQUAL(*tc.GetBusesThroughStop("Rasskazovka"s), rasskazovka_buses);
  ASSERT(!tc.GetBusesThroughStop("SomeStop"s));
}

void TestGetAllBuses() {
  TransportCatalogue tc;
  AddCircularAndLinearBuses(tc);
  auto buses = tc.GetAllBuses();
  ASSERT_EQUAL(buses.size(), 2);
  ASSERT_EQUAL(buses[0]->name, "750"s);
  ASSERT_EQUAL(buses[1]->name, "828"s);

}

void TestGetAllStops() {
  TransportCatalogue tc;
  AddCircularAndLinearBuses(tc);
  auto stops = tc.GetAllStops();
  ASSERT_EQUAL(stops.size(), 5);
  ASSERT(stops.count("Tolstopaltsevo"s));
  ASSERT(stops.count("Marushkino"s));
  ASSERT(stops.count("Rasskazovka"s));
  ASSERT(stops.count("Biryulyovo"s));
  ASSERT(stops.count("Universam"s));
}

void TestGetRouteStatForCircularBus() {
  TransportCatalogue tc;
  AddCircularBus(tc);
  auto bus = tc.GetRouteStat("828"s);
  ASSERT_EQUAL(bus->bus_name, "828"s);
  ASSERT_EQUAL(bus->stops_count, 4);
  ASSERT_EQUAL(bus->unique_stops_count, 3);
  ASSERT_EQUAL(bus->route_distance, 15500);
  ASSERT(abs(bus->curvature - 1.95908) < 1e-5);
}

void TestGetRouteStatForLinearBus() {
  TransportCatalogue tc;
  AddLinearBus(tc);
  auto bus = tc.GetRouteStat("750"s);
  ASSERT_EQUAL(bus->bus_name, "750"s);
  ASSERT_EQUAL(bus->stops_count, 7);
  ASSERT_EQUAL(bus->unique_stops_count, 3);
  ASSERT_EQUAL(bus->route_distance, 27600);
  ASSERT(abs(bus->curvature - 1.31808) < 1e-5);
}

void TestGetDistanceBetweenStops() {
  TransportCatalogue tc;
  AddLinearBus(tc);
  ASSERT_EQUAL(tc.GetDistanceBetweenStops("Tolstopaltsevo"sv, "Marushkino"sv).value(), 3900);
  ASSERT_EQUAL(tc.GetDistanceBetweenStops("Marushkino"sv, "Tolstopaltsevo"sv).value(), 3700);
  ASSERT_EQUAL(tc.GetDistanceBetweenStops("Marushkino"sv, "Marushkino"sv).value(), 100);
  ASSERT(!tc.GetDistanceBetweenStops("Tolstopaltsevo"sv, "Tolstopaltsevo"sv).has_value());
}

void TransportCatalogueRunTest() {
  TestAddStop();
  TestAddCircularBus();
  TestAddLinearBus();
  TestGetBusesThroughStop();
  TestGetAllBuses();
  TestGetAllStops();
  TestGetRouteStatForCircularBus();
  TestGetRouteStatForLinearBus();
  TestGetDistanceBetweenStops();
}

}

//int main() {
//  TransportCatalogueRunTest();
//}
