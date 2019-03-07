#pragma once

#include <utllinkedlist.h>

class EventInfo;
class CUtlString;
class EventHandler;


class EventHandler {
public:

	static void notifyListeners(EventInfo* event);

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
	typedef CUtlLinkedList<EventHandler*> EventListeners;

	static EventListeners listeners;
};

