#version 410 core

#define POSITION	0
#define FRAG_COLOR	0

precision highp int;

uniform vec3 PersonalColor;
uniform float Time;

layout(location = FRAG_COLOR, index = 0) out vec4 FragColor;


void main()
{
    vec3 cdiff = (mix(0.9, 1.1, cos(Time * 3)) * PersonalColor);
    FragColor = vec4(cdiff, 1.0);
}
