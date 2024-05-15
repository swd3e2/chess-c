#version 330

uniform int is_hovered;
uniform int possible_move;

out vec4 frag_color;

in vec4 color;

void main() {
  if (is_hovered == 1) {
    frag_color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
  } else if (possible_move == 1) {
    frag_color = color + vec4(0.0f, 0.3f, 0.0f, 1.0f);
  } else {
    frag_color = color;
  }
}