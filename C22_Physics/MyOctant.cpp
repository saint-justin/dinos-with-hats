#include "MyOctant.h"


uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 4;
uint MyOctant::m_uIdealEntityCount = 5;

//All the different constructors
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount)
{
	Init();

	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;

	m_pRoot = this;
	m_lChild.clear();

	std::vector<vector3> minMaxV3; //Holds all min and max vectors of bounding object

	uint objectN = m_pEntityMngr->GetEntityCount();
	for (uint i = 0; i < objectN; i++)
	{
		MyEntity* entityP = m_pEntityMngr->GetEntity(i);
		MyRigidBody* rigidBodyP = entityP->GetRigidBody();
		minMaxV3.push_back(rigidBodyP->GetMinGlobal());
		minMaxV3.push_back(rigidBodyP->GetMaxGlobal());
	}

	MyRigidBody* rigidBodyP = new MyRigidBody(minMaxV3);

	vector3 halfWidthV3 = rigidBodyP->GetHalfWidth();
	float maxF = halfWidthV3.x;
	for (int i = 0; i < 3; i++)
	{
		if (maxF < halfWidthV3[i])
			maxF = halfWidthV3[i];
	}

	vector3 centerV3 = rigidBodyP->GetCenterLocal();
	minMaxV3.clear();
	SafeDelete(rigidBodyP);

	m_fSize = maxF * 2.0f;
	m_v3Center = centerV3;
	m_v3Min = m_v3Center - (vector3(maxF));
	m_v3Max = m_v3Center + (vector3(maxF));

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);

}

MyOctant::MyOctant(vector3 a_v3Center, float a_fSize)
{
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	m_uOctantCount++;
}

MyOctant::MyOctant(MyOctant const& other)
{
	m_uChildren = other.m_uChildren;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pParent = other.m_pParent;

	m_pRoot = other.m_pRoot;
	m_lChild = other.m_lChild;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}
}

//RO3
MyOctant& MyOctant::operator=(MyOctant const& other)
{
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

MyOctant* MyOctant::GetChild(uint a_nChild)
{
	if (a_nChild > 7)
		return nullptr;
	return m_pChild[a_nChild];
}

MyOctant* MyOctant::GetParent(void)
{
	return m_pParent;
}

uint MyOctant::GetOctantCount(void)
{
	return m_uOctantCount;
}


//Octant methods
void MyOctant::Swap(MyOctant& other)
{
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_pParent, other.m_pParent);
	for (uint i = 0; i < 8; i++)
	{
		std::swap(m_pChild[i], other.m_pChild[i]);
	}
}

bool MyOctant::IsColliding(uint a_uRBIndex)
{
	uint ObjCount = m_pEntityMngr->GetEntityCount();

	//If index is larger than the number of elements in the bounding obj, there is no collision
	if (a_uRBIndex >= ObjCount)
		return false;

	//Use AABB to check for collisions since the Octree doesn't rotate
	//All global space vectors
	MyEntity* entityPtr = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* rigidBodyPtr = entityPtr->GetRigidBody();
	vector3 v3Min = rigidBodyPtr->GetMinGlobal();
	vector3 v3Max = rigidBodyPtr->GetMaxGlobal();

	//Check for x
	if (m_v3Max.x < v3Min.x)
		return false;
	if (m_v3Min.x > v3Max.x)
		return false;

	//Check for y
	if (m_v3Max.y < v3Min.y)
		return false;
	if (m_v3Min.y > v3Max.y)
		return false;

	//Check for z
	if (m_v3Max.z < v3Min.z)
		return false;
	if (m_v3Min.z > v3Max.z)
		return false;

	return true;
}

//Displays the wireframe box of our octant
void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
			glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_nIndex);
	}
}

