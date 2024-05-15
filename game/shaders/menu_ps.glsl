#version 330

uniform int is_hovered;

out vec4 frag_color;

in vec4 color;

void main() {
  frag_color = color;
}