#pragma once

#include "DrawableObject.h"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include <GL/glew.h>
#include <SDL_ttf.h>
#include <string>

class TextObject :public DrawableObject
{
private:
	unsigned int texture;
	bool ownsTexture = true;

public:
	TextObject();
	~TextObject();
	void Render(glm::mat4 globalModelTransform);
	void Update(float deltaTime);
	void LoadText(string text, SDL_Color textColor, int fontSize);
	void LoadTextWrapped(string text, SDL_Color textColor, int fontSize, int maxWidth);
	unsigned int GetTextureId() const { return texture; }
	void SetTextureId(unsigned int id) { 
		if (texture != 0 && ownsTexture) { glDeleteTextures(1, &texture); }
		texture = id; ownsTexture = false; 
	}

};