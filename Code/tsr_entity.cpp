
struct EPositions
{
	eastl::vector<r32> X;
	eastl::vector<r32> Y;
	eastl::vector<r32> Z;
};

// expressed in quaternion?
struct ERotations
{
	eastl::vector<r32> X;
	eastl::vector<r32> Y;
	eastl::vector<r32> Z;
	eastl::vector<r32> W;
};

struct EScales
{
	eastl::vector<r32> X;
	eastl::vector<r32> Y;
	eastl::vector<r32> Z;
};

struct ETransforms 
{
	EPositions	positions;
	ERotations	rotations;
	EScales		scales;
};

struct Entities
{
	ETransforms transforms;
};


void TSR_InitializeEntities(Entities * entities) 
{

}