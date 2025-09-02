#pragma once

#include "MeshVbo.h"

class SpriteMeshVbo : public MeshVbo {

protected:
	bool isReset = false;
public:
	static string const MESH_NAME;
	SpriteMeshVbo();
	virtual string GetMeshName();
	virtual void Render();
	virtual void LoadData();

	void AdjustTexcoord(float * uv);
	void ResetTexcoord();
};