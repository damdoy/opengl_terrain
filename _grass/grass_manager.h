#ifndef GRASS_MANAGER_H
#define GRASS_MANAGER_H

#include <vector>
#include <cmath>

#include "grass_sub_manager.h"
#include "grass_sub_manager_blades.h"
#include "../drawable.h"
#include "grass_element.h"
#include "../_terrain_quad/qterrain.h"
#include "../texture.h"

#define SIDE_GRASS 150
#define NB_GRASS (SIDE_GRASS*SIDE_GRASS)
#define GRASS_DRAW_DISTANCE 3

class Grass_manager : public Drawable{

public:
   Grass_manager(){

   }

   ~Grass_manager(){

   }

   void init(QTerrain *terrain, float water_height){

      srand(time(0));

      //really simple noise to have wind (wind is represented as a position shift in grass)
      noise_gen_wind.set_noise_function(NOISE_SELECT_EASE);
      noise_gen_wind.set_noise_level(1);

      std::vector<std::vector<float> > noise_val = noise_gen_wind.get_2D_noise(512, 512, -1.0f, 1.0f, -1.0f, 1.0f);
      tex_wind_noise.set_data(noise_val);

      GLuint pid_grass = load_shaders_files("grass_blades_vshader.glsl", "grass_blades_fshader.glsl");
      _pid = pid_grass;

      this->terrain = terrain;

      sub_mgr_size = 3;

      //init les submanagers
      for (int i = -sub_mgr_size; i < (int)sub_mgr_size; i++) {
         for (int j = -sub_mgr_size; j < (int)sub_mgr_size; j++) {
            Grass_sub_manager_blades sub_manager;
            sub_manager.init(terrain, water_height, pid_grass, nb_grass_elem_side, 100*i, 100*(i+1), 100*j, 100*(j+1));
            sub_manager.load();


            sub_managers.push_back(sub_manager);
         }
      }

      //texture data definition (wind)
      glGenTextures(1, &_tex_wind_noise);
      glBindTexture(GL_TEXTURE_2D, _tex_wind_noise);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex_wind_noise.get_width(), tex_wind_noise.get_height(), 0, GL_RED, GL_FLOAT, tex_wind_noise.get_tex_data());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); //can be clamp to edge, clamp to border or gl repeat
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
      GLuint tex_id = glGetUniformLocation(_pid, "tex_wind");
      glUniform1i(tex_id, 1 /*GL_TEXTURE1*/);

      wind_dir[0] = wind_dir[1] = 0.0f;
   }

   virtual void set_enabled(bool enabled){
      this->enabled = enabled;

      for (size_t i = 0; i < sub_managers.size(); i++) {
         sub_managers[i].set_enabled(enabled);
      }
   }

   virtual void set_view_matrix(glm::mat4x4 view){
      this->view_matrix = view;
   }

   virtual void set_projection_matrix(glm::mat4x4 projection){
      this->projection_matrix = projection;
   }

   void set_terrain(QTerrain *terrain){
      this->terrain = terrain;
   }

   void draw(){

      update_sub_managers();

      wind_dir[0] += 0.0005f;
      if(wind_dir[0] > 10.0f){
         wind_dir[0] = 0;
      }

      wind_dir[1] += 0.00025f;
      if(wind_dir[1] > 10.0f){
         wind_dir[1] = 0;
      }

      glUseProgram(_pid);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, _tex_wind_noise);
      GLuint tex_id = glGetUniformLocation(_pid, "tex_wind");
      glUniform1i(tex_id, 1 /*GL_TEXTURE1*/);

      glUniform2fv( glGetUniformLocation(_pid, "wind_dir"), 1, this->wind_dir);

      //call these uniform one time for all sub managers
      glUniform3fv( glGetUniformLocation(_pid, "light_position"), 1, this->light_position);
      glUniform3fv( glGetUniformLocation(_pid, "sun_dir"), 1, this->sun_dir);
      glUniform3fv( glGetUniformLocation(_pid, "sun_col"), 1, this->sun_col);
      glUniform3fv( glGetUniformLocation(_pid, "camera_position"), 1, this->camera_position);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));


      if(has_shadow_buffer){
         glUniformMatrix4fv( glGetUniformLocation(_pid, "shadow_matrix"), 1, GL_FALSE, glm::value_ptr(this->shadow_matrix));

         glUniform1i( glGetUniformLocation(_pid, "shadow_buffer_texture_width"), shadow_buffer_texture_width);
         glUniform1i( glGetUniformLocation(_pid, "shadow_buffer_texture_height"), shadow_buffer_texture_height);

         glActiveTexture(GL_TEXTURE4);
         glBindTexture(GL_TEXTURE_2D, _shadow_texture_id);
         GLuint tex_id = glGetUniformLocation(_pid, "shadow_buffer_tex");
         glUniform1i(tex_id, 4 /*GL_TEXTURE4*/);
      }

      glDisable(GL_CULL_FACE);

      if(enabled){
         for (size_t i = 0; i < sub_managers.size(); i++) {
            sub_managers[i].draw();
         }
      }

      glEnable(GL_CULL_FACE);

      glUseProgram(0);
   }

