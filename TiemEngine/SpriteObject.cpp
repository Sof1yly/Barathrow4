
#include "SpriteObject.h"
#include "GameEngine.h"
#include "SpriteMeshVbo.h"

SpriteObject::SpriteObject(string fileName, int row, int column)
{
	texture = GameEngine::GetInstance()->GetRenderer()->LoadTexture(fileName);

	this->rowMax = row;
	this->columnMax = column;
	this->currentRow = 0;
	this->currentColumn = 0;
	this->GenUV();

	this->startRow = 0;
	this->startColumn = 0;
	this->loopMax = 1;
	this->loopCount = 0;
	this->animationTime = 0;
	this->timeCount = 0;
}

SpriteObject::~SpriteObject()
{
}

void SpriteObject::Render(glm::mat4 globalModelTransform)
{
	SpriteMeshVbo *spriteMesh = dynamic_cast<SpriteMeshVbo *> (GameEngine::GetInstance()->GetRenderer()->GetMesh(SpriteMeshVbo::MESH_NAME));

	GLuint modelMatixId = GameEngine::GetInstance()->GetRenderer()->GetModelMatrixAttrId();
	GLuint modeId = GameEngine::GetInstance()->GetRenderer()->GetModeUniformId();

	glBindTexture(GL_TEXTURE_2D, texture);
	if (modelMatixId == -1) {
		cout << "Error: Can't perform transformation " << endl;
		return;
	}

	glm::mat4 currentMatrix = this->getTransform();

	if (spriteMesh != nullptr) {

		currentMatrix = globalModelTransform * currentMatrix;
		glUniformMatrix4fv(modelMatixId, 1, GL_FALSE, glm::value_ptr(currentMatrix));
		glUniform1i(modeId, 1);
		spriteMesh->AdjustTexcoord(uv);
		spriteMesh->Render();
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void SpriteObject::Update(float deltaTime)
{
	timeCount += deltaTime;
	if (timeCount > animationTime)
	{
		this->NextAnimation();
		this->GenUV();
		timeCount = 0;
	}

	
}

void SpriteObject::GenUV()
{
	float frameW = 1.0f / columnMax;
	float frameH = 1.0f / rowMax;

	float u0 = currentColumn * frameW;
	float v0 = currentRow * frameH;

	float u1 = u0 + frameW;
	float v1 = v0 + frameH;

	uv[0] = u0; uv[1] = v0;
	uv[2] = u1; uv[3] = v0;
	uv[4] = u1; uv[5] = v1;
	uv[6] = u0; uv[7] = v1;
}

void SpriteObject::SetAnimationLoop(int startRow, int startColumn, int howManyFrame, int delayBetaweenFrame)
{
	this->startRow = startRow;
	this->startColumn = startColumn;
	this->loopMax = howManyFrame;
	this->loopCount = 0;
	this->animationTime = delayBetaweenFrame;

	currentColumn = startColumn;
	currentRow = startRow;

	GenUV();
}

void SpriteObject::NextAnimation()
{
	loopCount++;
	if (loopCount < loopMax)
	{
		currentColumn++;
		if (currentColumn == columnMax)
		{
			currentColumn = 0;
			currentRow++;
			if (currentRow == rowMax)
			{
				currentRow = 0;
			}
		}
	}
	else
	{
		currentRow = startRow;
		currentColumn = startColumn;
		loopCount = 0;
	}
	
}
