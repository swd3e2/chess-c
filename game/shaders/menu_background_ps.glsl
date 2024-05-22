#version 330

uniform vec2 uv_offset;
uniform float time;

uniform sampler2D tex;

out vec4 frag_color;

in vec4 color;
in vec2 texcoords;

void main() {
  frag_color = texture(tex, texcoords + uv_offset);
}