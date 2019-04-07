#include "MyOctant.h"
using namespace Simplex;

uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;
uint MyOctant::m_uIdealEntityCount = 5;

MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	//Intialize variables
	Init();
	
	//assume this is root
	m_pRoot = this;

	//clear out children, if any
	m_lChild.clear();

	//store attributes
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uMaxLevel = a_nMaxLevel;

	//assume first octant
	m_uOctantCount = 0;
	m_uID = 0;


	//just use one vector so we dont have to waste time combining (order doesn't matter)
	std::vector<vector3> ptExtents;

	//number of entities to encapsulate
	int entityCount = m_pEntityMngr->GetEntityCount();

	//for each entity, get the min and max and store in vector
	for (int i = 0; i < entityCount; i++)
	{
		MyEntity* tempEnt = m_pEntityMngr->GetEntity(i);
		ptExtents.push_back(tempEnt->GetRigidBody()->GetMinGlobal());
		ptExtents.push_back(tempEnt->GetRigidBody()->GetMaxGlobal());
	}

	//generate a rigidbody that encapsulates all of those points
	MyRigidBody* totalBody = new MyRigidBody(ptExtents);

	//the center of that rigidbody is the center of the octant
	m_v3Center = totalBody->GetCenterLocal();
	
	//get the halfwidths of the octant
	vector3 hWidths = totalBody->GetHalfWidth();
	
	//to store the largest half width
	float largest = 0;

	//find which halfwidth is largest, and store it
	for (int i = 0; i < 3; i++)
	{
		if (hWidths[i] > largest)
		{
			largest = hWidths[i];
		}
	}

	//this halfwidth made into fullwidth is our size
	m_fSize = largest * 2;

	//max and min of octant are largest halfwidth +/- center
	m_v3Max = vector3(m_v3Center.x + largest, m_v3Center.y + largest, m_v3Center.z + largest);
	m_v3Min = vector3(m_v3Center.x - largest, m_v3Center.y - largest, m_v3Center.z - largest);

	//increment how many octants there are
	m_uOctantCount++;

	//construct the octree
	ConstructTree(m_uMaxLevel);
}

MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	//init variables
	Init();

	//store arguments
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	//add one more octant
	m_uOctantCount++;

	//largest halfWidth
	float largest = m_fSize / 2.f;

	//same as in other constructor
	m_v3Max = vector3(m_v3Center.x + largest, m_v3Center.y + largest, m_v3Center.z + largest);
	m_v3Min = vector3(m_v3Center.x - largest, m_v3Center.y - largest, m_v3Center.z - largest);

}

MyOctant::MyOctant(MyOctant const & other)
{
	//get singletons
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}

	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;
	m_v3Center = other.m_v3Center;
	m_fSize = other.m_fSize;
	m_lChild = other.m_lChild;
	m_uChildren = other.m_uChildren;
	m_pParent = other.m_pParent;
	m_uID = other.m_uID;
	m_pRoot = other.m_pRoot;
	m_uLevel = other.m_uLevel;

}

MyOctant& MyOctant::operator=(MyOctant const & other)
{
	//same as the MyEntity copy assignment operator
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}

MyOctant::~MyOctant(void)
{
	Release();
}

void MyOctant::Swap(MyOctant & other)
{
	//Swaps all the memory of two octants
	for (int i = 0; i < 8; i++)
	{
		std::swap(m_pChild[i], other.m_pChild[i]);
	}
	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();
	std::swap(m_fSize, other.m_fSize);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_pParent, other.m_pParent);
	std::swap(m_lChild, other.m_lChild);
	std::swap(m_uChildren, other.m_uChildren);
	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);
}

//Accessors
float MyOctant::GetSize(void)
{
	return m_fSize;
}
vector3 MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}
vector3 MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}
vector3 MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}
MyOctant * MyOctant::GetChild(uint a_nChild)
{
	return a_nChild > 7 ? nullptr : m_pChild[a_nChild];
}
MyOctant * MyOctant::GetParent(void)
{
	return m_pParent;
}
uint MyOctant::GetOctantCount(void)
{
	return m_uOctantCount;
}

