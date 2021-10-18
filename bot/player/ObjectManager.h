#pragma once

#include <list>

class ObjectManager {
public:
	class Object {
	public:
		virtual ~Object() {
			manager.objects.remove_if([this](const Object *item) {
				return item == this;
			});
		}
	protected:
		ObjectManager &manager;

		Object(ObjectManager &manager) :
				manager(manager) {
			manager.objects.push_back(this);
		}
	};

protected:
	std::list<Object*> objects;
};
