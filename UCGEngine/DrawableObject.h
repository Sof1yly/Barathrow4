#pragma once
#include "glm.hpp"
#include <vector>

using namespace std;

class DrawableObject
{
protected:
	glm::vec3 pos;
	glm::vec3 size;
	float angle;
	float alpha = 1.0f;   // 0.0 = fully transparent, 1.0 = fully opaque

	// Parent-child transform system
	DrawableObject* parent = nullptr;
	glm::vec3 localPos;  // Normalized position relative to parent (fraction of parent size)

public:
	glm::mat4 getTransform();
	glm::mat4 getLocalTransform();

	DrawableObject();
	~DrawableObject();
	virtual void Render(glm::mat4 globalModelTransform) = 0;
	virtual void Update(float deltaTime);
	void SetSize(float sizeX, float sizeY);
	void SetPosition(glm::vec3 newPosition);
	void Translate(glm::vec3 moveDistance);
	void SetRotate(float angle);

	glm::vec3 GetPosition() const { return pos; }
	glm::vec2 GetSize() const { return size; }
	float GetRotate() const { return angle; }
	float GetAlpha() const { return alpha; }
	void SetAlpha(float a) { alpha = a; }

	// Parent-child relationship
	void SetParent(DrawableObject* p);
	DrawableObject* GetParent() const { return parent; }
	void SetLocalPosition(glm::vec3 localPos);
	glm::vec3 GetLocalPosition() const { return localPos; }

	// Update world position based on parent
	void UpdateFromParent();

	//Rotate, Scale ???

};