bool MyOctant::IsColliding(uint a_uRBIndex)
{
	//get our total entity count for octree
	int entityCount = m_pEntityMngr->GetEntityCount();

	//if the index given is greater than the entity count, something is wrong
	if (a_uRBIndex >= entityCount) return false;

	//get the corresponding entity and its rigid body
	MyEntity* entity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* entityRB = entity->GetRigidBody();

	//get the min/max for use in AABB
	vector3 min = entityRB->GetMinGlobal();
	vector3 max = entityRB->GetMaxGlobal();

	//AABB test
	if (m_v3Min.x > max.x) { return false; }
	if (m_v3Min.y > max.y) { return false; }
	if (m_v3Min.z > max.z) { return false; }

	if (m_v3Max.x < min.x) { return false; }
	if (m_v3Max.y < min.y) { return false; }
	if (m_v3Max.z < min.z) { return false; }

	//If none of the tests returned, must be overlapping
	return true;
}

void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	//if we've found the index we're search for
	if (m_uID == a_nIndex)
	{
		//draw a cube representing this octant
		DrawCube(a_v3Color);
		return;
	}
	//if we didn't find it yet, loop through our children and recurse until found
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_nIndex, a_v3Color);
	}
}

void MyOctant::Display(vector3 a_v3Color)
{
	//recursively call display on each of our children
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}

	//display this octant
	DrawCube(a_v3Color);
}

void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	//get total number of leaves
	int numOfLeaves = m_lChild.size();
	
	//loop through all the leaves
	for (int i = 0; i < numOfLeaves; i++)
	{
		//recursive call their display
		m_lChild[i]->DisplayLeafs(a_v3Color);
	}

	//draw this octant
	DrawCube(a_v3Color);
}

void MyOctant::ClearEntityList(void)
{
	//call for each child recursively
	for (int i = 0; i < m_uChildren; i++)
	{
		m_lChild[i]->ClearEntityList();
	}

	//clear this octant's entity list
	m_EntityList.clear();
}

void MyOctant::Subdivide(void)
{
	//if we're trying to go a level too deep
	if (m_uLevel >= m_uMaxLevel)
	{
		return;
	}
	
	//only subdivide leaves
	if (!IsLeaf())
	{
		return;
	}

	//this octant is no longer a leaf and will have 8 children
	m_uChildren = 8;

	//create a center point that we will move around to create the octant at
	vector3 tempCenter = m_v3Center;

	//the half width size of the new child octants
	float childHW = m_fSize / 4.f;

	//the offset that we'll use to move the center to the correct spots
	//also the size of the child used to create the new octant
	float childSize = m_fSize / 2.f;

	//define the center offsets for each octant
	float left = tempCenter.x - childHW;
	float right = tempCenter.x + childHW;
	float bottom = tempCenter.y - childHW;
	float top = tempCenter.y + childHW;
	float front = tempCenter.z - childHW;
	float back = tempCenter.z + childHW;

	//create vectors for the center of each octant
	vector3 positions[8];
	positions[0] = vector3(left, bottom, front);
	positions[1] = vector3(left, bottom, back);
	positions[2] = vector3(right, bottom, front);
	positions[3] = vector3(right, bottom, back);
	positions[4] = vector3(left, top, front);
	positions[5] = vector3(left, top, back);
	positions[6] = vector3(right, top, front);
	positions[7] = vector3(right, top, back);

	//Create the octants based on those centers
	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = new MyOctant(positions[i], childSize);
	}


	for (int i = 0; i < 8; i++)
	{
		//assign each child that this is their parent
		m_pChild[i]->m_pParent = this;

		//assign the root to each child
		m_pChild[i]->m_pRoot = m_pRoot;

		// each child is one level deeper than its parent
		m_pChild[i]->m_uLevel = m_uLevel + 1;

		//if any of the children still have too many entities
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
		{
			//subdivide further
			m_pChild[i]->Subdivide();
		}
	}
}


