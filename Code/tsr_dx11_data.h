/*
	This .h file contains all specific data structures that will be transformed along the way in the Direct3d11 pipeline.
	Additionally, it will also contain data structures that will be in use to configure DX11 and/or interact with it.
*/

#ifndef TSR_DX11_DATA
#define TSR_DX11_DATA

#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#define NO_VSYNC 0
#define VSYNC 1

struct DX11ScnData
{
	ID3D11Texture2D* depthStencilBuffer{};
	ID3D11Texture2D* renderTexture{};
	ID3D11RenderTargetView* renderTargetView{};
	ID3D11DepthStencilView* depthStencilView{};
	ID3D11ShaderResourceView* shaderResourceView{};
	D3D11_VIEWPORT viewport{};
};

struct DX11Data
{
	ID3D11Device* device{};
	ID3D11DeviceContext* imDeviceContext{};
	D3D_FEATURE_LEVEL* featureLevel{};
	IDXGISwapChain* swapChain{};
	ID3D11Texture2D* depthStencilBuffer{};
	ID3D11RenderTargetView* renderTargetView{};
	ID3D11DepthStencilView* depthStencilView{};
	ID3D11RasterizerState* currentRasterizerState{};
	D3D11_VIEWPORT windowViewport{};
	DX11ScnData scnData{};
	ID3D11Buffer* dx11_cbuffer{};
};

struct DX11VertexShaderData
{
	ID3D10Blob* shaderBuffer{};
	ID3D11VertexShader* shader{};
	ID3D11InputLayout* inputLayout{};
};

struct DX11PixelShaderData
{
	ID3D10Blob* shaderBuffer{};
	ID3D11PixelShader* shader{};

};

//Simple vertex implementation
struct Vertex 
{
	DirectX::XMFLOAT3 Position{0.0f, 0.0f, 0.0f};
	DirectX::XMFLOAT4 Color{0.0f, 0.0f, 0.0f, 1.0f};
};

struct BufferData
{
	ID3D11Buffer* buffer{};
	UINT stride{0};
	UINT offset{0};
};

struct IMData
{
	r32 rot[3]{ 0.0f, 1.0f, 0.0f };
	r32 rotSpeed{ 60.0f };
};


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
};

#endif
