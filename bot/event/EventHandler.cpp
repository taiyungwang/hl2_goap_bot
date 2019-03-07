#include "EventHandler.h"

#include <utlstring.h>

EventHandler::EventListeners EventHandler::listeners;

EventHandler::~EventHandler() {
	FOR_EACH_LL(listeners, i)
	{
		if (listeners[i] == this) {
			listeners.Remove(i);
			break;
		}
	}
}

void EventHandler::notifyListeners(EventInfo* event) {
	FOR_EACH_LL(listeners, i) {
		if (listeners[i]->handle(event)) {
			break;
		}
	}
}

