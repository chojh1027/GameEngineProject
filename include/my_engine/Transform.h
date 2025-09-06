#pragma once
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "box2d-lite/MathUtils.h"
#include "my_engine/Component.h"

class GameObject;

// 트랜스폼 클래스 정의
class Transform : public Component {
public:
	Transform(GameObject* gameObject) : Component(gameObject) {}
	Transform(GameObject* gameObject, const Vec2& pos, float rot, const Vec2& scl)
		: Component(gameObject), position(pos), rotation(rot), scale(scl) {}
	~Transform() {}

	Vec2	GetPosition() const;
	void	SetPosition(const Vec2& pos);

	float	GetRotation() const;
	void	SetRotation(float rot);

	Vec2	GetScale() const;
	void	SetScale(const Vec2& scl);

private:

	Vec2	position = Vec2(0.0f, 0.0f);
	float	rotation = 0.0f;
	Vec2	scale = Vec2(1.0f, 1.0f);
};

#endif // TRANSFORM_H