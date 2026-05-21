#pragma once

#include "DrawableObject.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "TriangleMeshVbo.h"
class GameObject :public DrawableObject
{
	glm::vec4 color;
	int type;

public:
	GameObject();
	~GameObject();
	void SetColor(float r, float g, float b,float a = 1.0f);
	void Render(glm::mat4 globalModelTransform);
};

