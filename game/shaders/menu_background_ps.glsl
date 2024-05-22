#version 330

uniform int is_hovered;

uniform sampler2D tex;

out vec4 frag_color;

in vec4 color;
in vec2 texcoords;

void main() {
  frag_color = texture(tex, texcoords);
  if (is_hovered == 1) {
    frag_color.xyz -= vec3(0.2f);
  }
}