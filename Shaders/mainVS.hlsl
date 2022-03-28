cbuffer cb {
    matrix mWorld;
    matrix mWVP;
};

void main(
    float3 iPos : POSITION,
    float4 iCol : COLOR,
    out float4 oPos : SV_POSITION,
    out float4 oCol : COLOR
)
{
    oPos = mul(float4(iPos, 1.0f), mWVP);

    oCol = iCol;
}
