
cbuffer matriceData : register(b0)
{
	matrix view;
	matrix projection;
	matrix model;
};

struct vertexShaderInput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : COLOR;
};

struct pixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : COLOR;
};

pixelShaderInput main( vertexShaderInput input )
{
	pixelShaderInput output;
	float4 position = float4(input.pos.xyz, 1);
		
	position = mul(position, model);
	position = mul(position, view);
	position = mul(position, projection);

	output.pos = position;
	
	output.uv = input.uv;

	return output;
}