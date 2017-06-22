
Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

struct pixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : COLOR;
};

float4 main(pixelShaderInput input ) : SV_TARGET
{
	float4 color = shaderTexture.Sample(sampleType, input.uv);

	return color;
}