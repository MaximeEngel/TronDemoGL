#version 410 core

#define POSITION	0

precision highp float;
precision highp int;

layout(location = POSITION) in vec3 Position;


void main()
{	
    gl_Position = vec4(Position, 1.0);
}
