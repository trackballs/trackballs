#version 130

#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;
uniform mat4 model_matrix;

uniform float gameTime;

attribute vec3 in_Position;
attribute vec4 in_Color;
attribute vec2 in_Texcoord;
attribute vec4 in_Normal;
attribute vec2 in_Velocity;

varying vec3 cpos;
varying vec2 texco;
varying vec3 inormal;
varying vec3 worldpos;

void main(void) {
  worldpos = in_Position;
  vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.);
  gl_Position = mvp_matrix * pos;
  cpos = vec4(model_matrix * pos).xyz;

  vec2 vel = mod(-gameTime * 4. * in_Velocity, 1.);

  float npa = in_Position.x + 69. * in_Position.y + 5. * gameTime;
  // noise2 doesn't do anything?
  vec2 noi = 0.3 * vec2(sin(npa), cos(npa));

  texco = in_Texcoord + vel + noi;

  inormal = vec4(model_matrix * 2. * vec4(in_Normal.xyz - 0.5, 0.)).xyz;
}
