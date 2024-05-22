#version 330

uniform vec2 uv_offset;
uniform float time;

layout(location = 0) in vec2 position;
layout(location = 1) in vec4 color0;
layout(location = 2) in vec2 in_texcoords;

out vec4 color;
out vec2 texcoords;

void main() {
  gl_Position.xy = position;
  gl_Position.z = -1.0f;
  texcoords = in_texcoords;
  color = color0;
}