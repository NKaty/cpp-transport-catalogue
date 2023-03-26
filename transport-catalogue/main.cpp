#include "json_reader.h"

#include <iostream>

using namespace std;

int main() {
  transport_catalogue::TransportCatalogue catalogue;
  request::RequestHandler request_handler(catalogue);
  request_handler.ProcessJsonRequests(cin, cout);

  return 0;
}
