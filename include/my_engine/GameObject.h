#pragma once
#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include "my_engine/Transform.h"
#include "my_engine/Component.h"

// ���� ������Ʈ Ŭ���� ����
// �������� ���:
// Transform	- Position, Rotation, Scale
// Components	- Component Ŭ������ ��ӹ޴� Ŭ�������� �迭
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