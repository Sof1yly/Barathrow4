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

public:
	glm::mat4 getTransform();

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

	//Rotate, Scale ???

};

