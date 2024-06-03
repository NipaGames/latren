#version 330 core

layout (location = 0) in vec3 begin;
layout (location = 1) in vec3 end;

out VS_OUT {
    vec3 begin;
    vec3 end;
} vs_out;

void main() {
    vs_out.begin = begin;
    vs_out.end = end;
}