#version 130

#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform float screen_width;
uniform float screen_height;

attribute vec2 in_Position;
attribute vec4 in_Color;
attribute vec2 in_Texcoord;

varying vec4 color;
varying vec2 texco;

void main(void) {
  gl_Position = vec4(2 * in_Position.x / screen_width - 1,
                     1 - 2 * in_Position.y / screen_height, 0., 1.);
  color = in_Color;
  texco = in_Texcoord;
}
