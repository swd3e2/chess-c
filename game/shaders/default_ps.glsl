#version 330

uniform int is_hovered;
uniform sampler2D tex;

in vec4 color;
in vec2 uv;

out vec4 frag_color;

void main() {
  vec4 texColor = texture(tex, uv);
  if(texColor.a < 0.1) discard;

  if (is_hovered > 0) {
    frag_color = vec4(1.0f);
  } else {
    frag_color = texColor;
  }
}
