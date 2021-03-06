#version 130

#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;
uniform mat4 model_matrix;

attribute vec3 in_Position;

varying vec3 cpos;

void main(void) {
  vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.);
  gl_Position = mvp_matrix * pos;
  cpos = vec4(model_matrix * pos).xyz;
}
