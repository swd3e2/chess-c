#version 330

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color0;
layout(location = 2) in vec2 in_uv;

out vec4 color;
out vec2 uv;

void main() {
  gl_Position.xy = position;
  gl_Position.z = -1.0f;

  color = color0;
}
