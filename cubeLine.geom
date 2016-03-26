#version 410 core

layout(lines) in;
layout(triangle_strip, max_vertices = 36) out;


uniform mat4 MVP;
uniform int TotalPoints;
uniform vec4 CurrentFinalPosition;
out vec4 position;
out float offsetY;

void emitPointLine(vec4 pos, float topBottom) {
    gl_Position = MVP * pos;
    position = pos;
    offsetY = topBottom;
    EmitVertex();
}

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

    // Ends
    for (int i = 0; i < 2; ++i) {
        emitPointLine(pos[i] + nOffsetInvTop, 1.0);
        emitPointLine(pos[i] + nOffsetTop, 1.0);
        emitPointLine(pos[i] + nOffsetInvBottom, 0.0);
        emitPointLine(pos[i] + nOffsetBottom, 0.0);
        EndPrimitive();
    }

    // Top
    emitPointLine(pos[0] + nOffsetInvTop, 1.0);
    emitPointLine(pos[1] + nOffsetInvTop, 1.0);
    emitPointLine(pos[0] + nOffsetTop, 1.0);
    emitPointLine(pos[1] + nOffsetTop, 1.0);
    EndPrimitive();

    // Bottom
    emitPointLine(pos[0] + nOffsetInvBottom, 0.0);
    emitPointLine(pos[1] + nOffsetInvBottom, 0.0);
    emitPointLine(pos[0] + nOffsetBottom, 0.0);
    emitPointLine(pos[1] + nOffsetBottom, 0.0);
    EndPrimitive();

    // Left
    emitPointLine(pos[0] + nOffsetInvBottom, 0.0);
    emitPointLine(pos[1] + nOffsetInvBottom, 0.0);
    emitPointLine(pos[0] + nOffsetInvTop, 1.0);
    emitPointLine(pos[1] + nOffsetInvTop, 1.0);
    EndPrimitive();

    // Right
    emitPointLine(pos[0] + nOffsetBottom, 0.0);
    emitPointLine(pos[1] + nOffsetBottom, 0.0);
    emitPointLine(pos[0] + nOffsetTop, 1.0);
    emitPointLine(pos[1] + nOffsetTop, 1.0);
    EndPrimitive();
}
