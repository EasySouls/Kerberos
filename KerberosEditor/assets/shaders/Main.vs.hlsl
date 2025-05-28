struct VSInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD;
};

struct VSOutput
{
	float4 position : SV_Position;
	float3 color : COLOR0;
	float3 normal : NORMAL0;
	float2 texcoord : TEXCOORD0;
};

VSOutput Main(VSInput input)
{
	VSOutput output = (VSOutput)0;
	output.position = float4(input.position, 1.0f);
	output.color = float4(0.5, 0.5, 0.5, 1.0);
	output.normal = input.normal;
	output.texcoord = input.texcoord;
	return output;
}