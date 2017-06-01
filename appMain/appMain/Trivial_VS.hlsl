
cbuffer vertex : register(b0)
{
	float4 position;
	float4 color;
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

	output.pos.w = 1;
	
	output.pos.xyz = input.pos.xyz;
		
	output.pos += position;
	
	output.color = input.color;

	return output;
}