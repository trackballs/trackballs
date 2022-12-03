#version 130

#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform mat4 mvp_matrix;
uniform mat4 model_matrix;
uniform mat4 object_matrix;

attribute vec3 in_Position;
attribute vec4 in_Color;
attribute vec2 in_Texcoord;
attribute vec4 in_Normal;

varying vec3 cpos;
varying vec3 inormal;
varying vec3 upvector;

void main(void) {
  vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.);
  gl_Position = mvp_matrix * (object_matrix * pos);
  cpos = vec4(model_matrix * (object_matrix * pos)).xyz;
  upvector = vec4(model_matrix * vec4(0, 0, 1, 0)).xyz;

  // As model_matrix M is orthogonal, M = transpose(inverse(M))
  inormal = vec4(model_matrix * (object_matrix * 2. * vec4(in_Normal.xyz - 0.5, 0.))).xyz;
}