void MyOctant::Display(vector3 a_v3Color)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
		glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	uint nLeafs = m_lChild.size();
	for (uint child = 0; child < nLeafs; child++)
	{
		m_lChild[child]->DisplayLeafs(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) *
		glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctant::ClearEntityList(void)
{
	for (uint child = 0; child < m_uChildren; child++)
	{
		m_pChild[child]->ClearEntityList();
	}
	m_EntityList.clear();
}

void MyOctant::Subdivide(void)
{
	//If the node has reached the max depth, return with no changes
	if (m_uLevel >= m_uMaxLevel)
		return;

	//If the node has already been subdivided, return with no changes
	if (m_uChildren != 0)
		return;

	//Since this will have children, it is not a leaf
	m_uChildren = 8;

	float fSize = m_fSize / 4.0f;
	float fSizeD = fSize * 2.0f;
	vector3 centerV3;

	//Octant 0 bottom row, back left spot
	centerV3 = m_v3Center;
	centerV3.x -= fSize;
	centerV3.y -= fSize;
	centerV3.z -= fSize;
	m_pChild[0] = new MyOctant(centerV3, fSizeD);

	//Octant 1 bottom row, back right spot
	centerV3.x += fSizeD;
	m_pChild[1] = new MyOctant(centerV3, fSizeD);

	//Octant 2 bottom row, front right spot
	centerV3.z += fSizeD;
	m_pChild[2] = new MyOctant(centerV3, fSizeD);

	//Octant 3 bottom row, front left spot
	centerV3.x -= fSizeD;
	m_pChild[3] = new MyOctant(centerV3, fSizeD);

	//Octant 4 top row, front left spot
	centerV3.y += fSizeD;
	m_pChild[4] = new MyOctant(centerV3, fSizeD);

	//Octant 5 top row, back left spot
	centerV3.z -= fSizeD;
	m_pChild[5] = new MyOctant(centerV3, fSizeD);

	//Octant 6 top row, back right spot
	centerV3.x += fSizeD;
	m_pChild[6] = new MyOctant(centerV3, fSizeD);
	//Octant 7 top row, front right spot
	centerV3.z += fSizeD;
	m_pChild[7] = new MyOctant(centerV3, fSizeD);

	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
		{
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
	uint count = 0;
	uint objCount = m_pEntityMngr->GetEntityCount();
	for (uint i = 0; i < objCount; i++)
	{
		if (IsColliding(i))
			count++;
		if (count > a_nEntities)
			return true;
	}
	return false;
}

void MyOctant::KillBranches(void)
{
	/*Goes through the whole tree until it finds a node with no children and once it gets back to its parent, it will kill all of the children (great taken out of context) recursivley
	until returned back to the node it was called by*/
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}

void MyOctant::ConstructTree(uint a_nMaxLevel)
{
	//Only used on the root node
	if (m_uLevel != 0)
		return;

	m_uMaxLevel = a_nMaxLevel;

	m_uOctantCount = 1;

	m_EntityList.clear();

	//Clear the tree
	KillBranches();
	m_lChild.clear();

	//If at the base tree
	if (ContainsMoreThan(m_uIdealEntityCount))
	{
		Subdivide();
	}

	//Add octant id
	AssignIDtoEntity();

	//Construct list of objects
	ConstructList();
}

void MyOctant::AssignIDtoEntity(void)
{
	//Traverse until at a leaf
	for (uint child = 0; child < m_uChildren; child++)
	{
		m_pChild[child]->AssignIDtoEntity();
	}
	if (m_uChildren == 0) //If a leaf
	{
		uint entities = m_pEntityMngr->GetEntityCount();
		for (uint index = 0; index < entities; index++)
		{
			if (IsColliding(index))
			{
				m_EntityList.push_back(index);
				m_pEntityMngr->AddDimension(index, m_uID);
			}
		}
	}
}

//Creates a list of all the children a node has
void MyOctant::ConstructList(void)
{
	for (uint child = 0; child < m_uChildren; child++)
	{
		m_pChild[child]->ConstructList();
	}

	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}



void MyOctant::Release(void)
{
	//Only occurs if you are only releasing the root
	if (m_uLevel == 0)
	{
		KillBranches();
	}
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_EntityList.clear();
	m_lChild.clear();
}

void MyOctant::Init(void)
{
	m_uChildren = 0;

	m_fSize = 0.0f;

	m_uID = m_uOctantCount;
	m_uLevel = 0;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pRoot = nullptr;
	m_pParent = nullptr;

	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = nullptr;
	}
}