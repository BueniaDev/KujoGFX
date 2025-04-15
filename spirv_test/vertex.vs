#version 330 core
layout (location = 0) in vec3 vertpos_modelspace;

void main()
{
    gl_Position.xyz = vertpos_modelspace;
    gl_Position.w = 1.0;
}