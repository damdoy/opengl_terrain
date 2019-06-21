#ifndef TREES_MANAGER_H
#define TREES_MANAGER_H

#include "../drawable.h"
#include "trunk.h"
#include "../_terrain_quad/qterrain.h"
#include "trees_sub_manager.h"
#include "individual_leaves.h"

#define TREES_DRAW_DISTANCE 3
#define NB_TREES_SIDE 4

class Trees_manager : public Drawable{
public:

   void init(QTerrain *terrain, float water_height){

      this->terrain = terrain;
      this->water_height = water_height;

      /////// load a texture for the bark (before generating the texture)
      // texture_bark.load_image("bark_texture.png");
      //
      // glGenTextures(1, &_tex_bark);
      // glBindTexture(GL_TEXTURE_2D, _tex_bark);
      // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture_bark.get_width(), texture_bark.get_height(), 0, GL_RGB, GL_FLOAT, texture_bark.get_tex_data());
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      // glGenerateMipmap(GL_TEXTURE_2D);


      noise_gen_wind.set_noise_function(NOISE_SELECT_EASE);
      noise_gen_wind.set_noise_level(3);

      std::vector<std::vector<float> > noise_val = noise_gen_wind.get_2D_noise(512, 512, -1.0f, 1.0f, -1.0f, 1.0f);
      tex_wind_noise.set_data(noise_val);

      glGenTextures(1, &_tex_wind_noise);
      glBindTexture(GL_TEXTURE_2D, _tex_wind_noise);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex_wind_noise.get_width(), tex_wind_noise.get_height(), 0, GL_RED, GL_FLOAT, tex_wind_noise.get_tex_data());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT); //can be clamp to edge, clamp to border or gl repeat
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

      _pid_trunk = load_shaders_files("trunk_vshader.glsl", "trunk_fshader.glsl");
      _pid_indiv_leaves = load_shaders_files("leaves_individual_vshader.glsl", "leaves_individual_fshader.glsl");

      sub_mgr_size = 5;

      //init the trees for all the sub managers
      sub_managers.resize(sub_mgr_size*2+1);
      int i_abs = 0;
      for (int i = -sub_mgr_size; i <= (int)sub_mgr_size; i++) {
         for (int j = -sub_mgr_size; j <= (int)sub_mgr_size; j++) {
            Trees_sub_manager sub_manager;
            sub_manager.init(terrain, water_height, _pid_trunk, _pid_indiv_leaves, NB_TREES_SIDE, 50*i, 50*(i+1), 50*j, 50*(j+1));
            sub_manager.load();

            sub_managers[i_abs].push_back(sub_manager);
         }
         i_abs++;
      }

      wind_dir[0] = wind_dir[1] = 0.0f;
   }

   virtual void draw(){

      GLuint tex_id = 0;

      wind_dir[0] += 0.002f;
      if(wind_dir[0] > 10.0f){
         wind_dir[0] = 0;
      }

      wind_dir[1] += 0.001f;
      if(wind_dir[1] > 10.0f){
         wind_dir[1] = 0;
      }

      //activate/deactivate sub_mgr depending on cam pos
      update_sub_managers();

      glUseProgram(_pid_trunk);

      glUniform3fv( glGetUniformLocation(_pid_trunk, "light_position"), 1, this->light_position);
      glUniform3fv( glGetUniformLocation(_pid_trunk, "camera_position"), 1, this->camera_position);
      glUniform4fv( glGetUniformLocation(_pid_trunk, "clip_coord"), 1, this->clip_coord);

      glUniform1ui( glGetUniformLocation(_pid_trunk, "shadow_mapping_effect"), this->shadow_mapping_effect);
      glUniform1ui( glGetUniformLocation(_pid_trunk, "window_width"), this->window_width);
      glUniform1ui( glGetUniformLocation(_pid_trunk, "window_height"), this->window_height);

      glUniform3fv( glGetUniformLocation(_pid_trunk, "sun_dir"), 1, this->sun_dir);
      glUniform3fv( glGetUniformLocation(_pid_trunk, "sun_col"), 1, this->sun_col);

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex_bark);
      tex_id = glGetUniformLocation(_pid, "tex");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

      for (size_t i = 0; i < sub_managers.size(); i++) {
         for (size_t j = 0; j < sub_managers[i].size(); j++) {
            sub_managers[i][j].draw_trunks();
         }
      }

      glUseProgram(0);

      glUseProgram(_pid_indiv_leaves);

      glUniform3fv( glGetUniformLocation(_pid_indiv_leaves, "light_position"), 1, this->light_position);
      glUniform3fv( glGetUniformLocation(_pid_indiv_leaves, "camera_position"), 1, this->camera_position);
      glUniform4fv( glGetUniformLocation(_pid_indiv_leaves, "clip_coord"), 1, this->clip_coord);

      glUniform1ui( glGetUniformLocation(_pid_indiv_leaves, "shadow_mapping_effect"), this->shadow_mapping_effect);
      glUniform1ui( glGetUniformLocation(_pid_indiv_leaves, "window_width"), this->window_width);
      glUniform1ui( glGetUniformLocation(_pid_indiv_leaves, "window_height"), this->window_height);

      glUniform3fv( glGetUniformLocation(_pid_indiv_leaves, "sun_dir"), 1, this->sun_dir);
      glUniform3fv( glGetUniformLocation(_pid_indiv_leaves, "sun_col"), 1, this->sun_col);

      glUniform2fv( glGetUniformLocation(_pid_indiv_leaves, "wind_dir"), 1, this->wind_dir);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, _tex_wind_noise);
      tex_id = glGetUniformLocation(_pid_indiv_leaves, "tex_wind");
      glUniform1i(tex_id, 1 /*GL_TEXTURE1*/);

      glDisable(GL_CULL_FACE);

      for (size_t i = 0; i < sub_managers.size(); i++) {
         for (size_t j = 0; j < sub_managers[i].size(); j++) {
            sub_managers[i][j].draw_ileaves();
         }
      }

      glEnable(GL_CULL_FACE);

      glUseProgram(0);
   }

   virtual void set_view_matrix(glm::mat4x4 view){
      Drawable::set_view_matrix(view);
      for (size_t i = 0; i < sub_managers.size(); i++) {
         for (size_t j = 0; j < sub_managers[i].size(); j++) {
            sub_managers[i][j].set_view_matrix(view);
         }
      }
   }

   virtual void set_projection_matrix(glm::mat4x4 projection){
      Drawable::set_projection_matrix(projection);
      for (size_t i = 0; i < sub_managers.size(); i++) {
         for (size_t j = 0; j < sub_managers[i].size(); j++) {
            sub_managers[i][j].set_projection_matrix(projection);
         }
      }
   }

