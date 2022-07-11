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
	DirectX::XMFLOAT4 Direction;
	DirectX::XMFLOAT4 Color; //RGBA Where A is LightIntensity.
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