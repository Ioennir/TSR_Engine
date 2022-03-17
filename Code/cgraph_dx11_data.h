/*
	This .h file contains all specific data structures that will be transformed along the way in the Direct3d11 pipeline.
	Additionally, it will also contain data structures that will be in use to configure DX11 and/or interact with it.
*/

#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#define NO_VSYNC 0
#define VSYNC 1

struct DX11Data
{
	ID3D11Device* device{};
	ID3D11DeviceContext* imDeviceContext{};
	D3D_FEATURE_LEVEL* featureLevel{};
	IDXGISwapChain* swapChain{};
	ID3D11Texture2D* depthStencilBuffer{};
	ID3D11RenderTargetView* renderTargetView{};
	ID3D11RenderTargetView* textureRTView{};
	ID3D11DepthStencilView* depthStencilView{};
	ID3D11ShaderResourceView* shaderResView{};
	ID3D11RasterizerState* currentRasterizerState{};
	ID3D11Texture2D* renderTexture{};
	D3D11_VIEWPORT screenViewport{};
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