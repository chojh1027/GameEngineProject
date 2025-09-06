#include "my_engine/transform.h"
#include "my_engine/GameObject.h"

void Transform::SetPosition(const Vec2& position) {
	this->position = position;
}

Vec2 Transform::GetPosition() const {
	return this->position;
}

void Transform::SetRotation(float rotation) {
	this->rotation = rotation;
}

float Transform::GetRotation() const {
	return this->rotation;
}

void Transform::SetScale(const Vec2& scale) {
	this->scale = scale;
}

Vec2 Transform::GetScale() const {
	return this->scale;
}