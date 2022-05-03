#include "tsr_platform.h"
#include "tsr_gui.h"
#include "tsr_types.h"
#include "tsr_dx11_data.h"
#include "tsr_profiling.h"
#include "tsr_macros.h"

enum class Primitive : ui32 {Triangle, Plane, Cube, Sphere, Icosphere, Pyramid, Cilinder, Cone, Torus, Capsule, Count};

/*
void TSR_DrawGUI(DX11Data& dxData, IMData* imData, FrameStats& fStats);
void InitializeCamera(DirectX::XMFLOAT3 position, DirectX::XMFLOAT3 target, DirectX::XMFLOAT3 up, float fov, float aspectRatio, CameraData* camData);
void InitializeCBuffer(CameraData& camData, DX11Data* dxData, ConstantBuffer* cbuffer);
void UpdateCBuffer(const CameraData& camData, float deltarot, float rotaxis[3], ConstantBuffer* cbuffer);
void TSR_Update(float dt);
void TSR_Draw(float rotVelocity, CameraData* camData, ConstantBuffer* cbuffer, IMData* imData, DX11Data& dxData, DX11VertexShaderData& vsData, DX11PixelShaderData& psData, BufferData& vb, BufferData& ib, RenderData* renderData);
*/