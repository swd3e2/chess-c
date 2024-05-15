#version 330

uniform int is_hovered;

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color0;

out vec4 color;

void main() {
  gl_Position.xy = position;
  gl_Position.z = -1.0f;
  color = color0;
}