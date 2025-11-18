#pragma once
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "my_engine/Constant.h"
#include "my_engine/Transform.h"


// Transform	- Position, Rotation, Scale
// Components	- Component array
// Init()		
// Update()		
// Destroy()	

class GameObject {
public:
	Transform* transform = nullptr;

	GameObject() {
		transform = new Transform(this);
		components[0] = transform;
	}
	GameObject(const Transform& t) {
		transform = new Transform(this, t.GetPosition(), t.GetRotation(), t.GetScale());
		components[0] = transform;
	}
	~GameObject() {}

void Init();
void Start();
void Update(float deltaTime);
void FixedUpdate(float fixedDeltaTime);
void Destroy();

	void AddComponent(Component* component);

private:
	Component* components[MAX_COMPONENT_COUNT];
	int componentCount = 1;
};


#endif // GAMEOBJECT_H