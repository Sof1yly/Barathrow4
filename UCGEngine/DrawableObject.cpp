
#include "DrawableObject.h"

#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include <cmath>



glm::mat4 DrawableObject::getTransform()
{
	glm::mat4 transform = glm::mat4(1.0);
	transform = glm::translate(transform, glm::vec3(pos.x, pos.y, 0));
	transform = glm::rotate(transform, angle * (3.14f / 180.0f), glm::vec3(0, 0, 1));
	transform = glm::scale(transform, glm::vec3(size.x, size.y, 1));

	return transform;
}

glm::mat4 DrawableObject::getLocalTransform()
{
	glm::mat4 transform = glm::mat4(1.0);
	transform = glm::translate(transform, glm::vec3(localPos.x, localPos.y, 0));
	transform = glm::rotate(transform, angle * (3.14f / 180.0f), glm::vec3(0, 0, 1));
	transform = glm::scale(transform, glm::vec3(size.x, size.y, 1));

	return transform;
}

DrawableObject::DrawableObject()
{
	pos = glm::vec3(0.0, 0.0, 0.0);
	size = glm::vec3(1.0, 1.0, 1.0);
	angle = 0.0f;
	localPos = glm::vec3(0.0, 0.0, 0.0);
}


DrawableObject::~DrawableObject()
{
}

void DrawableObject::SetSize(float sizeX, float sizeY)
{
	size = glm::vec3(sizeX, sizeY, 1);
}

void DrawableObject::SetPosition(glm::vec3 newPosition)
{
	pos = newPosition;
}

void DrawableObject::Translate(glm::vec3 moveDistance)
{
	pos = pos + moveDistance;
}

void DrawableObject::Update(float deltaTime)
{

}

void DrawableObject::SetRotate(float angle) {
	this->angle = angle;
}

void DrawableObject::SetParent(DrawableObject* p)
{
	parent = p;
	if (parent != nullptr)
	{
		UpdateFromParent();
	}
}

void DrawableObject::SetLocalPosition(glm::vec3 lPos)
{
	localPos = lPos;
	if (parent != nullptr)
	{
		UpdateFromParent();
	}
}

void DrawableObject::UpdateFromParent()
{
	if (parent != nullptr)
	{
		// localPos is normalized: (0,0) = center, offset is fraction of parent size
		glm::vec2 parentSize = parent->GetSize();
		pos = parent->GetPosition() + glm::vec3(
			localPos.x * fabs(parentSize.x),
			localPos.y * fabs(parentSize.y),
			0.0f);
	}
}

