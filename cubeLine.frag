#version 410 core

#define POSITION	0
#define FRAG_COLOR	0

precision highp int;

uniform vec3 PersonalColor;
uniform float Time;

layout(location = FRAG_COLOR, index = 0) out vec4 FragColor;
in vec4 position;

highp float rand(vec2 co)
{
    highp float a = 12.9898;
    highp float b = 78.233;
    highp float c = 43758.5453;
    highp float dt= dot(co.xy ,vec2(a,b));
    highp float sn= mod(dt,3.14);
    return fract(sin(sn) * c);
}

void main()
{
    vec3 cdiff = (mix(0.9, 1.0f, cos(Time * 3)) * PersonalColor);
    cdiff += clamp(-0.5 + rand(vec2(Time, Time * position.x)), -0.1f, 0.0f);
    FragColor = vec4(cdiff, 1.0);
}
