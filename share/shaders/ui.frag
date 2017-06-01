#version 130

#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D tex;
varying vec4 color;
varying vec2 texco;

void main(void) {
  gl_FragColor = color * texture(tex, texco);
}
