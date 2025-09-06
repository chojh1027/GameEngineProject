#include "my_engine/Component.h"

Component::Component(GameObject* gameObject) 
	: gameObject(gameObject) {}

Component::~Component() {}