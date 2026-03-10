#pragma once

#include "DrawableObject.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include <string>
class ImageObject : public DrawableObject
{
	unsigned int texture = 0;
	float alpha = 1.0f;
	bool ownsTexture = true; 

public:
	ImageObject();
	~ImageObject();
	void SetTexture(string path);
	void SetAlpha(float a) { alpha = a; }
	float GetAlpha() const { return alpha; }
	unsigned int GetTextureId() const { return texture; }
	void SetTextureId(unsigned int id) { texture = id; ownsTexture = false; }
	void Render(glm::mat4 globalModelTransform);
};
