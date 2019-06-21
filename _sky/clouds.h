#ifndef CLOUDS_H
#define CLOUDS_H

#include <vector>
#include <cmath>

#include "../drawable.h"
#include "../_plane/plane.h"
#include "../noise_generator.hpp"

class Clouds : public Drawable{

public:
   Clouds(){

   }

   void init(){
      _pid = load_shaders_files("clouds_vshader.glsl", "clouds_fshader.glsl");

      sky_plane.init(_pid);

      clouds_pos[0] = 0.0f;
      clouds_pos[1] = 0.0f;

      transf_sky.rotate(1.0f, 0.0f, 0.0f, 3.1415f);
      transf_sky.scale(3000.0f, 1.0f, 3000.0f);
      transf_sky.translate(0.0f, -150.0f, 0.0f);

      //2d perlin noise for clouds
      noise_gen.set_noise_function(NOISE_SELECT_PERLIN);
      noise_gen.set_noise_level(2);

      std::vector< std::vector<float> > noise_vec = noise_gen.get_2D_noise(1024, 1024, 0.0f, 8.0f, 0.0f, 8.0f);

      clouds_texture.set_data(noise_vec);

      glGenTextures(1, &_tex_clouds);
      glBindTexture(GL_TEXTURE_2D, _tex_clouds);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, clouds_texture.get_width(), clouds_texture.get_height(), 0, GL_RED, GL_FLOAT, clouds_texture.get_tex_data());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); //can be clamp to edge, clamp to border or gl repeat
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT); //repeat is best to avoid hard cuts in cloud texture
      GLuint tex_id = glGetUniformLocation(_pid, "tex_clouds");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

      clouds_level = 0.5f;
   }

   virtual void set_model_matrix(glm::mat4x4 model){
      this->model_matrix = model;
      sky_plane.set_model_matrix(model*transf_sky.get_matrix());
   }

   virtual void set_view_matrix(glm::mat4x4 view){
      this->view_matrix = view;
      sky_plane.set_view_matrix(view);
   }

   virtual void set_projection_matrix(glm::mat4x4 projection){
      this->projection_matrix = projection;
      sky_plane.set_projection_matrix(projection);
   }

   //0.0 = not cloudy, 1.0 = very cloudy
   virtual void set_level(float clouds_level){
      if(clouds_level > 1.0f){
         this->clouds_level = 1.0f;
      }
      else if(clouds_level <= 0.0f){
         this->clouds_level = 0.0f;
      }
      else{
         this->clouds_level = clouds_level;
      }
   }

   void draw(){
      glUseProgram(_pid);

      //move clouds slowly
      clouds_pos[0] += 0.03f;
      clouds_pos[1] += 0.02f;

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex_clouds);
      GLuint tex_id = glGetUniformLocation(_pid, "tex_clouds");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

      glUniform1f(glGetUniformLocation(_pid, "clouds_level"), this->clouds_level);

      glUniform2fv( glGetUniformLocation(_pid, "clouds_pos"), 1, this->clouds_pos);

      glUniform3fv( glGetUniformLocation(_pid, "sun_dir"), 1, this->sun_dir);
      glUniform3fv( glGetUniformLocation(_pid, "sun_col"), 1, this->sun_col);

      sky_plane.draw_without_pid();
      glUseProgram(0);
   }

protected:
   Plane sky_plane;
   Transform transf_sky;
   Noise_generator noise_gen;
   float clouds_pos[2];

   GLuint _tex_clouds;
   float clouds_level;
   Texture clouds_texture;
};

#endif
