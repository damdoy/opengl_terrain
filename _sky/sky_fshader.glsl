#version 330 core

in vec3 frag_normal_transformed;
in vec3 frag_position;

uniform vec3 camera_position;
uniform vec3 light_position;
uniform uint lighting_mode;
uniform bool activate_specular;
uniform vec3 spot_direction;
uniform bool activate_spot;
uniform float sky_size;

in vec3 sun_dir_corr;

out vec4 color;

const vec3 night_colour = vec3(0.0, 0.0, 0.5);
const vec3 day_colour = vec3(0.5, 0.5, 1.0);
const vec3 red_sky = vec3(0.8, 0.3, 0.1);

//really simple noise
float noise(vec3 p){
    return (sin(p.x*0.12)*sin(p.y*0.16)*sin(p.z*0.20))+(cos(p.x*0.4)*cos(p.y*0.2)*cos(p.z*0.14));
}


void main(){
   vec3 sky_colour = vec3(0.0, 0.0, 0.0);

   mat3x3 rotation_matrix = mat3(vec3(sun_dir_corr[1], sun_dir_corr[0], 0),
                                 vec3(-sun_dir_corr[0], sun_dir_corr[1], 0),
                                 vec3(0, 0, 1));

   float star_prob = noise(rotation_matrix*frag_position);

   vec3 night_pixel = night_colour;

   //draws stars from noise (only peaks in noise are stars)
   if(star_prob > 1.65){
      night_pixel = vec3(1.0);
   }

   sky_colour = night_pixel;

   float up_sun = dot(sun_dir_corr, vec3(0.0, -1.0, 0.0));
   float evening_sun = dot(sun_dir_corr, vec3(-1.0, 0.0, 0.0));

   if( up_sun > -0.25){

      float day_quantity = clamp((up_sun+0.25)*3, 0.0, 1.0);

      sky_colour = day_quantity*day_colour+(1.0-day_quantity)*night_pixel;
   }

   vec3 pos_sun = (-sun_dir_corr)*sky_size + sun_dir_corr*8;
   vec3 frag_position_comp = frag_position-pos_sun;

   if(evening_sun > 0.8){

      float evening_quantity = (evening_sun-0.8)*(1/0.2);

      sky_colour = (evening_quantity)*red_sky+(1.0-evening_quantity)*sky_colour;
   }

   //sun colour
   if(dot(frag_position_comp, sun_dir_corr) < 0){
      sky_colour = vec3(1.0, 1.0, 1.0);
   }

   color.a = 1.0;
   color.rgb = sky_colour;
}
