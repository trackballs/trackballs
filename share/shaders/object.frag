#version 130

#ifdef GL_ES
precision mediump float;
#endif

uniform int fog_active;
uniform mat4 model_matrix;
uniform vec4 specular;
uniform float shininess;
uniform float use_lighting;

uniform sampler2D tex;
varying vec2 texco;
varying vec3 cpos;
varying vec3 inormal;
varying vec4 fcolor;
varying float flatkey;

void main(void) {
  // Object shader with custom specular color

  // Linear combinations of units need not be units
  vec3 normal;
  if (flatkey >= 1.) {
    normal = normalize(inormal);
  } else {
    normal = normalize(cross(dFdx(cpos),dFdy(cpos)));
  }
  vec4 texcolor = fcolor * texture(tex, texco);

  // Lighting model, not tuned very much
  vec3 L = normalize(gl_LightSource[0].position.xyz - cpos);
  vec3 E = normalize(-cpos);
  vec3 R = normalize(-reflect(L, normal));
  vec4 Iambient = texcolor * gl_LightSource[0].ambient;
  vec4 Idiffuse = texcolor * gl_LightSource[0].diffuse * max(dot(normal, L), 0.0);
  Idiffuse = clamp(Idiffuse, 0.0, 1.0);
  vec4 Ispecular = specular * gl_LightSource[0].specular *
                   pow(max(dot(R, E), 0.0), 0.2*shininess);
  Ispecular = clamp(Ispecular, 0.0, 1.0);

  vec4 surfcolor = gl_LightModel.ambient + Iambient + Idiffuse + Ispecular;
  if (use_lighting > 0.) {
    surfcolor = clamp(surfcolor, 0.0, 1.0);
  } else {
    surfcolor = texcolor;
  }


  float dist;
  if (fog_active == 0) {
    // Fog not active, skip
    dist = 0.;
  } else {
    // Apply linear fog as in original
    dist = clamp(1.0 - (gl_Fog.end - length(cpos)) * gl_Fog.scale, 0., 1.0);
  }
  // Force override alpha
  gl_FragColor = vec4(mix(surfcolor, gl_Fog.color, dist).xyz, texcolor.w);
}
