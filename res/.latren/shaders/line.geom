#version 330 core

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
    vec3 begin;
    vec3 end;
} gs_in[];

uniform mat4 projection, view;

void main() {
    gl_Position = projection * view * vec4(gs_in[0].begin, 1.0); 
    EmitVertex();

    gl_Position = projection * view * vec4(gs_in[0].end, 1.0);
    EmitVertex();
    
    EndPrimitive();
}  