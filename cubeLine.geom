#version 410 core

layout(lines) in;
layout(triangle_strip, max_vertices = 36) out;


uniform mat4 MVP;

void main()
{
    float HalfLineWidth = 0.03f;
    float HalfLineHeight = 1.0f;

    vec3 n = (gl_in[1].gl_Position - gl_in[0].gl_Position).xyz;
    vec3 n2 = vec3(0, 0, 1);
    vec3 nOffset = normalize(cross(n, n2)) * HalfLineWidth;
    vec3 nOffsetInv = -1 * nOffset;

    vec4 nOffsetInvTop = vec4(nOffsetInv, 0.0);
    //nOffsetInvTop.z += HalfLineHeight;
    vec4 nOffsetInvBottom = vec4(nOffsetInv, 0.0);
    nOffsetInvBottom.z -= HalfLineHeight;
    vec4 nOffsetTop = vec4(nOffset, 0.0);
    //nOffsetTop.z += HalfLineHeight;
    vec4 nOffsetBottom = vec4(nOffset, 0.0);
    nOffsetBottom.z -= HalfLineHeight;

    // Faces
    for (int i = 0; i < 2; ++i) {
        gl_Position = MVP * (gl_in[i].gl_Position + nOffsetInvTop);
        EmitVertex();

        gl_Position = MVP * (gl_in[i].gl_Position + nOffsetTop);
        EmitVertex();

        gl_Position = MVP * (gl_in[i].gl_Position + nOffsetInvBottom);
        EmitVertex();

        gl_Position = MVP * (gl_in[i].gl_Position + nOffsetBottom);
        EmitVertex();

        EndPrimitive();
    }

    // Top
    gl_Position = MVP * (gl_in[0].gl_Position + nOffsetInvTop);
    EmitVertex();

    gl_Position = MVP * (gl_in[1].gl_Position + nOffsetInvTop);
    EmitVertex();

    gl_Position = MVP * (gl_in[0].gl_Position + nOffsetTop);
    EmitVertex();

    gl_Position = MVP * (gl_in[1].gl_Position + nOffsetTop);
    EmitVertex();

    EndPrimitive();

    // Bottom
    gl_Position = MVP * (gl_in[0].gl_Position + nOffsetInvBottom);
    EmitVertex();

    gl_Position = MVP * (gl_in[1].gl_Position + nOffsetInvBottom);
    EmitVertex();

    gl_Position = MVP * (gl_in[0].gl_Position + nOffsetBottom);
    EmitVertex();

    gl_Position = MVP * (gl_in[1].gl_Position + nOffsetBottom);
    EmitVertex();

    EndPrimitive();

    // Left
    gl_Position = MVP * (gl_in[0].gl_Position + nOffsetInvBottom);
    EmitVertex();

    gl_Position = MVP * (gl_in[1].gl_Position + nOffsetInvBottom);
    EmitVertex();

    gl_Position = MVP * (gl_in[0].gl_Position + nOffsetInvTop);
    EmitVertex();

    gl_Position = MVP * (gl_in[1].gl_Position + nOffsetInvTop);
    EmitVertex();

    EndPrimitive();

    // Right
    gl_Position = MVP * (gl_in[0].gl_Position + nOffsetBottom);
    EmitVertex();

    gl_Position = MVP * (gl_in[1].gl_Position + nOffsetBottom);
    EmitVertex();

    gl_Position = MVP * (gl_in[0].gl_Position + nOffsetTop);
    EmitVertex();

    gl_Position = MVP * (gl_in[1].gl_Position + nOffsetTop);
    EmitVertex();

    EndPrimitive();
}
