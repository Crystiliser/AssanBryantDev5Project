
cbuffer matriceData : register(b0)
{
	matrix view;
	matrix projection;
};

struct vertexShaderInput
{
	float4 pos : POSITION;
	float4 color : COLOR;
};

struct pixelShaderInput
{
	float4 color : COLOR;
	float4 pos : SV_POSITION;
};

pixelShaderInput main( vertexShaderInput input )
{
	pixelShaderInput output;
	float4 position = float4(input.pos.xyz, 1);
		
	position = mul(position, view);
	position = mul(position, projection);

	output.pos = position;
	
	output.color = input.color;

	return output;
}