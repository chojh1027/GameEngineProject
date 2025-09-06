#pragma once
#ifndef COMPONENT_H
#define COMPONENT_H

class GameObject;
class Transform;

// 게임오브젝트에 붙는 컴포넌트 클래스
// 게임오브젝트의 생명주기에 맞춰서 동작
// Init(), Update(), Destroy() 생명주기 함수를 가짐
// 부모 게임오브젝트와 트랜스폼에 대한 포인터를 가짐

class Component {
public:
	GameObject* gameObject = nullptr;

	Component(GameObject* gameObject);
	~Component();

	virtual void Init() {}
	virtual void Update(float deltaTime) {}
	virtual void Destroy() {}

	void SetActive(bool active) { isActive = active; }
	bool IsActive() const { return isActive; }

private:
	bool isActive = true;
};

#endif // COMPONENT_H