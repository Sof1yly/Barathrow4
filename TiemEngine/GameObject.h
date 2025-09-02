#pragma once

#include "DrawableObject.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "TriangleMeshVbo.h"
class GameObject :public DrawableObject
{
	glm::vec3 color;
	int type;

public:
	GameObject();
	~GameObject();
	void SetColor(float r, float g, float b);
	void Render(glm::mat4 globalModelTransform);
};

