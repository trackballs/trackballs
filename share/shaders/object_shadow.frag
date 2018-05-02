#version 130

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D tex;

varying vec2 texco;
varying vec4 fcolor;

void main(void) {
  vec4 texcolor = fcolor * texture(tex, texco);
  if (texcolor.w < 0.001) { discard; }
}
