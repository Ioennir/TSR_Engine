//TODO(Fran): directional, point, spot, rect...

struct LightAmbient
{
	DirectX::XMFLOAT4 Color; //RGBA Where A is LightIntensity.
};

struct LightDirectional
{
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Direction;
	DirectX::XMFLOAT4 Color; //RGBA Where A is LightIntensity.
};

struct LightPoint //Spherical
{
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Color; //RGBA Where A is LightIntensity.
	DirectX::XMFLOAT3 Fallof;
	r32 Range;
};

struct LightSpot //ConeLight
{
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Direction;
	DirectX::XMFLOAT4 Color; //RGBA Where A is LightIntensity.
};

struct LightRect //Squared
{
	DirectX::XMFLOAT4 Position;
	DirectX::XMFLOAT4 Direction;
	DirectX::XMFLOAT4 Color; //RGBA Where A is LightIntensity.
};

namespace Lighting
{
	BufferData LightBuffer{};
	ID3D11ShaderResourceView* LightBufferView;
	BufferData PointLightBuffer{};
	ID3D11ShaderResourceView* PointLightBufferView;
}

void TSR_InitDirectionalLightBuffer(BufferData * LightBuffer)
{

	eastl::vector<LightDirectional> LightsDir;
	LightsDir.insert(
		LightsDir.end(),
		{
			{5.0f, 7.0f, -5.0f, 1.0f},
			TSR_DX_NormalizeFLOAT4({5.0f, 7.0f, -5.0f, 0.0f}),
			{1.0f, 1.0f, 1.0f, 1.0f}
		}
	);

	TSR_DX11_BuildStructuredBuffer(
		DX11::dxData.device,
		sizeof(LightDirectional),
		0,
		LightsDir.size(),
		0,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE,
		LightsDir.data(),
		LightBuffer,
		&Lighting::LightBufferView
	);

	eastl::vector<LightPoint> LightsPoint;
	LightsPoint.insert(
		LightsPoint.end(),
		{
			{0.0f, 4.0f, 0.0f, 1.0f},
			{1.0f, 1.0f, 1.0f, 1.0f},
			{0.0f, 0.2f, 0.0f},
			100.0f
		}
	);

	TSR_DX11_BuildStructuredBuffer(
		DX11::dxData.device,
		sizeof(LightPoint),
		0,
		LightsPoint.size(),
		0,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE,
		LightsPoint.data(),
		&Lighting::PointLightBuffer,
		&Lighting::PointLightBufferView
	);
}