protected:
   GLuint _pid_trunk;
   GLuint _pid_indiv_leaves;
   GLuint _tex_bark;

   uint sub_mgr_size;

   GLuint _tex_wind_noise;
   Noise_generator noise_gen_wind;
   Texture tex_wind_noise;

   float wind_dir[2];

   std::vector<std::vector<Trees_sub_manager> > sub_managers;

   QTerrain *terrain;
   float water_height;
   Texture texture_bark;

   Trunk trunk;

   void update_sub_managers(){

      //default values
      size_t i_cam = 10000000;
      size_t j_cam = 10000000;

      for (size_t i = 0; i < sub_mgr_size*2; i++) {
         for (size_t j = 0; j < sub_mgr_size*2; j++) {

            float start_x = sub_managers[i][j].get_start_x();
            float end_x = sub_managers[i][j].get_end_x();
            float start_y = sub_managers[i][j].get_start_y();
            float end_y = sub_managers[i][j].get_end_y();

            float cam_x = this->camera_position[0];
            float cam_y = this->camera_position[2];

            if(cam_x > start_x && cam_x < end_x && cam_y > start_y && cam_y < end_y){
               i_cam = i;
               j_cam = j;
            }
         }
      }

      for (size_t i = 0; i < sub_mgr_size*2+1; i++) {
         for (size_t j = 0; j < sub_mgr_size*2+1; j++) {

            int dist_to_cam = abs(i_cam-i)+abs(j_cam-j);

            //similar algo to the grass manager
            float point_dir_start_x = sub_managers[i][j].get_start_x()-this->camera_position[0];
            float point_dir_end_x = sub_managers[i][j].get_end_x()-this->camera_position[0];
            float point_dir_start_y = sub_managers[i][j].get_start_y()-this->camera_position[2];
            float point_dir_end_y = sub_managers[i][j].get_end_y()-this->camera_position[2];

            float cam_dir_x = this->camera_direction[0];
            float cam_dir_y = this->camera_direction[2];

            //calculate dot products with cam direction for all 4 corners of sub grass
            float dot_top_left = point_dir_start_x*cam_dir_x+point_dir_start_y*cam_dir_y;
            float dot_top_right = point_dir_end_x*cam_dir_x+point_dir_start_y*cam_dir_y;
            float dot_bot_left = point_dir_start_x*cam_dir_x+point_dir_end_y*cam_dir_y;
            float dot_bot_right = point_dir_end_x*cam_dir_x+point_dir_end_y*cam_dir_y;

            //in view = in the front of the camera (this is really permissive but still removes 50% sub terrains)
            bool is_sub_tree_in_view = (dot_top_left > 0 || dot_top_right > 0 || dot_bot_left > 0 || dot_bot_right > 0);

            if(dist_to_cam <= TREES_DRAW_DISTANCE && is_sub_tree_in_view){
               sub_managers[i][j].set_enabled(true);
            }
            else{
               sub_managers[i][j].set_enabled(false);
            }
         }
      }

   }
};

#endif
