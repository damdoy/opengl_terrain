#ifndef SKY_SPHERE_H
#define SKY_SPHERE_H

#include "../_sphere/sphere.h"

class Sky_sphere : public Sphere{
public:
   void init(unsigned int vertical_segments = 0, unsigned int horizontal_segments = 0, float sky_size = 512.0f){

      Sphere::init(vertical_segments, horizontal_segments, true);

      sun_dir[0] = sin(sun_counter);
      sun_dir[1] = cos(sun_counter);
      sun_dir[2] = 0;

      sun_rgb[0] = 1.0;
      sun_rgb[1] = 1.0;
      sun_rgb[2] = 0.9;

      const_sun_daylight[0] = 1.0;
      const_sun_daylight[1] = 1.0;
      const_sun_daylight[2] = 0.9;

      const_sun_night[0] = 0.0;
      const_sun_night[1] = 0.0;
      const_sun_night[2] = 0.1;

      const_sun_red[0] = 1.0;
      const_sun_red[1] = 0.3;
      const_sun_red[2] = 0.1;

      sun_counter = 2.0;

      this->sky_size = sky_size;

      glDeleteProgram(_pid); //remove previous shader

      _pid = load_shaders_files("sky_vshader.glsl", "sky_fshader.glsl");
      if(_pid == 0) exit(-1);
   }

   void draw(){
      glFrontFace(GL_CW);

      glUseProgram(_pid);

      glUniform3fv( glGetUniformLocation(_pid, "sun_dir"), 1, this->sun_dir);
      glUniform1f( glGetUniformLocation(_pid, "sky_size"), this->sky_size);

      Sphere::draw();

      glFrontFace(GL_CCW);
   }

   void advance_sun(){
      sun_counter += 0.005;
      sun_dir[0] = sin(sun_counter);
      sun_dir[1] = cos(sun_counter);

      sun_rgb[0] = const_sun_daylight[0];
      sun_rgb[1] = const_sun_daylight[1];
      sun_rgb[2] = const_sun_daylight[2];

      sun_rgb[0] = const_sun_night[0];
      sun_rgb[1] = const_sun_night[1];
      sun_rgb[2] = const_sun_night[2];

      float up_sun = sun_dir[1]*(-1.0);

      float evening_sun = sun_dir[0]*(-1.0);

      if(up_sun > -0.25){
         float day_quantity = (up_sun+0.25)*3;

         if(day_quantity > 1){
            day_quantity = 1;
         }

         if(day_quantity < 0){
            day_quantity = 0;
         }

         sun_rgb[0] = day_quantity*const_sun_daylight[0]+(1.0-day_quantity)*const_sun_night[0];
         sun_rgb[1] = day_quantity*const_sun_daylight[1]+(1.0-day_quantity)*const_sun_night[1];
         sun_rgb[2] = day_quantity*const_sun_daylight[2]+(1.0-day_quantity)*const_sun_night[2];
      }

      if(evening_sun > 0.8){

         float evening_quantity = (evening_sun-0.8)*(1/0.2);

         sun_rgb[0] = evening_quantity*const_sun_red[0]+(1.0f-evening_quantity)*sun_rgb[0];
         sun_rgb[1] = evening_quantity*const_sun_red[1]+(1.0f-evening_quantity)*sun_rgb[1];
         sun_rgb[2] = evening_quantity*const_sun_red[2]+(1.0f-evening_quantity)*sun_rgb[2];
      }

   }

   float* get_sun_direction(){
      return sun_dir;
   }

   float* get_sun_pos(){
      return sun_pos;
   }

   float* get_sun_rgb_color(){
      return sun_rgb;
   }

protected:
   float sun_dir[3];
   float sun_pos[3];
   float sun_rgb[3];
   float const_sun_daylight[3];
   float const_sun_night[3];
   float const_sun_red[3];

   float sun_counter;
   float sky_size;
};

#endif
