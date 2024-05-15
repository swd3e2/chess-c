#version 330

uniform vec2 posOffset;
uniform int is_hovered;
uniform int is_selected;

uniform sampler2D tex;

in vec4 color;
in vec2 uv;

out vec4 frag_color;

void main() {
  vec4 texColor = texture(tex, uv);
  frag_color = texColor;
  if (is_selected > 0) {
    frag_color.xyz += vec3(0.31f);
  }
}
