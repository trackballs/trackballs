#ifdef GL_ES
precision mediump int;
precision mediump float;
#endif

uniform mat4 proj_matrix;
uniform mat4 model_matrix;

uniform float gameTime;

attribute vec3 in_Position;
attribute vec4 in_Color;
attribute vec2 in_Texcoord;
attribute vec2 in_Velocity;
attribute vec4 in_Normal;

varying vec3 cpos;
varying vec2 texco;
varying vec3 inormal;

void main(void) {
  vec4 pos = vec4(in_Position.x, in_Position.y, in_Position.z, 1.);
  mat4 mvp_matrix = proj_matrix * model_matrix;
  gl_Position = mvp_matrix * pos;
  cpos = vec4(model_matrix * pos).xyz;

  vec2 vel = mod(-gameTime * 4. * in_Velocity, 1.);

  float npa = in_Position.x + 69. * in_Position.y + 5. * gameTime;
  // noise2 doesn't do anything?
  vec2 noi = 0.3 * vec2(sin(npa), cos(npa));

  texco = vec2(0.625, 0.125) + 0.125 * in_Texcoord + 0.125 * vel + 0.125 * noi;

  inormal = vec4(model_matrix*2.*vec4(in_Normal.xyz-0.5,0.)).xyz;
}
