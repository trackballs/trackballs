#version 130

#ifdef GL_ES
precision mediump float;
#endif

uniform int render_stage;

varying vec4 fcolor;

void main(void) {
  // Transparent elements in the opaque pass cast no shadows
  if (render_stage == 0 && fcolor.w < 0.95) { discard; }
}
