#version 410 core

in block
{
    vec2 Texcoord;
} In;

uniform sampler2D ColorBuffer;
uniform sampler2D NormalBuffer;
uniform sampler2D DepthBuffer;

uniform mat4 InverseProjection;
uniform vec3 LightPosition;
uniform vec3 LightColor;
uniform float LightIntensity;

layout(location = 0, index = 0) out vec4 Color;

vec3 pointLight(vec3 p, vec3 n, vec3 v, vec3 diffuseColor, vec3 specularColor, float specularPower)
{
    vec3 l = normalize(LightPosition - p);
    float ndotl = max(dot(n, l), 0.0);
    vec3 h = normalize(l+v);
    float ndoth = max(dot(n, h), 0.0);
    float d = distance(LightPosition, p);
    float att = 1.f / (d*d);
    return att * LightColor * LightIntensity * (diffuseColor * ndotl + specularColor * pow(ndoth, specularPower));
}

void main(void)
{
    vec4 colorBuffer = texture(ColorBuffer, In.Texcoord).rgba;
    vec4 normalBuffer = texture(NormalBuffer, In.Texcoord).rgba;
    float depth = texture(DepthBuffer, In.Texcoord).r;

    vec3 n = normalBuffer.rgb;
    vec3 diffuseColor = colorBuffer.rgb;
    vec3 specularColor = colorBuffer.aaa;
    float specularPower = normalBuffer.a;

    vec2 xy = In.Texcoord * 2.0 -1.0;
    vec4 wP = InverseProjection * vec4(xy, depth * 2.0 -1.0, 1.0);
    vec3 p = vec3(wP.xyz / wP.w);
    vec3 v = normalize(-p);

    Color = vec4(pointLight(p, n, v, diffuseColor, specularColor, specularPower), 1.0);
    //Color = vec4(LightColor, 1.0);
}
