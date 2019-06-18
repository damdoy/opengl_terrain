#version 330 core

in vec3 frag_position;

uniform vec3 camera_position;
uniform vec3 light_position;

uniform sampler2D shadow_buffer_tex;

in vec3 frag_normal_transformed;
in float red;
in float green;
in float blue;
out vec3 color;

in vec4 shadow_coord;

void main(){
   vec3 light_dir = normalize(light_position-frag_position);
   float diffuse_light = 0.0;

   diffuse_light = dot(frag_normal_transformed, light_dir);
   float light_dist = length(light_position-frag_position);
   diffuse_light /= 1+pow(light_dist, -0.5);

   float lum = 0.8*diffuse_light;

   float shadow = 1;

   if ( texture( shadow_buffer_tex, shadow_coord.xy/shadow_coord.w ).x  <  shadow_coord.z/shadow_coord.w){
       shadow = 0.5;
   }

   color = vec3(lum, lum, lum);
}

void main_colors_const(){
   color = vec3(red, green, blue);
}
