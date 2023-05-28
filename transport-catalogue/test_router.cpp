#include "testing_library.h"
#include "router.h"

using namespace std;

using namespace graph;

namespace {

void TestBuildRoute() {
  {
    DirectedWeightedGraph<int> graph(15);
    graph.AddEdge(Edge<int>{1, 2, 3});
    graph.AddEdge(Edge<int>{2, 1, 5});
    graph.AddEdge(Edge<int>{2, 3, 4});
    graph.AddEdge(Edge<int>{3, 4, 1});
    graph.AddEdge(Edge<int>{3, 4, 10});
    Router router(graph);
    const auto route = router.BuildRoute(1, 4);
    ASSERT_EQUAL(route->weight, 8);
  }
  {
    DirectedWeightedGraph<int> graph(15);
    graph.AddEdge(Edge<int>{1, 2, 3});
    graph.AddEdge(Edge<int>{2, 1, 5});
    graph.AddEdge(Edge<int>{2, 3, 4});
    graph.AddEdge(Edge<int>{3, 2, 3});
    graph.AddEdge(Edge<int>{3, 4, 1});
    graph.AddEdge(Edge<int>{3, 4, 10});
    Router router(graph);
    const auto route = router.BuildRoute(3, 1);
    ASSERT_EQUAL(route->weight, 8);
  }
  {
    DirectedWeightedGraph<int> graph(15);
    graph.AddEdge(Edge<int>{1, 2, 3});
    graph.AddEdge(Edge<int>{2, 1, 5});
    graph.AddEdge(Edge<int>{2, 3, 4});
    graph.AddEdge(Edge<int>{3, 2, 3});
    graph.AddEdge(Edge<int>{3, 4, 1});
    graph.AddEdge(Edge<int>{3, 4, 10});
    graph.AddEdge(Edge<int>{5, 7, 10});
    Router router(graph);
    const auto route = router.BuildRoute(5, 1);
    ASSERT(!route.has_value());
  }
}

}

void RouterRunTest() {
  TestBuildRoute();
}
