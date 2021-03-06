#include "MyRigidBody.h"
using namespace Simplex;
//Allocation
void MyRigidBody::Init(void)
{
	m_pMeshMngr = MeshManager::GetInstance();
	m_bVisibleBS = false;
	m_bVisibleOBB = true;
	m_bVisibleARBB = false;

	m_fRadius = 0.0f;

	m_v3ColorColliding = C_RED;
	m_v3ColorNotColliding = C_WHITE;

	m_v3Center = ZERO_V3;
	m_v3MinL = ZERO_V3;
	m_v3MaxL = ZERO_V3;

	m_v3MinG = ZERO_V3;
	m_v3MaxG = ZERO_V3;

	m_v3HalfWidth = ZERO_V3;
	m_v3ARBBSize = ZERO_V3;

	m_m4ToWorld = IDENTITY_M4;
}
void MyRigidBody::Swap(MyRigidBody& a_pOther)
{
	std::swap(m_pMeshMngr, a_pOther.m_pMeshMngr);
	std::swap(m_bVisibleBS, a_pOther.m_bVisibleBS);
	std::swap(m_bVisibleOBB, a_pOther.m_bVisibleOBB);
	std::swap(m_bVisibleARBB, a_pOther.m_bVisibleARBB);

	std::swap(m_fRadius, a_pOther.m_fRadius);

	std::swap(m_v3ColorColliding, a_pOther.m_v3ColorColliding);
	std::swap(m_v3ColorNotColliding, a_pOther.m_v3ColorNotColliding);

	std::swap(m_v3Center, a_pOther.m_v3Center);
	std::swap(m_v3MinL, a_pOther.m_v3MinL);
	std::swap(m_v3MaxL, a_pOther.m_v3MaxL);

	std::swap(m_v3MinG, a_pOther.m_v3MinG);
	std::swap(m_v3MaxG, a_pOther.m_v3MaxG);

	std::swap(m_v3HalfWidth, a_pOther.m_v3HalfWidth);
	std::swap(m_v3ARBBSize, a_pOther.m_v3ARBBSize);

	std::swap(m_m4ToWorld, a_pOther.m_m4ToWorld);

	std::swap(m_CollidingRBSet, a_pOther.m_CollidingRBSet);
}
void MyRigidBody::Release(void)
{
	m_pMeshMngr = nullptr;
	ClearCollidingList();
}
//Accessors
bool MyRigidBody::GetVisibleBS(void) { return m_bVisibleBS; }
void MyRigidBody::SetVisibleBS(bool a_bVisible) { m_bVisibleBS = a_bVisible; }
bool MyRigidBody::GetVisibleOBB(void) { return m_bVisibleOBB; }
void MyRigidBody::SetVisibleOBB(bool a_bVisible) { m_bVisibleOBB = a_bVisible; }
bool MyRigidBody::GetVisibleARBB(void) { return m_bVisibleARBB; }
void MyRigidBody::SetVisibleARBB(bool a_bVisible) { m_bVisibleARBB = a_bVisible; }
float MyRigidBody::GetRadius(void) { return m_fRadius; }
vector3 MyRigidBody::GetColorColliding(void) { return m_v3ColorColliding; }
vector3 MyRigidBody::GetColorNotColliding(void) { return m_v3ColorNotColliding; }
void MyRigidBody::SetColorColliding(vector3 a_v3Color) { m_v3ColorColliding = a_v3Color; }
void MyRigidBody::SetColorNotColliding(vector3 a_v3Color) { m_v3ColorNotColliding = a_v3Color; }
vector3 MyRigidBody::GetCenterLocal(void) { return m_v3Center; }
vector3 MyRigidBody::GetMinLocal(void) { return m_v3MinL; }
vector3 MyRigidBody::GetMaxLocal(void) { return m_v3MaxL; }
vector3 MyRigidBody::GetCenterGlobal(void){	return vector3(m_m4ToWorld * vector4(m_v3Center, 1.0f)); }
vector3 MyRigidBody::GetMinGlobal(void) { return m_v3MinG; }
vector3 MyRigidBody::GetMaxGlobal(void) { return m_v3MaxG; }
vector3 MyRigidBody::GetHalfWidth(void) { return m_v3HalfWidth; }
matrix4 MyRigidBody::GetModelMatrix(void) { return m_m4ToWorld; }
vector3 MyRigidBody::c() { return GetCenterGlobal(); }
std::vector<vector3> MyRigidBody::u()
{
	matrix4 toWorld = GetModelMatrix();
	
	vector3 scale = ZERO_V3;
	quaternion orientation = IDENTITY_QUAT;
	vector3 translation = ZERO_V3;
	vector3 skew = ZERO_V3;
	vector4 perspective = vector4(0);
	
	glm::decompose(toWorld, scale, orientation, translation, skew, perspective);

	std::vector<vector3> axes;
	
	axes.push_back(vector3(toWorld * vector4(AXIS_X, 1)) - translation);
	axes.push_back(vector3(toWorld * vector4(AXIS_Y, 1)) - translation);
	axes.push_back(vector3(toWorld * vector4(AXIS_Z, 1)) - translation);	

	//debug draw
	//m_pMeshMngr->AddLineToRenderList(IDENTITY_M4, vector3(0), axes[0], C_RED, C_RED);
	//m_pMeshMngr->AddLineToRenderList(IDENTITY_M4, vector3(0), axes[1], C_GREEN, C_GREEN);
	//m_pMeshMngr->AddLineToRenderList(IDENTITY_M4, vector3(0), axes[2], C_BLUE, C_BLUE);


	return axes;
}
vector3 MyRigidBody::e() { return GetHalfWidth(); }
void MyRigidBody::SetModelMatrix(matrix4 a_m4ModelMatrix)
{
	//to save some calculations if the model matrix is the same there is nothing to do here
	if (a_m4ModelMatrix == m_m4ToWorld)
		return;

	//Assign the model matrix
	m_m4ToWorld = a_m4ModelMatrix;

	//Calculate the 8 corners of the cube
	vector3 v3Corner[8];
	//Back square
	v3Corner[0] = m_v3MinL;
	v3Corner[1] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MinL.z);
	v3Corner[2] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MinL.z);
	v3Corner[3] = vector3(m_v3MaxL.x, m_v3MaxL.y, m_v3MinL.z);

	//Front square
	v3Corner[4] = vector3(m_v3MinL.x, m_v3MinL.y, m_v3MaxL.z);
	v3Corner[5] = vector3(m_v3MaxL.x, m_v3MinL.y, m_v3MaxL.z);
	v3Corner[6] = vector3(m_v3MinL.x, m_v3MaxL.y, m_v3MaxL.z);
	v3Corner[7] = m_v3MaxL;

	//Place them in world space
	for (uint uIndex = 0; uIndex < 8; ++uIndex)
	{
		v3Corner[uIndex] = vector3(m_m4ToWorld * vector4(v3Corner[uIndex], 1.0f));
	}

	//Identify the max and min as the first corner
	m_v3MaxG = m_v3MinG = v3Corner[0];

	//get the new max and min for the global box
	for (uint i = 1; i < 8; ++i)
	{
		if (m_v3MaxG.x < v3Corner[i].x) m_v3MaxG.x = v3Corner[i].x;
		else if (m_v3MinG.x > v3Corner[i].x) m_v3MinG.x = v3Corner[i].x;

		if (m_v3MaxG.y < v3Corner[i].y) m_v3MaxG.y = v3Corner[i].y;
		else if (m_v3MinG.y > v3Corner[i].y) m_v3MinG.y = v3Corner[i].y;

		if (m_v3MaxG.z < v3Corner[i].z) m_v3MaxG.z = v3Corner[i].z;
		else if (m_v3MinG.z > v3Corner[i].z) m_v3MinG.z = v3Corner[i].z;
	}

	//we calculate the distance between min and max vectors
	m_v3ARBBSize = m_v3MaxG - m_v3MinG;
}
//The big 3
MyRigidBody::MyRigidBody(std::vector<vector3> a_pointList)
{
	Init();
	//Count the points of the incoming list
	uint uVertexCount = a_pointList.size();

	//If there are none just return, we have no information to create the BS from
	if (uVertexCount == 0)
		return;

	//Max and min as the first vector of the list
	m_v3MaxL = m_v3MinL = a_pointList[0];

	//Get the max and min out of the list
	for (uint i = 1; i < uVertexCount; ++i)
	{
		if (m_v3MaxL.x < a_pointList[i].x) m_v3MaxL.x = a_pointList[i].x;
		else if (m_v3MinL.x > a_pointList[i].x) m_v3MinL.x = a_pointList[i].x;

		if (m_v3MaxL.y < a_pointList[i].y) m_v3MaxL.y = a_pointList[i].y;
		else if (m_v3MinL.y > a_pointList[i].y) m_v3MinL.y = a_pointList[i].y;

		if (m_v3MaxL.z < a_pointList[i].z) m_v3MaxL.z = a_pointList[i].z;
		else if (m_v3MinL.z > a_pointList[i].z) m_v3MinL.z = a_pointList[i].z;
	}

	//with model matrix being the identity, local and global are the same
	m_v3MinG = m_v3MinL;
	m_v3MaxG = m_v3MaxL;

	//with the max and the min we calculate the center
	m_v3Center = (m_v3MaxL + m_v3MinL) / 2.0f;

	//we calculate the distance between min and max vectors
	m_v3HalfWidth = (m_v3MaxL - m_v3MinL) / 2.0f;

	//Get the distance between the center and either the min or the max
	m_fRadius = glm::distance(m_v3Center, m_v3MinL);
}
MyRigidBody::MyRigidBody(MyRigidBody const& a_pOther)
{
	m_pMeshMngr = a_pOther.m_pMeshMngr;

	m_bVisibleBS = a_pOther.m_bVisibleBS;
	m_bVisibleOBB = a_pOther.m_bVisibleOBB;
	m_bVisibleARBB = a_pOther.m_bVisibleARBB;

	m_fRadius = a_pOther.m_fRadius;

	m_v3ColorColliding = a_pOther.m_v3ColorColliding;
	m_v3ColorNotColliding = a_pOther.m_v3ColorNotColliding;

	m_v3Center = a_pOther.m_v3Center;
	m_v3MinL = a_pOther.m_v3MinL;
	m_v3MaxL = a_pOther.m_v3MaxL;

	m_v3MinG = a_pOther.m_v3MinG;
	m_v3MaxG = a_pOther.m_v3MaxG;

	m_v3HalfWidth = a_pOther.m_v3HalfWidth;
	m_v3ARBBSize = a_pOther.m_v3ARBBSize;

	m_m4ToWorld = a_pOther.m_m4ToWorld;

	m_CollidingRBSet = a_pOther.m_CollidingRBSet;
}
MyRigidBody& MyRigidBody::operator=(MyRigidBody const& a_pOther)
{
	if (this != &a_pOther)
	{
		Release();
		Init();
		MyRigidBody temp(a_pOther);
		Swap(temp);
	}
	return *this;
}
MyRigidBody::~MyRigidBody() { Release(); };
//--- a_pOther Methods
void MyRigidBody::AddCollisionWith(MyRigidBody* a_pOther)
{
	/*
		check if the object is already in the colliding set, if
		the object is already there return with no changes
	*/
	auto element = m_CollidingRBSet.find(a_pOther);
	if (element != m_CollidingRBSet.end())
		return;
	// we couldn't find the object so add it
	m_CollidingRBSet.insert(a_pOther);
}
void MyRigidBody::RemoveCollisionWith(MyRigidBody* a_pOther)
{
	m_CollidingRBSet.erase(a_pOther);
}
void MyRigidBody::ClearCollidingList(void)
{
	m_CollidingRBSet.clear();
}
bool MyRigidBody::IsColliding(MyRigidBody* const a_pOther)
{
	//check if spheres are colliding as pre-test
	bool bColliding = (glm::distance(GetCenterGlobal(), a_pOther->GetCenterGlobal()) < m_fRadius + a_pOther->m_fRadius);

	//if they are colliding check the SAT
	int satNum = SAT(a_pOther);
	if (satNum != eSATResults::SAT_NONE)
	{
		bColliding = false;// reset to false

		//color the plane based on if it's x, y or z
		vector3 color;
		switch (satNum)
		{
		case eSATResults::SAT_AX:
		case eSATResults::SAT_BX:
		case eSATResults::SAT_AXxBX:
		case eSATResults::SAT_AXxBY:
		case eSATResults::SAT_AXxBZ:
			color = C_RED;
			break;
		case eSATResults::SAT_AY:
		case eSATResults::SAT_BY:
		case eSATResults::SAT_AYxBX:
		case eSATResults::SAT_AYxBY:
		case eSATResults::SAT_AYxBZ:
			color = C_GREEN;
			break;
		case eSATResults::SAT_AZ:
		case eSATResults::SAT_BZ:
		case eSATResults::SAT_AZxBX:
		case eSATResults::SAT_AZxBY:
		case eSATResults::SAT_AZxBZ:
			color = C_BLUE;
			break;
		}

		//get the midpoint between the two objects
		vector3 point = (GetCenterGlobal() + a_pOther->GetCenterGlobal()) / 2;

		//tried to rotate in the right way, couldn't figure it out
		//plane = glm::lookAt(point, a_pOther->GetCenterGlobal(), vector3(GetModelMatrix() * vector4(AXIS_Y, 1)));
		
		//move the plane to that midpoint
		matrix4 plane = glm::translate(IDENTITY_M4, point);

		//draw the plane between the two objects
		m_pMeshMngr->AddPlaneToRenderList(plane, color);
	}

	if (bColliding) //they are colliding
	{
		this->AddCollisionWith(a_pOther);
		a_pOther->AddCollisionWith(this);
	}
	else //they are not colliding
	{
		this->RemoveCollisionWith(a_pOther);
		a_pOther->RemoveCollisionWith(this);
	}

	return bColliding;
}
void MyRigidBody::AddToRenderList(void)
{
	if (m_bVisibleBS)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireSphereToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(vector3(m_fRadius)), C_BLUE_CORNFLOWER);
		else
			m_pMeshMngr->AddWireSphereToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(vector3(m_fRadius)), C_BLUE_CORNFLOWER);
	}
	if (m_bVisibleOBB)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(m_v3HalfWidth * 2.0f), m_v3ColorColliding);
		else
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(m_m4ToWorld, m_v3Center) * glm::scale(m_v3HalfWidth * 2.0f), m_v3ColorNotColliding);
	}
	if (m_bVisibleARBB)
	{
		if (m_CollidingRBSet.size() > 0)
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(GetCenterGlobal()) * glm::scale(m_v3ARBBSize), C_YELLOW);
		else
			m_pMeshMngr->AddWireCubeToRenderList(glm::translate(GetCenterGlobal()) * glm::scale(m_v3ARBBSize), C_YELLOW);
	}
}

