#include "testing_library.h"
#include "ranges.h"

using namespace std;

using namespace ranges;

namespace {

void TestBeginEnd() {
  {
    vector<int> numbers{1, 2, 3, 4, 5};
    Range<vector<int>::iterator> range(numbers.begin(), numbers.end());
    ASSERT_EQUAL(*range.begin(), 1);
    ASSERT_EQUAL(*(--range.end()), 5);
  }
}

void TestAsRange() {
  {
    vector<int> numbers{1, 2, 3, 4, 5};
    auto range = AsRange(numbers);
    ASSERT_EQUAL(*range.begin(), 1);
    ASSERT_EQUAL(*(--range.end()), 5);
  }
}

void RangesRunTest() {
  TestBeginEnd();
  TestAsRange();
}

}

//int main() {
//  RangesRunTest();
//}