bool MyOctant::IsLeaf(void)
{
	return m_uChildren == 0;
}

bool MyOctant::ContainsMoreThan(uint a_nEntities)
{
	//get all entities in octree
	int entityCount = m_pEntityMngr->GetEntityCount();

	//to store number of entities in this octant
	int entitiesContained = 0;
	
	//check each entity for collision
	for (int i = 0; i < entityCount; i++)
	{
		//if entity is inside octant, add 1 to the count
		if (IsColliding(i)) { entitiesContained++; }

		//if there are more entities in the octant than given, return true
		if (entitiesContained > a_nEntities) { return true; }
	}

	//otherwise there are less than given number, return false
	return false;
}

void MyOctant::KillBranches(void)
{
	//loop through this octant children, if any
	for (int i = 0; i < m_uChildren; i++)
	{
		//recursively call to delete all children
		m_pChild[i]->KillBranches();

		//make sure pointer is valid, then delete and set to nullptr
		SafeDelete(m_pChild[i]);
	}

	//no children should exist after this
	m_uChildren = 0;
}

void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	//Only root constructs tree
	if (!IsRoot()) return;

	//store max level incase it changed
	m_uMaxLevel = a_nMaxLevel;

	//since this is root, only one octant currently
	m_uOctantCount = 1;

	//clear out any stragglers
	m_EntityList.clear();
	m_lChild.clear();

	//kill branches if any
	KillBranches();
	
	//If this octant has more objects than desired, subdivide
	if (ContainsMoreThan(m_uIdealEntityCount)) Subdivide();

	//Recursively find leaves, check for objects inside leaves, 
	//add the IDs of objects inside the leaf to list
	AssignIDtoEntity();

	//Recursively add all children to root's child list
	ConstructList();
}

void MyOctant::AssignIDtoEntity(void)
{
	//recursively run this function for all children octants
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->AssignIDtoEntity();
	}
	
	//if this is a leaf
	if (IsLeaf())
	{
		//get all our entities
		int entityCount = m_pEntityMngr->GetEntityCount();

		//check if any are inside our octant
		for (int i = 0; i < entityCount; i++)
		{
			//if this entity is inside the octant
			if (IsColliding(i))
			{
				//add it to the entity list
				m_EntityList.push_back(i);

				//let the entitymanager know a collision is happening
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}

void MyOctant::Release(void)
{
	//if this is the root
	if (m_uLevel == 0)
	{
		//kill all children
		KillBranches();
	}

	//clear out our lists
	m_EntityList.clear();
	m_lChild.clear();

	//reset sizes to 0 as we now contain nothing
	m_fSize = 0.f;
	m_uChildren = 0;
}

void MyOctant::Init(void)
{
	//initialize all the things
	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}
	m_pEntityMngr = MyEntityManager::GetInstance();
	m_pMeshMngr = MeshManager::GetInstance();
	m_fSize = 0.f;
	m_uChildren = 0;
	m_uLevel = 0;
	m_uID = m_uOctantCount;
	m_pRoot = nullptr;
	m_pParent = nullptr;
	m_v3Center = ZERO_V3;
	m_v3Min = ZERO_V3;
	m_v3Max = ZERO_V3;
}

void MyOctant::ConstructList(void)
{
	//recursively run this function for all children octants
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ConstructList();
	}

	//if this octant has entities in it
	if (m_EntityList.size() != 0)
	{
		//add them to the root's child list
		m_pRoot->m_lChild.push_back(this);
	}
}

void MyOctant::DrawCube(vector3 color)
{
	//draws a wireframe cube where the octant is w/ given color
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), color, RENDER_WIRE);
}

bool Simplex::MyOctant::IsRoot()
{
	return m_uLevel == 0;
}
