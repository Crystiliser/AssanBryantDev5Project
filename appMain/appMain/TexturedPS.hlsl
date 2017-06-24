
Texture2D shaderTexture : register(t0);
SamplerState sampleType : register(s0);

struct pixelShaderInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float2 uv : COLOR;
	float3 pointLight : PLIGHT;
	float3 wPos : WPOS;
};

struct Light
{
	float3 position;
	float4 color;
	float ambientIntensity;
	float diffuseIntensity;
};
float4 main(pixelShaderInput input ) : SV_TARGET
{
	float4 color = shaderTexture.Sample(sampleType, input.uv);
	Light pointLight;

	pointLight.position = input.pointLight;
	pointLight.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
	pointLight.ambientIntensity = 0.3f;
	pointLight.diffuseIntensity = 1.0f;

	float4 result1;
	float3 lightDir = normalize(pointLight.position - input.wPos);
	float lightRatio = saturate(dot(lightDir, input.normal));
	float attenuation = 1.0f - (saturate(length(pointLight.position - input.wPos)) / 5.0f);
	lightRatio *= (attenuation * attenuation);
	result1 = lightRatio * pointLight.color;

	float4 result2 = (pointLight.color * pointLight.ambientIntensity);

	color = saturate((color * result1) + (color * result2) );

	return color;
}