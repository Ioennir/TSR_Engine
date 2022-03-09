/*
	This .h file contains all specific data structures that will be transformed along the way in the Direct3d11 pipeline.
	Additionally, it will also contain data structures that will be in use to configure DX11 and/or interact with it.
*/

#pragma once
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <DirectXPackedVector.h>

struct DX11Info
{
	ID3D11Device* device;
	ID3D11DeviceContext* imDeviceContext;
	D3D_FEATURE_LEVEL* featureLevel;
	IDXGISwapChain* swapChain;
	ID3D11Texture2D* depthStencilBuffer;
	ID3D11RenderTargetView* renderTargetView;
	ID3D11DepthStencilView* depthStencilView;
	D3D11_VIEWPORT screenViewport;

} typedef DX11Info;

//Simple vertex implementation
struct Vertex 
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT4 Color;
} typedef Vertex;