uint MyRigidBody::SAT(MyRigidBody* const a_pOther)
{
	MyRigidBody* thisOBB = this;
	MyRigidBody* otherOBB = a_pOther;


	float rThis, rOther;
	matrix3 rotation, absRotation;

	//put otherOBB into the same space as thisOBB
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			rotation[i][j] = glm::dot(thisOBB->u()[i], otherOBB->u()[j]);

	//get the vector that moves between the two objects
	vector3 translation = otherOBB->c() - thisOBB->c();

	//move that into the same coordinate space
	translation = vector3(glm::dot(translation, thisOBB->u()[0]), 
						  glm::dot(translation, thisOBB->u()[1]), 
						  glm::dot(translation, thisOBB->u()[2]));
	//add the smallest possible value for floats to the rotation matrix so that when
	//two lines are near to parallel, the cross product doesn't become null
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			absRotation[i][j] = abs(rotation[i][j]) + FLT_EPSILON;

	// X, Y, Z Axes for this OBB
	for (int i = 0; i < 3; i++)
	{
		rThis = thisOBB->e()[i];
		rOther = otherOBB->e()[0] * absRotation[i][0] + otherOBB->e()[1] * absRotation[i][1] + otherOBB->e()[2] * absRotation[i][2];
		if (abs(translation[i]) > rThis + rOther) 
			return i+1;
	}

	// X, Y, Z Axes for other OBB
	for (int i = 0; i < 3; i++)
	{
		rThis = thisOBB->e()[0] * absRotation[0][i] + thisOBB->e()[1] * absRotation[1][i] + thisOBB->e()[2] * absRotation[2][i];
		rOther = otherOBB->e()[i];
		if (abs(translation[0] * rotation[0][i] + translation[1] * rotation[1][i] + translation[2] * rotation[2][i]) > rThis + rOther)
			return i+4;
	}

	// Test the axis of ThisOBB's X cross OtherOBB's X
	rThis = thisOBB->e()[1] * absRotation[2][0] + thisOBB->e()[2] * absRotation[1][0];
	rOther = otherOBB->e()[1] * absRotation[0][2] + otherOBB->e()[2] * absRotation[0][1];
	if (abs(translation[2] * rotation[1][0] - translation[1] * rotation[2][0]) > rThis + rOther) 
		return eSATResults::SAT_AXxBX;

	// Test the axis of ThisOBB's X with OtherOBB's Y
	rThis = thisOBB->e()[1] * absRotation[2][1] + thisOBB->e()[2] * absRotation[1][1];
	rOther = otherOBB->e()[0] * absRotation[0][2] + otherOBB->e()[2] * absRotation[0][0];
	if (abs(translation[2] * rotation[1][1] - translation[1] * rotation[2][1]) > rThis + rOther) 
		return eSATResults::SAT_AXxBY;

	// Test the axis of ThisOBB's X with OtherOBB's Z
	rThis = thisOBB->e()[1] * absRotation[2][2] + thisOBB->e()[2] * absRotation[1][2];
	rOther = otherOBB->e()[0] * absRotation[0][1] + otherOBB->e()[1] * absRotation[0][0];
	if (abs(translation[2] * rotation[1][2] - translation[1] * rotation[2][2]) > rThis + rOther) 
		return eSATResults::SAT_AXxBZ;

	// Test the axis of ThisOBB's Y with OtherOBB's X
	rThis = thisOBB->e()[0] * absRotation[2][0] + thisOBB->e()[2] * absRotation[0][0];
	rOther = otherOBB->e()[1] * absRotation[1][2] + otherOBB->e()[2] * absRotation[1][1];

	if (abs(translation[0] * rotation[2][0] - translation[2] * rotation[0][0]) > rThis + rOther) 
		return eSATResults::SAT_AYxBX;

	// Test the axis of ThisOBB's Y with OtherOBB's Y
	rThis = thisOBB->e()[0] * absRotation[2][1] + thisOBB->e()[2] * absRotation[0][1];
	rOther = otherOBB->e()[0] * absRotation[1][2] + otherOBB->e()[2] * absRotation[1][0];
	if (abs(translation[0] * rotation[2][1] - translation[2] * rotation[0][1]) > rThis + rOther) 
		return eSATResults::SAT_AYxBY;

	// Test the axis of ThisOBB's Y with OtherOBB's Z
	rThis = thisOBB->e()[0] * absRotation[2][2] + thisOBB->e()[2] * absRotation[0][2];
	rOther = otherOBB->e()[0] * absRotation[1][1] + otherOBB->e()[1] * absRotation[1][0];
	if (abs(translation[0] * rotation[2][2] - translation[2] * rotation[0][2]) > rThis + rOther) 
		return eSATResults::SAT_AYxBZ;

	// Test the axis of ThisOBB's Z with OtherOBB's X
	rThis = thisOBB->e()[0] * absRotation[1][0] + thisOBB->e()[1] * absRotation[0][0];
	rOther = otherOBB->e()[1] * absRotation[2][2] + otherOBB->e()[2] * absRotation[2][1];
	if (abs(translation[1] * rotation[0][0] - translation[0] * rotation[1][0]) > rThis + rOther) 
		return eSATResults::SAT_AZxBX;

	// Test the axis of ThisOBB's Z with OtherOBB's Y
	rThis = thisOBB->e()[0] * absRotation[1][1] + thisOBB->e()[1] * absRotation[0][1];
	rOther = otherOBB->e()[0] * absRotation[2][2] + otherOBB->e()[2] * absRotation[2][0];
	if (abs(translation[1] * rotation[0][1] - translation[0] * rotation[1][1]) > rThis + rOther) 
		return eSATResults::SAT_AZxBY;

	// Test the axis of ThisOBB's Z with OtherOBB's Z
	rThis = thisOBB->e()[0] * absRotation[1][2] + thisOBB->e()[1] * absRotation[0][2];
	rOther = otherOBB->e()[0] * absRotation[2][1] + otherOBB->e()[1] * absRotation[2][0];
	if (abs(translation[1] * rotation[0][2] - translation[0] * rotation[1][2]) > rThis + rOther) 
		return eSATResults::SAT_AZxBZ;


	//No axis could be found that seperates the two objects
	//must be intersecting
	return eSATResults::SAT_NONE;

}