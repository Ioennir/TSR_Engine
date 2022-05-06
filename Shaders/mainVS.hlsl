cbuffer cb
{
	matrix mWorld;
	matrix mWVP; //transform
	matrix normalMatrix;
};

static const float3 LIGHT_DIR = normalize(float3(5.0f, 5.0f, -5.0f));

void main(
    float3 iPos : POSITION,
    float4 iCol : COLOR,
    float3 iNormal : NORMAL,
    out float4 oPos : SV_POSITION,
    out float4 oCol : COLOR
)
{
	oPos = mul(float4(iPos, 1.0f), mWVP);
	
	float3 normalWorldSpace = normalize(mul((float3x3)normalMatrix, iNormal));
    
	float LightIntensity = max(dot(normalWorldSpace, LIGHT_DIR), 0);
    
	oCol = LightIntensity * iCol;
}
