#include "MyOctantExample.h"
using namespace Simplex;

//set static vars
uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;
uint MyOctant::m_uIdealEntityCount = 5;

#pragma region RuleOf3
//constructor -- root
Simplex::MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	//constructor
	Init();

	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;

	m_uOctantCount = 0;
	m_uID = m_uOctantCount;
	m_pRoot = this;
	m_lChild.clear();

	//MinMax vectors of bounding box
	std::vector<vector3> MinMax;

	//number of entities
	int enNum = m_pEntityMngr->GetEntityCount();
	for (int i = 0; i < enNum; i++)
	{
		//temp entity & its rigidbody
		MyEntity* ent = m_pEntityMngr->GetEntity(i);
		MyRigidBody* rb = ent->GetRigidBody();

		//add the global min & max to list
		MinMax.push_back(rb->GetMinGlobal());
		MinMax.push_back(rb->GetMaxGlobal());
	}

	//rigidbody for bounding wrt entities
	MyRigidBody* rb = new MyRigidBody(MinMax);

	//find center, min + max w/ halfwidths
	m_v3Center = rb->GetCenterLocal();

	//find largest dimension
	vector3 halfWidth = rb->GetHalfWidth();
	float maxDistance = halfWidth.x;

	//delete
	MinMax.clear();
	SafeDelete(rb);

	for (int i = 1; i < 3; i++)
	{
		//if y/z is larger, it's the max
		if (maxDistance < halfWidth[i])
		{
			maxDistance = halfWidth[i];
		}
	}
	m_v3Max = m_v3Center + vector3(maxDistance, maxDistance, maxDistance);
	m_v3Min = m_v3Center - vector3(maxDistance, maxDistance, maxDistance);

	//set size
	m_fSize = maxDistance * 2.0f;

	//increment octant count
	m_uOctantCount++;

	//make tree
	ConstructTree(m_uMaxLevel);
}

//constructor -- branch
Simplex::MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();

	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	//find max & min with the halfwidths
	float half = m_fSize / 2;
	m_v3Max = m_v3Center + vector3(half, half, half);
	m_v3Min = m_v3Center - vector3(half, half, half);

	m_uOctantCount++;
}

//copy constructor
Simplex::MyOctant::MyOctant(MyOctant const & other)
{
	m_uID = other.m_uID;
	m_uChildren = other.m_uChildren;
	m_uLevel = other.m_uLevel;

	m_fSize = other.m_fSize;

	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pParent = other.m_pParent;
	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}
	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;
}

//copy assignment operator
MyOctant & Simplex::MyOctant::operator=(MyOctant const & other)
{
	//if not same octant
	if (this != &other)
	{
		//clear current octant
		Release();
		Init();
		//temp octant
		MyOctant oct(other);
		//use Swap
		Swap(oct);
	}
	return *this;
}

//destructor
Simplex::MyOctant::~MyOctant(void)
{
	Release();
}

#pragma endregion

void Simplex::MyOctant::Swap(MyOctant & other)
{
	std::swap(m_uID, other.m_uID);
	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Max, other.m_v3Max);
	std::swap(m_v3Min, other.m_v3Min);

	std::swap(m_pParent, other.m_pParent);
	for (int i = 0; i < 8; i++)
	{
		std::swap(m_pChild[i], other.m_pChild[i]);
	}

	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);

}

#pragma region ACCESSORS
float Simplex::MyOctant::GetSize(void)
{
	return m_fSize;
}

vector3 Simplex::MyOctant::GetCenterGlobal(void)
{
	return m_v3Center;
}

vector3 Simplex::MyOctant::GetMinGlobal(void)
{
	return m_v3Min;
}

vector3 Simplex::MyOctant::GetMaxGlobal(void)
{
	return m_v3Max;
}

MyOctant * Simplex::MyOctant::GetChild(uint a_nChild)
{
	//if the ID is out of bounds
	if (a_nChild > 7)
	{
		return nullptr;
	}
	//return that index
	return m_pChild[a_nChild];
}

MyOctant * Simplex::MyOctant::GetParent(void)
{
	return m_pParent;
}

bool Simplex::MyOctant::IsColliding(uint a_uRBIndex)
{
	//number of cubes
	int objectNum = m_pEntityMngr->GetEntityCount();

	if (a_uRBIndex >= objectNum)
	{
		return false;
	}
	//get that entity + its rigidbody
	MyEntity* entity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* rb = entity->GetRigidBody();
	//get min + max of entity
	vector3 min = rb->GetMinGlobal();
	vector3 max = rb->GetMaxGlobal();

	//check if outside x bounds
	if (min.x > m_v3Max.x)
	{
		//no collision
		return false;
	}
	if (max.x < m_v3Min.x)
	{
		return false;
	}

	//check if outside y bounds
	if (min.y > m_v3Max.y)
	{
		//no collision
		return false;
	}
	if (max.y < m_v3Min.y)
	{
		return false;
	}

	//check if outside z bounds
	if (min.z > m_v3Max.z)
	{
		//no collision
		return false;
	}
	if (max.z < m_v3Min.z)
	{
		return false;
	}


	//object not out of bounds
	return true;

}
#pragma endregion

