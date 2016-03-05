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

in block
{
	vec2 TexCoord;
        vec3 Normal;
        vec3 Position;
} In;

layout(location = FRAG_COLOR, index = 0) out vec4 FragColor;

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
        vec3 cdiff = DiffuseColorSub() * PersonalColor;
        vec3 cspec = SpecularColorSub();
	vec3 n = In.Normal;
        vec3 l = vec3(1., 1., 1.);
	float ndotl = clamp(dot(n,l), 0., 1.);
	vec3 diffuse = cdiff * ndotl;
        vec3 h = normalize(l-normalize(In.Position));
        float ndoth = max(dot(n, h), 0.0);
        vec3 specular = cspec * pow(ndoth, 2);
        FragColor = vec4(diffuse + specular, 1.0);
}
