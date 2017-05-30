#version 130

#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform mat4 proj_matrix;
uniform mat4 model_matrix;

uniform float gameTime;

attribute vec3 in_Position;
attribute vec4 in_Color;
attribute vec4 in_Texcoord;
attribute vec2 in_Velocity;
attribute vec4 in_Normal;

varying vec4 fcolor;
varying vec3 texco;
varying vec3 cpos;
varying vec3 inormal;
varying float flatkey;

void main(void) {
  vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.);
  mat4 mvp_matrix = proj_matrix * model_matrix;
  gl_Position = mvp_matrix * pos;
  cpos = vec4(model_matrix * pos).xyz;
  fcolor = in_Color;
  vec2 vel = mod(-gameTime * in_Velocity, 1.);
  texco = vec3(vec2(in_Texcoord.xy + vel).xy,in_Texcoord.z);

  if (distance(in_Normal.xyz,vec3(0.5,0.5,0.5)) > 0.1) {
    flatkey = 1.0;
  } else {
    flatkey = -1.0;
  }

  // As model_matrix M is orthogonal, M = transpose(inverse(M))
  inormal = vec4(model_matrix*2.*vec4(in_Normal.xyz-0.5,0.)).xyz;
}
