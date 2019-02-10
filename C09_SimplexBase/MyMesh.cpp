#include "MyMesh.h"


MyMesh::MyMesh()
{
	Init();
}

MyMesh::~MyMesh()
{

}

void MyMesh::Init()
{
	m_uShaderProgramID = 0; //Shader Program
	m_uVAO = 0; //Vertex Array Obj
	m_uVBO = 0; //Vertex buffer object
	m_uShaderProgramID = LoadShaders("Shaders//Basic.vs", "Shaders//Basic.fs");
}


void MyMesh::AddPosition(vector3 a_v3Pos)
{
	m_lVertexPos.push_back(a_v3Pos);
}

void MyMesh::AddColor(vector3 a_v3Col)
{
	m_lVertexCol.push_back(a_v3Col);
}

void MyMesh::CompleteMesh()
{
	for (uint i = 0; i < m_lVertexPos.size(); i++)
	{
		m_lVertexCol.push_back(C_PURPLE);
	}
	for (uint i = 0; i < m_lVertexPos.size(); i++)
	{
		m_lVertex.push_back(m_lVertexPos[i]);
		m_lVertex.push_back(m_lVertexCol[i]);
	}
}

void MyMesh::CompileOpenGL3X()
{
	CompleteMesh();

	glGenVertexArrays(1, &m_uVAO);
	glGenBuffers(1, &m_uVBO);

	glBindVertexArray(m_uVAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_uVBO);

	glBufferData(GL_ARRAY_BUFFER, m_lVertex.size() * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);

	int attributeCount = 2;

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, attributeCount * sizeof(vector3), (GLvoid*)0);

	glEnableVertexAttribArray(1);
	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, attributeCount * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	glBindVertexArray(0);
}

void MyMesh::Render(matrix4 a_m4Model, matrix4 a_m4View, matrix4 a_m4Projection)
{
	glBindVertexArray(m_uVAO);
	glUseProgram(m_uShaderProgramID);

	//read uniforms and send values
	GLuint MVP = glGetUniformLocation(m_uShaderProgramID, "MVP");//Model View Projection
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(a_m4Projection * a_m4View * a_m4Model));

	//draw content
	glDrawArrays(GL_TRIANGLES, 0, m_lVertex.size());
}