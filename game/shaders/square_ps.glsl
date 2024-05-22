#version 330

uniform int possible_move;
uniform int attack_move;

out vec4 frag_color;

in vec4 color;

void main() {
  if (possible_move == 1) {
    frag_color = color + vec4(0.2f, 0.2f, 0.2f, 1.0f);
  } else if (attack_move == 1) {
    frag_color = color + vec4(0.4f, 0.2f, 0.2f, 1.0f);
  } else {
    frag_color = color;
  }
}