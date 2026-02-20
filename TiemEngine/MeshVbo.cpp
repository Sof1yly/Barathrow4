#include "MeshVbo.h"


MeshVbo::MeshVbo()
{
}

MeshVbo::~MeshVbo()
{
	if (posVboId != (GLuint)-1) {
		glDeleteBuffers(1, &posVboId);
		posVboId = (GLuint)-1;
	}
	if (texVboId != (GLuint)-1) {
		glDeleteBuffers(1, &texVboId);
		texVboId = (GLuint)-1;
	}
}

GLuint MeshVbo::GetPosId()
{
	return this->posVboId;
}

GLuint MeshVbo::GetTexId()
{
	return this->texVboId;
}

void MeshVbo::SetAttribId(GLuint posId, GLuint texId)
{
	this->posAttribId = posId;
	this->texAttribId = texId;
}
