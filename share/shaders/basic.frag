#version 130

#ifdef GL_ES
precision mediump float;
#endif

uniform int fog_active;
uniform int render_stage;
uniform mat4 model_matrix;

uniform sampler2DArray arrtex;
varying vec4 fcolor;
varying vec3 texco;
varying vec3 cpos;
varying vec3 inormal;
varying float flatkey;

void main(void) {
  if (render_stage == 0 && fcolor.w < 0.95) {
    // Drop transparent elements in first stage only.
    // Tolerance is generous since float packing may induce slight errors
    discard;
  }

  // Linear combinations of units need not be units
  vec3 normal;
  if (flatkey >= 1.) {
    normal = normalize(inormal);
  } else {
    normal = normalize(cross(dFdx(cpos),dFdy(cpos)));
  }
  float clamped = max(0, min(8, floor(16.*texco.z+0.5)));
  vec4 texcolor = fcolor * texture(arrtex, vec3(texco.xy,clamped));

  // Lighting model, not tuned very much
  vec3 L = normalize(gl_LightSource[0].position.xyz - cpos);
  vec3 E = normalize(-cpos);
  vec3 R = normalize(-reflect(L, normal));
  vec4 Iambient = texcolor * gl_LightSource[0].ambient;
  vec4 Idiffuse = texcolor * gl_LightSource[0].diffuse * max(dot(normal, L), 0.0);
  Idiffuse = clamp(Idiffuse, 0.0, 1.0);
//    vec4 Ispecular = texcolor * gl_LightSource[0].specular *
//                     pow(max(dot(R, E), 0.0), 0.3 * gl_FrontMaterial.shininess);
//    Ispecular = clamp(Ispecular, 0.0, 1.0);
  vec4 Ispecular = vec4(0, 0, 0, 0);

  vec4 surfcolor = gl_LightModel.ambient + Iambient + Idiffuse + Ispecular;
  surfcolor = clamp(surfcolor, 0.0, 1.0);

  float dist;
  if (fog_active == 0) {
    // Fog not active, skip
    dist = 0.;
  } else {
    // Apply linear fog as in original
    dist = clamp(1.0 - (gl_Fog.end - length(cpos)) * gl_Fog.scale, 0., 1.0);
  }
  // Force override alpha
  gl_FragColor = vec4(mix(surfcolor, gl_Fog.color, dist).xyz, fcolor.w);
}
