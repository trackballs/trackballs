#version 130

#ifdef GL_ES
precision mediump float;
#endif

uniform int fog_active;
uniform vec4 line_color;

varying vec3 cpos;

void main(void) {
  // Lines are simple!
  float dist;
  if (fog_active == 0) {
    // Fog not active, skip
    dist = 0.;
  } else {
    // Apply linear fog as in original
    dist = clamp(1.0 - (gl_Fog.end - length(cpos)) * gl_Fog.scale, 0., 1.0);
  }
  gl_FragColor = vec4(mix(line_color, gl_Fog.color, dist).xyz, line_color.w);
}
