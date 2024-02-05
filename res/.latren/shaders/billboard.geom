#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT {
    vec3 pos;
} gs_in[];

uniform mat4 projection, view;
uniform vec3 viewPos;

out vec2 fragmentTexCoord;

void main() {
    vec3 pos = gs_in[0].pos;

    vec3 toCamera = normalize(viewPos - pos);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(toCamera, up);
    mat4 vp = projection * view;

    pos -= (right * 0.5);
    gl_Position = vp * vec4(pos, 1.0);
    fragmentTexCoord = vec2(1.0, 1.0);
    EmitVertex();

    pos.y += 1.0;
    gl_Position = vp * vec4(pos, 1.0);
    fragmentTexCoord = vec2(1.0, 0.0);
    EmitVertex();

    pos.y -= 1.0;
    pos += right;
    gl_Position = vp * vec4(pos, 1.0);
    fragmentTexCoord = vec2(0.0, 1.0);
    EmitVertex();

    pos.y += 1.0;
    gl_Position = vp * vec4(pos, 1.0);
    fragmentTexCoord = vec2(0.0, 0.0);
    EmitVertex();

    EndPrimitive();
}