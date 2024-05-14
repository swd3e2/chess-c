#version 330

uniform int is_hovered;

out vec4 frag_color;

in vec4 color;

void main() {
  if (is_hovered == 0) {
    frag_color = color;
  } else {
    frag_color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
  }
}