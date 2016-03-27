#version 410 core

in block
{
        vec2 Texcoord;
} In;

uniform sampler2D Texture;
uniform float Gamma = 1.0;
uniform float Time;
uniform float Width;
uniform float Height;

layout(location = 0, index = 0) out vec4  Color;

void main(void)
{
        vec3 color = texture(Texture, In.Texcoord).rgb;
//        if (Time > 5) {
//            color += vec3(In.Texcoord * color.rb * (cos(Time) + 0.5), 0.0);
//        }
        Color = vec4(pow(color, vec3(1.0/Gamma)), 1.0);
}
