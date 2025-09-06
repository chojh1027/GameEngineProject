#pragma once
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "my_engine/Transform.h"
#include "my_engine/Component.h"

// 게임 오브젝트 클래스 정의
// 가져야할 요소:
// Transform	- Position, Rotation, Scale
// Components	- Component 클래스를 상속받는 클래스들의 배열
// Init()		
// Update()		
// Destroy()	

class GameObject {
public:
	Transform* transform = nullptr;

	GameObject() : components{} {
		transform = new Transform(this);
		components[0] = transform;
	}
	~GameObject() {}

	void Init() {}
	void Update(float deltaTime) {}
	void Destroy() {}

private:
	Component* components[10];
};


#endif // GAMEOBJECT_H