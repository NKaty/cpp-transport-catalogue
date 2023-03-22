#include "json_reader.h"

#include <iostream>

using namespace std;

int main() {
  transport_catalogue::TransportCatalogue catalogue;
  request::ProcessJsonRequests(catalogue, cin, cout);

  return 0;
}
