#pragma once
#ifndef COMPONENT_H
#define COMPONENT_H

class GameObject;
class Transform;

class Component {
public:
	GameObject* gameObject = nullptr;

	Component(GameObject* gameObject);
	~Component();

	virtual void Init() {}
	virtual void Start() {}
	virtual void Update(float deltaTime) {}
	virtual void Destroy() {}

	void SetActive(bool active) { isActive = active; }
	bool IsActive() const { return isActive; }

private:
	bool isActive = true;
};

#endif // COMPONENT_H