protected:

   Noise_generator noise_gen_wind;
   QTerrain *terrain;
   Texture tex_wind_noise;
   Texture texture_grass;

   float wind_dir[2];

   const int nb_grass_elem_side = SIDE_GRASS;
   uint sub_mgr_size;
   std::vector<Grass_sub_manager_blades> sub_managers;

   std::vector<glm::mat4x4> mat_vector;

   GLuint _pid;
   GLuint _vao_grass;
   GLuint _tex_wind_noise;

   void update_sub_managers(){

      //default values
      size_t i_cam = 10000000;
      size_t j_cam = 10000000;

      for (size_t i = 0; i < sub_mgr_size*2; i++) {
         for (size_t j = 0; j < sub_mgr_size*2; j++) {

            int curr_pos = i*(sub_mgr_size*2)+j;

            float start_x = sub_managers[curr_pos].get_start_x();
            float end_x = sub_managers[curr_pos].get_end_x();
            float start_y = sub_managers[curr_pos].get_start_y();
            float end_y = sub_managers[curr_pos].get_end_y();

            float cam_x = this->camera_position[0];
            float cam_y = this->camera_position[2];

            if(cam_x > start_x && cam_x < end_x && cam_y > start_y && cam_y < end_y){
               i_cam = i;
               j_cam = j;
            }
         }
      }

      for (size_t i = 0; i < sub_mgr_size*2; i++) {
         for (size_t j = 0; j < sub_mgr_size*2; j++) {

            int curr_pos = i*(sub_mgr_size*2)+j;

            //limit draw by distance
            int dist_to_cam = abs(i_cam-i)+abs(j_cam-j);

            float point_dir_start_x = sub_managers[curr_pos].get_start_x()-this->camera_position[0];
            float point_dir_end_x = sub_managers[curr_pos].get_end_x()-this->camera_position[0];
            float point_dir_start_y = sub_managers[curr_pos].get_start_y()-this->camera_position[2];
            float point_dir_end_y = sub_managers[curr_pos].get_end_y()-this->camera_position[2];

            float cam_dir_x = this->camera_direction[0];
            float cam_dir_y = this->camera_direction[2];

            //calculate dot products with cam direction for all 4 corners of sub grass
            float dot_top_left = point_dir_start_x*cam_dir_x+point_dir_start_y*cam_dir_y;
            float dot_top_right = point_dir_end_x*cam_dir_x+point_dir_start_y*cam_dir_y;
            float dot_bot_left = point_dir_start_x*cam_dir_x+point_dir_end_y*cam_dir_y;
            float dot_bot_right = point_dir_end_x*cam_dir_x+point_dir_end_y*cam_dir_y;

            //in view = in the front of the camera (this is really permissive but still removes 50% sub terrains)
            bool is_sub_grass_in_view = (dot_top_left > 0 || dot_top_right > 0 || dot_bot_left > 0 || dot_bot_right > 0);

            // if(dist_to_cam <= GRASS_DRAW_DISTANCE && i < sub_mgr_size && j < sub_mgr_size){
            if(dist_to_cam <= GRASS_DRAW_DISTANCE && is_sub_grass_in_view){
               sub_managers[curr_pos].set_enabled(true);
            }
            else{
               sub_managers[curr_pos].set_enabled(false);
            }
         }
      }

   }
};

#endif
