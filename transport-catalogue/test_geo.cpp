#include "testing_library.h"
#include "geo.h"

using namespace std;

using namespace geo;

namespace {

void TestComputeDistance() {
  Coordinates from{55.611087, 37.20829};
  Coordinates to{55.595884, 37.209755};
  ASSERT(abs(ComputeDistance(from, to) - 1693.0) < 1e-3);
}

}

void GeoRunTest() {
  TestComputeDistance();
}
