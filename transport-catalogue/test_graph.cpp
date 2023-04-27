#include "testing_library.h"
#include "graph.h"

using namespace std;

using namespace graph;

namespace {

void TestAddGetEdge() {
  DirectedWeightedGraph<int> graph(10);
  {
    const auto id = graph.AddEdge(Edge<int>{1, 2, 3});
    const auto edge = graph.GetEdge(id);
    ASSERT_EQUAL(edge.weight, 3);
    ASSERT_EQUAL(edge.from, 1);
    ASSERT_EQUAL(edge.to, 2);
  }
  {
    const auto id = graph.AddEdge(Edge<int>{2, 1, 5});
    const auto edge = graph.GetEdge(id);
    ASSERT_EQUAL(edge.weight, 5);
    ASSERT_EQUAL(edge.from, 2);
    ASSERT_EQUAL(edge.to, 1);
  }
}

void TestGetEdgeCount() {
  DirectedWeightedGraph<int> graph(10);
  ASSERT_EQUAL(graph.GetEdgeCount(), 0);
  {
    graph.AddEdge(Edge<int>{1, 2, 3});
    ASSERT_EQUAL(graph.GetEdgeCount(), 1);
  }
  {
    graph.AddEdge(Edge<int>{2, 1, 5});
    ASSERT_EQUAL(graph.GetEdgeCount(), 2);
  }
}

void TestGetVertexCount() {
  DirectedWeightedGraph<int> graph(10);
  ASSERT_EQUAL(graph.GetVertexCount(), 10);
}

void TestGetIncidentEdges() {
  DirectedWeightedGraph<int> graph(3);
  {
    const auto edges= graph.GetIncidentEdges(1);
    ASSERT_EQUAL(edges.end() - edges.begin(), 0);
  }
  {
    graph.AddEdge(Edge<int>{1, 2, 3});
    graph.AddEdge(Edge<int>{2, 1, 5});
    graph.AddEdge(Edge<int>{1, 1, 6});
    graph.AddEdge(Edge<int>{1, 19, 1});
    const auto edges = graph.GetIncidentEdges(1);
    ASSERT_EQUAL(edges.end() - edges.begin(), 3);
  }
}

void GraphRunTest() {
  TestAddGetEdge();
  TestGetEdgeCount();
  TestGetVertexCount();
  TestGetIncidentEdges();
}

}

//int main() {
//  GraphRunTest();
//}
