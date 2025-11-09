#pragma once

#include <vector>
#include "DrawableObject.h"
#include "GLRenderer.h"
#include "GameData.h"
#include <GL/glew.h>

using namespace std;
class GameEngine
{
	static GameEngine* instance;
	int winWidth, winHeight;
	int deltaTime;

	float areaW, areaH;
	
	GLRenderer *renderer;
	GameEngine();
public:
	static GameEngine* GetInstance();
	GLRenderer * GetRenderer();
	void Init(int width, int height);
	void SetDrawArea(float left, float right, float bottom, float top);
	float GetDrawAreaWidth();
	float GetDrawAreaHeight();
	void SetBackgroundColor(float r, float g, float b);
	void AddMesh(string name, MeshVbo* mesh);
	void ClearMesh();
	void Render(vector<DrawableObject*> renderObjects);
	int GetWindowWidth();
	int GetWindowHeight();
	int GetDeltaTime();
	void SetDeltaTime(int dt);
};