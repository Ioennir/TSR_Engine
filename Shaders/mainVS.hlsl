cbuffer cb {
    matrix wvp;
    matrix w;
    matrix v;
    matrix p;
};

void main(
    float3 iPos : POSITION,
    float4 iCol : COLOR,
    out float4 oPos : SV_POSITION,
    out float4 oCol : COLOR
)
{
    oPos = mul(float4(iPos, 1.0f), p * v * w);

    oCol = iCol;
}
