
#include "GameEngine.h"

GameEngine* GameEngine::instance = nullptr;

GameEngine::GameEngine()
{
	renderer = nullptr;
}

GameEngine * GameEngine::GetInstance()
{
	if (instance == nullptr) {
		instance = new GameEngine();
	}
	return instance;
}

GLRenderer * GameEngine::GetRenderer()
{
	return this->renderer;
}

/*void GameEngine::HandleMouse(int x, int y)
{
	float realX, realY;
	realX = -3 + x * (6.0 / winWidth);
	realY = -3 + (winHeight - y) * (6.0 / winHeight);
	if (this->objects.size() > 0) {
		DrawableObject *obj = this->objects.at(0);
		obj->SetPosition(glm::vec3(realX, realY, 0));
	}
}*/

/*void GameEngine::HandleKey(char ch)
{
	if (this->objects.size() > 0) {
		DrawableObject *obj = this->objects.at(0);
		switch (ch) {
		case 'u': obj->Translate(glm::vec3(0, 0.3, 0)); break;
		case 'd': obj->Translate(glm::vec3(0, -0.3, 0)); break;
		case 'l': obj->Translate(glm::vec3(-0.3, 0, 0)); break;
		case 'r': obj->Translate(glm::vec3(0.3, 0, 0)); break;
		}
	}
}*/

void GameEngine::Init(int width, int height)
{
	winWidth = width;
	winHeight = height;
	renderer = new GLRenderer(width, height);
	renderer->InitGL("../Resource/Shader/vertext.shd", "../Resource/Shader/fragment.shd");
	SetDrawArea(-1980, 1980, -1080, 1080);
	SetBackgroundColor(1.0f, 1.0f, 200.0f / 255);

}

void GameEngine::Render(vector<DrawableObject*> renderObjects)
{
	this->GetRenderer()->Render(renderObjects);
}

void GameEngine::SetDrawArea(float left, float right, float bottom, float top)
{
	renderer->SetOrthoProjection(-990, 990, -540, 540);
}

void GameEngine::SetBackgroundColor(float r, float g, float b)
{
	renderer->SetClearColor(1.0f, 1.0f, 200.0f / 255);
}

void GameEngine::AddMesh(string name, MeshVbo* mesh)
{
	renderer->AddMesh(name, mesh);
}

void GameEngine::ClearMesh()
{
	renderer->ClearMesh();
}

int GameEngine::GetWindowWidth()
{
	return winWidth;
}

int GameEngine::GetWindowHeight()
{
	return winHeight;
}

int GameEngine::GetDeltaTime()
{
	return deltaTime;
}

void GameEngine::SetDeltaTime(int dt)
{
	deltaTime = dt;
}