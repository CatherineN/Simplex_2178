#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

	// Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader); 

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));
	
	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);  

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
	//A--B
	//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue,-fValue, fValue); //0
	vector3 point1( fValue,-fValue, fValue); //1
	vector3 point2( fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue,-fValue,-fValue); //4
	vector3 point5( fValue,-fValue,-fValue); //5
	vector3 point6( fValue, fValue,-fValue); //6
	vector3 point7(-fValue, fValue,-fValue); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

	//F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// center points of the base
	vector3 top = vector3(0.0f, a_fHeight / 2, 0.0f);
	vector3 baseCenter = vector3(0.0f, -(a_fHeight / 2), 0.0f);

	// store points
	std::vector <vector3> base;

	// calculate angles
	// total interior angles
	float totalInterior = (a_nSubdivisions - 2) * 180;
	// individual interior angles
	float interiorAngle = totalInterior / a_nSubdivisions;
	// center angle
	float centerAngle = 180 - interiorAngle;
	// change to radians
	float theta = glm::radians(centerAngle);

	float newAngle = 0;

	// remaining points
	for (int i = 0; i < a_nSubdivisions + 1; i++)
	{
		// create point
		vector3 newBottom(glm::cos(newAngle) * a_fRadius, -a_fHeight / 2, glm::sin(newAngle) * a_fRadius);
		// add to storage
		base.push_back(newBottom);
		// increment angle
		newAngle += theta;
	}

	// create tris for the base
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddTri(baseCenter, base[i], base[i + 1]);
	}

	// create sides
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddTri(base[i + 1], base[i], top);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// center points
	vector3 topCenter = vector3(0.0f, a_fHeight / 2, 0.0f);
	vector3 bottomCenter = vector3(0.0f, -(a_fHeight / 2), 0.0f);

	std::vector<vector3> top;
	std::vector<vector3> bottom;

	//calculate angles
	// total interiors
	float totalInterior = (a_nSubdivisions - 2) * 180;
	float interiorAngles = totalInterior / a_nSubdivisions;
	float centerAngle = 180 - interiorAngles;
	// change to radians
	float theta = glm::radians(centerAngle);

	float newAngle = 0;

	// points
	for (int i = 0; i < a_nSubdivisions + 1; i++)
	{
		vector3 newTop(glm::cos(newAngle) * a_fRadius, a_fHeight / 2, glm::sin(newAngle) * a_fRadius);
		vector3 newBottom(glm::cos(newAngle) * a_fRadius, -a_fHeight / 2, glm::sin(newAngle) * a_fRadius);

		top.push_back(newTop);
		bottom.push_back(newBottom);

		newAngle += theta;
	}

	// base
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddTri(topCenter, top[i + 1], top[i]);
		AddTri(bottomCenter, bottom[i], bottom[i + 1]);
	}

	// sides
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddQuad(bottom[i + 1], bottom[i], top[i + 1], top[i]);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// storage
	std::vector <vector3> outerTop;
	std::vector <vector3> outerBottom;
	std::vector <vector3> innerTop;
	std::vector <vector3> innerBottom;

	// calculate angles
	// total interior
	float totalInterior = (a_nSubdivisions - 2) * 180;
	// individual interior angles
	float interiorAngle = totalInterior / a_nSubdivisions;
	// center angle
	float centerAngle = 180 - interiorAngle;
	// change to radians
	float theta = glm::radians(centerAngle);

	float newAngle = 0;

	// remaining points
	for (int i = 0; i < a_nSubdivisions + 1; i++)
	{
		// create point
		vector3 newTopOuter(glm::cos(newAngle) * a_fOuterRadius, a_fHeight / 2, glm::sin(newAngle) * a_fOuterRadius);
		vector3 newBottomOuter(glm::cos(newAngle) * a_fOuterRadius, -a_fHeight / 2, glm::sin(newAngle) * a_fOuterRadius);

		vector3 newTopInner(glm::cos(newAngle) * a_fInnerRadius, a_fHeight / 2, glm::sin(newAngle) * a_fInnerRadius);
		vector3 newBottomInner(glm::cos(newAngle) * a_fInnerRadius, -a_fHeight / 2, glm::sin(newAngle) * a_fInnerRadius);

		// add to storage
		outerTop.push_back(newTopOuter);
		outerBottom.push_back(newBottomOuter);

		innerTop.push_back(newTopInner);
		innerBottom.push_back(newBottomInner);

		// increment angle
		newAngle += theta;
	}

	// create quads for base
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddQuad(outerBottom[i], outerBottom[i + 1], innerBottom[i], innerBottom[i + 1]);
		AddQuad(innerTop[i], innerTop[i + 1], outerTop[i], outerTop[i + 1]);
	}

	// outside of tube
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddQuad(outerBottom[i + 1], outerBottom[i], outerTop[i + 1], outerTop[i]);
	}

	// inside of tube
	for (int i = 0; i < a_nSubdivisions; i++)
	{
		AddQuad(innerTop[i + 1], innerTop[i], innerBottom[i + 1], innerBottom[i]);
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	// Subdivision Height
	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	// Subdivision Axis
	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// storage
	std::vector <vector3> outerTop;
	std::vector <vector3> outerBottom;
	std::vector <vector3> innerTop;
	std::vector <vector3> innerBottom;
	
	// calculate angles
	// total interior angles
	float totalInterior = (a_nSubdivisionsB - 2) * 180;
	// individual interior angles
	float intAngles = totalInterior / a_nSubdivisionsB;
	// center angle
	float centerAngle = 180 - intAngles;
	// radians
	float theta = glm::radians(centerAngle);

	float newAngle = 0;

	// remaining points
	for (int i = 0; i < a_nSubdivisionsA + 1; i++)
	{
		// create point
		vector3 newTopOuter(glm::cos(newAngle) * a_fOuterRadius, a_fOuterRadius / 2, glm::sin(newAngle) * a_fOuterRadius);
		vector3 newBottomOuter(glm::cos(newAngle) * a_fOuterRadius, -a_fOuterRadius / 2, glm::sin(newAngle) * a_fOuterRadius);

		vector3 newTopInner(glm::cos(newAngle) * a_fInnerRadius, a_fInnerRadius / 2, glm::sin(newAngle) * a_fInnerRadius);
		vector3 newBottomInner(glm::cos(newAngle) * a_fInnerRadius, -a_fInnerRadius / 2, glm::sin(newAngle) * a_fInnerRadius);

		// add to storage
		outerTop.push_back(newTopOuter);
		outerBottom.push_back(newBottomOuter);

		innerTop.push_back(newTopInner);
		innerBottom.push_back(newBottomInner);

		// increment angle
		newAngle += theta;
	}

	// middle ring
	for (int i = 0; i < a_nSubdivisionsA; i++)
	{
		AddQuad(outerBottom[i + 1], outerBottom[i], outerTop[i + 1], outerTop[i]);
		AddQuad(innerTop[i + 1], innerTop[i], innerBottom[i + 1], innerBottom[i]);
	}

	// top and bottoms
	for (int i = 0; i < a_nSubdivisionsA; i++)
	{
		AddQuad(outerBottom[i], outerBottom[i + 1], innerBottom[i], innerBottom[i + 1]);
		AddQuad(innerTop[i], innerTop[i + 1], outerTop[i], outerTop[i + 1]);
	}


	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// center points
	vector3 topCenter = vector3(0.0f, a_fRadius, 0.0f);
	vector3 bottomCenter = vector3(0.0f, -a_fRadius, 0.0f);

	int topSize = -1;
	int bottomSize = -1;

	// storage
	std::vector <std::vector <vector3>> top;
	std::vector <std::vector <vector3>> bottom;

	// calculate angles
	// total interior angles
	float totalInterior = ((a_nSubdivisions * 2) - 2) * 180;
	// individual interior angles
	float intAngles = totalInterior / (a_nSubdivisions * 2);
	// center angle
	float centerAngle = 180 - intAngles;
	// radians
	float theta = glm::radians(centerAngle);

	float newAngle = 0;
	float newAngle2 = 0;
	float newRadius = 0.99f;
	float radChange = 1.0f / ((a_nSubdivisions * 2));

	// center ring
	std::vector <vector3> centerRing;
	for (int i = 0; i <= (a_nSubdivisions * 2); i++)
	{
		vector3 newCenter(glm::cos(newAngle) * a_fRadius, 0.0f, glm::sin(newAngle) * a_fRadius);
		centerRing.push_back(newCenter);
		// increment angle
		newAngle += theta;
	}

	newAngle2 += (PI / 2) / (a_nSubdivisions * 2);

	// remaining points
	for (int i = 0; i < (a_nSubdivisions * 2); i++)
	{
		newAngle = 0;

		// store points
		std::vector <vector3> topPoints;
		std::vector <vector3> bottomPoints;

		for (int j = 0; j < (a_nSubdivisions * 2) + 1; j++)
		{
			float yTop = (1 / a_nSubdivisions) * (i + 1);
			float yBottom = (-i - 1) * (a_fRadius / a_nSubdivisions);

			// create point
			vector3 newTop((glm::cos(newAngle) * a_fRadius) * newRadius, (glm::sin(newAngle2) * a_fRadius), (glm::sin(newAngle) * a_fRadius) * newRadius);
			vector3 newBottom((glm::cos(newAngle) * a_fRadius) * newRadius, -(glm::sin(newAngle2) * a_fRadius), (glm::sin(newAngle) * a_fRadius) * newRadius);

			// add to storage vector
			topPoints.push_back(newTop);
			bottomPoints.push_back(newBottom);

			newAngle += theta;
		}
		// add to vector of rings
		top.push_back(topPoints);
		bottom.push_back(bottomPoints);

		// increase the cap
		topSize++;
		bottomSize++;

		newRadius -= radChange;

		// next ring is smaller
		newAngle2 += (PI / 2) / (a_nSubdivisions * 2);
	}

	// Add Faces
	// middle strip
	for (int i = 0; i < (a_nSubdivisions * 2); i++)
	{
		AddQuad(centerRing[i + 1], centerRing[i], top[0][i + 1], top[0][i]);
		AddQuad(bottom[0][i + 1], bottom[0][i], centerRing[i + 1], centerRing[i]);
	}

	// top and bottoms
	for (int i = 0; i < (a_nSubdivisions * 2); i++)
	{
		AddTri(topCenter, top[topSize][i + 1], top[topSize][i]);
		AddTri(bottomCenter, bottom[bottomSize][i], bottom[bottomSize][i + 1]);
	}

	if (a_nSubdivisions > 1)
	{
		for (int i = 0; i < (a_nSubdivisions * 2) - 1; i++)
		{
			for (int j = 0; j < (a_nSubdivisions * 2); j++)
			{
				AddQuad(bottom[i][j], bottom[i][j + 1], bottom[i + 1][j], bottom[i + 1][j + 1]);
				AddQuad(top[i][j + 1], top[i][j], top[i + 1][j + 1], top[i + 1][j]);
			}
		}
	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}