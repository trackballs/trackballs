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
  float dist;
  if (fog_active == 0) {
    // Fog not active, skip
    dist = 0.;
  } else {
    // Apply linear fog as in original
    dist = clamp(1.0 - (fog_end - length(cpos)) / (fog_end - fog_start), 0., 1.0);
  }
  gl_FragColor = vec4(mix(line_color.xyz, fog_color, dist), line_color.w);
}