#pragma region DISPLAY
void Simplex::MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
			glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);

		return;
	}
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_nIndex);
	}
}

void Simplex::MyOctant::Display(vector3 a_v3Color)
{
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}

	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
		glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void Simplex::MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	int nLeafs = m_lChild.size();
	for (int nChild = 0; nChild < nLeafs; nChild++)
	{
		m_lChild[nChild]->DisplayLeafs(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
		glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);

}
#pragma endregion

void Simplex::MyOctant::ClearEntityList(void)
{
	//recursion
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ClearEntityList();
	}
	//clear
	m_EntityList.clear();
}

void Simplex::MyOctant::Subdivide(void)
{
	//don't divide past max
	if (m_uLevel >= m_uMaxLevel)
	{
		return;
	}

	//don't divide something already divided
	if (m_uChildren != 0)
	{
		return;
	}

	m_uChildren = 8;

	//divide size of box into 8
	float size = m_fSize / 4.0f;

	//new center
	vector3 center = m_v3Center;

	//move center to all 8 positions
	//bottom left back
	center.x -= size;
	center.y -= size;
	center.z -= size;
	m_pChild[0] = new MyOctant(center, size * 2);

	//bottom right back
	center.x += size * 2;
	m_pChild[1] = new MyOctant(center, size * 2);

	//top right back
	center.y += size * 2;
	m_pChild[2] = new MyOctant(center, size * 2);

	//top left back
	center.x -= size * 2;
	m_pChild[3] = new MyOctant(center, size * 2);

	//top left front
	center.z += size * 2;
	m_pChild[4] = new MyOctant(center, size * 2);

	//top right front
	center.x += size * 2;
	m_pChild[5] = new MyOctant(center, size * 2);

	//bottom right front
	center.y -= size * 2;
	m_pChild[6] = new MyOctant(center, size * 2);

	//bottom left front
	center.x -= size * 2;
	m_pChild[7] = new MyOctant(center, size * 2);

	//set the root, parent, and level
	for (int i = 0; i < 8; i++)
	{
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		//divide again if neccessary
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
		{
			m_pChild[i]->Subdivide();
		}
	}
}

bool Simplex::MyOctant::IsLeaf(void)
{
	return m_uChildren == 0;

}

bool Simplex::MyOctant::ContainsMoreThan(uint a_nEntities)
{
	//contained
	int count = 0;
	//total
	int objNum = m_pEntityMngr->GetEntityCount();

	//check which entities are contained
	for (int i = 0; i < objNum; i++)
	{
		if (IsColliding(i))
		{
			//this entity is contained
			count++;
		}
		//if count has grown larger than the number passed in return true
		if (count > a_nEntities)
		{
			return true;
		}
	}

}

void Simplex::MyOctant::KillBranches(void)
{
	//delete all pointers
	for (int i = 0; i++; i < m_uChildren)
	{
		//recursion
		m_pChild[i]->KillBranches();
		//delete
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}

	m_uChildren = 0;
}

void Simplex::MyOctant::ConstructTree(uint a_nMaxLevel)
{
	//only let root do this
	if (m_uLevel != 0)
	{
		return;
	}

	//set vars
	m_uMaxLevel = a_nMaxLevel;
	m_uOctantCount = 1;

	//cleanup
	m_EntityList.clear();
	KillBranches();
	m_lChild.clear();

	//if more than ideal number of entities, divide
	if (ContainsMoreThan(m_uIdealEntityCount))
	{
		Subdivide();
	}

	//assign the IDs
	AssignIDtoEntity();
	//make the list
	ConstructList();
}

void Simplex::MyOctant::AssignIDtoEntity(void)
{

	//traverse through the tree
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->AssignIDtoEntity();
	}

	if (IsLeaf())
	{
		//check for entities within bounds
		int numEnt = m_pEntityMngr->GetEntityCount();
		for (uint i = 0; i < numEnt; i++)
		{
			if (IsColliding(i))
			{
				//add to the list and assign ID
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}

uint Simplex::MyOctant::GetOctantCount(void)
{
	return m_uOctantCount;
}

void Simplex::MyOctant::Release(void)
{
	//clear everything from root
	if (m_uLevel == 0)
	{
		KillBranches();
	}

	//reset
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_EntityList.clear();
	m_lChild.clear();
}

void Simplex::MyOctant::Init(void)
{
	//set all the variables to 0
	m_uID = m_uOctantCount;
	m_uLevel = 0;
	m_uChildren = 0;

	m_fSize = 0.0f;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_v3Center = vector3(0.0f, 0.0f, 0.0f);
	m_v3Max = vector3(0.0f, 0.0f, 0.0f);
	m_v3Min = vector3(0.0f, 0.0f, 0.0f);

	m_pParent = nullptr;
	for (int i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}

	m_pRoot = nullptr;

}

void Simplex::MyOctant::ConstructList(void)
{
	//recursion
	for (int i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ConstructList();
	}

	//is a node that containt objects
	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}
