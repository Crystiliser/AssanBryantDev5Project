
cbuffer matriceData : register(b0)
{
	matrix view;
	matrix projection;
	matrix model;
	matrix pointLightPosition;
};

struct vertexShaderInput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : COLOR;
	float4 weights : WEIGHTS;
	float4 boneIndicies : BONES;
};

struct pixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : COLOR;
	float3 pointLight : PLIGHT;
	float3 wPos : WPOS;
};

pixelShaderInput main( vertexShaderInput input )
{
	pixelShaderInput output;
	float4 position = float4(input.pos.xyz, 1);
		
	position = mul(position, model);
	output.wPos = position;
	position = mul(position, view);
	position = mul(position, projection);

	output.pos = position;
	
	output.uv = input.uv;

	output.normal = input.normal;

	output.pointLight = float4(pointLightPosition._41, pointLightPosition._42, pointLightPosition._43, 1.0f);

	return output;
}