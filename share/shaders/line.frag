#version 130

#ifdef GL_ES
precision mediump float;
#endif

uniform vec4 line_color;

uniform int fog_active;
uniform float fog_start;
uniform float fog_end;
uniform vec3 fog_color;

varying vec3 cpos;

void main(void) {
  // Lines are simple!
  vec3 lcolor = line_color.xyz;
  if (fog_active == 1) {
    float dist = clamp(1.0 - (fog_end - length(cpos)) / (fog_end - fog_start), 0., 1.0);
    lcolor = mix(lcolor, fog_color, dist);
  }
  gl_FragColor = vec4(lcolor, line_color.w);
}
