#version 410 core

#define POSITION	0
#define NORMAL		1
#define TEXCOORD	2
#define FRAG_COLOR	0

precision highp int;

uniform sampler2D Diffuse;
uniform vec3 DiffuseColor;
uniform sampler2D Specular;
uniform vec3 SpecularColor;
uniform vec3 PersonalColor;
uniform float Time;

in block
{
	vec2 TexCoord;
        vec3 Normal;
        vec3 Position;
} In;

layout(location = 0) out vec4 Color;
layout(location = 1) out vec4 Normal;

subroutine vec3 diffuseColor();


subroutine (diffuseColor) vec3 diffuseUniform()
{
	return DiffuseColor;
}

subroutine (diffuseColor) vec3 diffuseTexture()
{
	return texture(Diffuse, vec2(In.TexCoord.x, 1. - In.TexCoord.y) ).rgb;
}


subroutine uniform diffuseColor DiffuseColorSub;


subroutine vec3 specularColor();


subroutine (specularColor) vec3 specularUniform()
{
        return SpecularColor;
}

subroutine (specularColor) vec3 specularTexture()
{
        return texture(Specular, vec2(In.TexCoord.x, 1. - In.TexCoord.y) ).rgb;
}


subroutine uniform specularColor SpecularColorSub;

void main()
{
        vec3 cdiff = DiffuseColorSub() * (mix(0.9, 1.1, cos(Time * 3)) * PersonalColor);
        vec3 cspec = SpecularColorSub();;
        Color = vec4(cdiff, cspec.r);
        Normal = vec4(In.Normal, 2);

}
