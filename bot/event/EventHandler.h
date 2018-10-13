#pragma once

#include <utllinkedlist.h>

class EventInfo;
class CUtlString;
class EventHandler;

typedef CUtlLinkedList<EventHandler*> EventListeners;

class EventHandler {
public:

	static void notifyListeners(EventInfo* event);

	static void resetHandlers() {
		listeners.RemoveAll();
	}

	EventHandler() {
		listeners.AddToTail(this);
	}

	virtual ~EventHandler();

	/**
	 * @param event Event that is subscribed to.
	 *
	 * @return True if the event is consumed by the listener.
	 */
	virtual bool handle(EventInfo* event) = 0;
private:
	static EventListeners listeners;
};

