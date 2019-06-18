#version 330 core

in vec3 frag_position;

uniform vec3 camera_position;
uniform vec3 light_position;

uniform sampler2D tex_wind; //wi
uniform sampler2D tex;

// in vec3 frag_normal_transformed;
out vec4 color;

// in float frag_diffuse_light;
in float shadow_val;
in vec2 frag_uv;
in vec2 global_uv;
in vec3 frag_light_intensity;

void main(){

   //poor random
   float rand_val = mod((global_uv.x*global_uv.x)+(0.13-global_uv.x*global_uv.y), 0.01)/0.01;

   float dist_to_centre = pow(distance(frag_uv, vec2(0.5, 0.5)), 2); //to give a "grass like" colours

   float dist_to_ground = clamp((1.0-frag_uv.y)*1, 0.0, 1.0); //darker at the base of the blade

   color.rgb = vec3( (0.1+rand_val/6.0+dist_to_centre)*dist_to_ground, (0.3+dist_to_centre)*dist_to_ground, (0.0+dist_to_centre)*dist_to_ground)*frag_light_intensity*shadow_val;
   color.a = 1.0;
}
