#include "pch.h"
#include "TextObject.h"
#include "GameEngine.h"
#include "SquareMeshVbo.h"

TextObject::TextObject()
{
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


TextObject::~TextObject()
{
	if (texture != 0 && ownsTexture) {
		glDeleteTextures(1, &texture);
		texture = 0;
	}
}

void TextObject::Render(glm::mat4 globalModelTransform)
{
	SquareMeshVbo *squareMesh = dynamic_cast<SquareMeshVbo *> (GameEngine::GetInstance()->GetRenderer()->GetMesh(SquareMeshVbo::MESH_NAME));

	GLuint modelMatixId = GameEngine::GetInstance()->GetRenderer()->GetModelMatrixAttrId();
	GLuint modeId = GameEngine::GetInstance()->GetRenderer()->GetModeUniformId();
	GLuint colorId = GameEngine::GetInstance()->GetRenderer()->GetColorUniformId();

	glBindTexture(GL_TEXTURE_2D, texture);

	if (modelMatixId == -1) {
		cout << "Error: Can't perform transformation " << endl;
		return;
	}

	glm::mat4 currentMatrix = this->getTransform();

	if (squareMesh != nullptr) {

		currentMatrix = globalModelTransform * currentMatrix;
		glUniformMatrix4fv(modelMatixId, 1, GL_FALSE, glm::value_ptr(currentMatrix));
		glUniform4f(colorId, 1.0f, 1.0f, 1.0f, alpha);
		glUniform1i(modeId, 1);
		//squareMesh->resetTexcoord();
		squareMesh->Render();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void TextObject::Update(float deltaTime)
{
}

void TextObject::LoadText(string text, SDL_Color textColor, int fontSize)
{
 if (text.empty())
	{
		SetSize(0.0f, 0.0f);
		return;
	}

	glBindTexture( GL_TEXTURE_2D, texture);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	TTF_Font * font = TTF_OpenFont("../Resource/Texture/fonts/Monopixies.ttf", fontSize);
	if (font)
	{
		SDL_Surface* surfaceMessage = TTF_RenderText_Blended(font, text.c_str(), textColor);
     if (!surfaceMessage)
		{
			SetSize(0.0f, 0.0f);
			TTF_CloseFont(font);
			return;
		}
		GLenum fmt = (surfaceMessage->format->Rmask == 0x000000FF) ? GL_RGBA : GL_BGRA;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surfaceMessage->w, surfaceMessage->h, 0, fmt, GL_UNSIGNED_BYTE, surfaceMessage->pixels);
		SetSize(surfaceMessage->w, -surfaceMessage->h);
		SDL_FreeSurface(surfaceMessage);
		TTF_CloseFont(font);
	}
	else
	{
		cout << "Error: " << TTF_GetError() << endl;
		return;
	}
	
}

void TextObject::LoadTextWrapped(string text, SDL_Color textColor, int fontSize, int maxWidth)
{
  if (text.empty())
	{
		SetSize(0.0f, 0.0f);
		return;
	}

	glBindTexture(GL_TEXTURE_2D, texture);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	TTF_Font* font = TTF_OpenFont("../Resource/Texture/fonts/Monopixies.ttf", fontSize);
	if (font)
	{
		SDL_Surface* surfaceMessage = TTF_RenderText_Blended_Wrapped(font, text.c_str(), textColor, maxWidth);
		if (surfaceMessage)
		{
			GLenum fmt = (surfaceMessage->format->Rmask == 0x000000FF) ? GL_RGBA : GL_BGRA;
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surfaceMessage->w, surfaceMessage->h, 0, fmt, GL_UNSIGNED_BYTE, surfaceMessage->pixels);
			SetSize(surfaceMessage->w, -surfaceMessage->h);
			SDL_FreeSurface(surfaceMessage);
		}
		TTF_CloseFont(font);
	}
	else
	{
		cout << "Error: " << TTF_GetError() << endl;
		return;
	}
}