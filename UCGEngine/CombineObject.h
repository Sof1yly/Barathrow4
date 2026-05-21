#pragma once

#include "DrawableObject.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
class CombineObject :public DrawableObject
{
	glm::vec3 color;
	glm::vec3 color2;
	glm::vec4 color3;

public:
	CombineObject();
	~CombineObject();
	void SetColor(float r, float g, float b);
	void SetColor2(float r, float g, float b);
	void SetColor3(float r, float g, float b, float a = 1.0f);
	void Render(glm::mat4 globalModelTransform);
};
