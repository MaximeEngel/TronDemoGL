#version 410 core

layout(lines) in;
layout(triangle_strip, max_vertices = 36) out;


uniform mat4 MVP;
uniform int TotalPoints;
uniform vec4 CurrentFinalPosition;
out vec4 position;

void main()
{
    float HeightOffset = 0.08f;
    float HalfLineWidth = 0.03f;
    float HalfLineHeight = 1.1f + HeightOffset;
    vec4 pos[2];
    pos[0] = gl_in[0].gl_Position;
    if (gl_PrimitiveIDIn >= TotalPoints - 2) {
        pos[1] = CurrentFinalPosition;
    } else {
        pos[1] = gl_in[1].gl_Position;
    }

    vec3 n = (pos[1] - pos[0]).xyz;
    vec3 n2 = vec3(0, 0, 1);
    vec3 nOffset = normalize(cross(n, n2)) * HalfLineWidth;
    vec3 nOffsetInv = -1 * nOffset;

    vec4 nOffsetInvTop = vec4(nOffsetInv, 0.0);
    nOffsetInvTop.z += HalfLineHeight;
    vec4 nOffsetInvBottom = vec4(nOffsetInv, 0.0);
    nOffsetInvBottom.z += HeightOffset;
    vec4 nOffsetTop = vec4(nOffset, 0.0);
    nOffsetTop.z += HalfLineHeight;
    vec4 nOffsetBottom = vec4(nOffset, 0.0);
    nOffsetBottom.z += HeightOffset;

    // Faces
    for (int i = 0; i < 2; ++i) {
        gl_Position = MVP * (pos[i] + nOffsetInvTop);
        position = gl_Position;
        EmitVertex();

        gl_Position = MVP * (pos[i] + nOffsetTop);
        position = gl_Position;
        EmitVertex();

        gl_Position = MVP * (pos[i] + nOffsetInvBottom);
        position = gl_Position;
        EmitVertex();

        gl_Position = MVP * (pos[i] + nOffsetBottom);
        position = gl_Position;
        EmitVertex();

        EndPrimitive();
    }

    // Top
    gl_Position = MVP * (pos[0] + nOffsetInvTop);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[1] + nOffsetInvTop);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[0] + nOffsetTop);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[1] + nOffsetTop);
    position = gl_Position;
    EmitVertex();

    EndPrimitive();

    // Bottom
    gl_Position = MVP * (pos[0] + nOffsetInvBottom);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[1] + nOffsetInvBottom);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[0] + nOffsetBottom);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[1] + nOffsetBottom);
    position = gl_Position;
    EmitVertex();

    EndPrimitive();

    // Left
    gl_Position = MVP * (pos[0] + nOffsetInvBottom);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[1] + nOffsetInvBottom);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[0] + nOffsetInvTop);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[1] + nOffsetInvTop);
    position = gl_Position;
    EmitVertex();

    EndPrimitive();

    // Right
    gl_Position = MVP * (pos[0] + nOffsetBottom);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[1] + nOffsetBottom);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[0] + nOffsetTop);
    position = gl_Position;
    EmitVertex();

    gl_Position = MVP * (pos[1] + nOffsetTop);
    position = gl_Position;
    EmitVertex();

    EndPrimitive();
}
