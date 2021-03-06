#version 410 core

#define POSITION	0
#define NORMAL		1
#define TEXCOORD	2
#define FRAG_COLOR	0

precision highp float;
precision highp int;

uniform mat4 MVP;
uniform mat4 MV;
uniform float Time;

layout(location = POSITION) in vec3 Position;
layout(location = NORMAL) in vec3 Normal;
layout(location = TEXCOORD) in vec2 TexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

out block
{
	vec2 TexCoord;
	vec3 Normal;
        vec3 Position;
} Out;

void main()
{	
	gl_Position = MVP * vec4(Position, 1.0);
	Out.TexCoord = TexCoord;
        Out.Normal = vec3(MV * vec4(Normal, 0.0));
        Out.Position = vec3(MV * vec4(Position, 1.0));
}
