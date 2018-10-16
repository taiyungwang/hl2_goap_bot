#include "ValveSDKTest.h"
#include <utlpriorityqueue.h>

struct Element {
	int idx;
	float val;
};

void ValveSDKTest::testPriorityQ() {
	Element elements[] = { Element { 0, 0.5f }, Element { 1, 0.1f }, Element {
			2, 0.9f } };
	CUtlPriorityQueue<Element> q;
	q.SetLessFunc([] (const Element& e1, const Element& e2) {
		return e1.val > e2.val;
	});
	for (int i = 0; i < 3; i++) {
		q.Insert(elements[i]);
	}
	int expected[] = { 1, 0, 2 };
	for (int i = 0; i < 3; i++) {
		TS_ASSERT_EQUALS(expected[i], q.ElementAtHead().idx);
		q.RemoveAtHead();
	}
}
