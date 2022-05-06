#include <stdint.h>
#include <DirectXMath.h>

typedef int8_t		i8;
typedef int16_t		i16;
typedef int32_t		i32;
typedef int64_t		i64;

typedef uint8_t		ui8;
typedef uint16_t	ui16;
typedef uint32_t	ui32;
typedef uint64_t	ui64;

typedef float		r32;
typedef double		r64;

typedef intptr_t	iptr;
typedef uintptr_t	uiptr;


//These structs are as simple as possible for now to perform
// a basic 3D projection and get things going.
struct CameraData
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mView;
	DirectX::XMMATRIX mProj;
};

struct ConstantBuffer
{
	DirectX::XMMATRIX mWorld;
	DirectX::XMMATRIX mVWP;
	DirectX::XMMATRIX normalMatrix;
};

struct Mesh
{
	eastl::vector<DirectX::XMFLOAT3> vertices;
	eastl::vector<ui32> indices;
};

struct Vertex
{
	DirectX::XMFLOAT3 Position{ 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 Color{ 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 Normal{ 0.0f, 0.0f, 0.0f };
};

struct RenderData
{
	eastl::vector<Mesh> meshes;
	eastl::vector<Vertex> vertexData;
	eastl::vector<ui32> totalIndices;
};