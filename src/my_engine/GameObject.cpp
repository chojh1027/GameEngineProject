#include "my_engine/GameObject.h"
#include "my_engine/transform.h"

void GameObject::Init() {
	for (int i = 0; i < this->componentCount; i++)
		if (components[i] != nullptr)
			components[i]->Init();
}

void GameObject::Start() {
	for (int i = 0; i < this->componentCount; i++)
		if (components[i] != nullptr && components[i]->IsActive())
			components[i]->Start();
}

void GameObject::Update(float deltaTime) {
	for (int i = 0; i < this->componentCount; i++)
		if (components[i] != nullptr && components[i]->IsActive())
			components[i]->Update(deltaTime);
}

void GameObject::Destroy() {
	for (int i = 0; i < this->componentCount; i++)
		if (components[i] != nullptr)
			components[i]->Destroy();
}

void GameObject::AddComponent(Component* component)
{
	if (componentCount >= MAX_COMPONENT_COUNT)
		return;

	components[componentCount] = component;
	componentCount++;
}