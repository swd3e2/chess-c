#version 330

uniform int is_hovered;

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color0;
layout(location = 2) in vec2 uv;

out vec4 color;

void main() {
  gl_Position.xy = position.xy;
  gl_Position.z = -1;
  color = color0;
}