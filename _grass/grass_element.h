#ifndef GRASS_ELEMENT_H
#define GRASS_ELEMENT_H

#include "../_plane/plane.h"
#include "../drawable.h"
#include "../transform.h"


//represent a signle grass blade, useful to get transf matrices for grass
class Grass_element : public Drawable{

public:

   Grass_element(){
      all_planes[0] = &plane_0_front;
      all_planes[1] = &plane_0_back;
      all_planes[2] = &plane_1_front;
      all_planes[3] = &plane_1_back;

      all_transf[0] = &transf_0_front;
      all_transf[1] = &transf_0_back;
      all_transf[2] = &transf_1_front;
      all_transf[3] = &transf_1_back;

      wind_dir[0] = 2.0f;
      wind_dir[1] = 2.0f;
   }

   ~Grass_element(){

   }

   void init(){
      GLuint pid = load_shaders_files("grass_vshader.glsl", "grass_fshader.glsl");

      init(pid);
   }

   float *get_wind_dir(){
      return wind_dir;
   }

   void set_wind_dir(float wind_dir[2]){
      this->wind_dir[0] = wind_dir[0];
      this->wind_dir[1] = wind_dir[1];
   }

   void init_matrices(){
      transf_0_front.rotate(0.0f, 1.0f, 0.0f, 3.1415f/4.0f);
      transf_0_front.rotate(1.0f, 0.0f, 0.0f, 3.1415f/2.0f);
      transf_0_back.rotate(0.0f, 1.0f, 0.0f, 5*3.1415f/4.0f);
      transf_0_back.rotate(1.0f, 0.0f, 0.0f, 3.1415f/2.0f);

      transf_1_front.rotate(0.0f, 1.0f, 0.0f, 3*3.1415f/4.0f);
      transf_1_front.rotate(1.0f, 0.0f, 0.0f, 3.1415f/2.0f);
      transf_1_back.rotate(0.0f, 1.0f, 0.0f, 7*3.1415f/4.0f);
      transf_1_back.rotate(1.0f, 0.0f, 0.0f, 3.1415f/2.0f);
   }

   void init(GLuint pid){
      this->_pid = pid;
      for (size_t i = 0; i < nb_planes; i++) {
         all_planes[i]->init(pid);
      }
      this->init_matrices();
   }

   void init(GLuint pid, GLuint vao){
      this->_pid = pid;
      for (size_t i = 0; i < nb_planes; i++) {
         all_planes[i]->init(pid, vao);
      }
      this->init_matrices();
   }

   virtual void set_enabled(bool enabled){
      this->enabled = enabled;
      for (size_t i = 0; i < nb_planes; i++) {
         all_planes[i]->set_enabled(enabled);
      }
   }

   virtual void set_model_matrix(glm::mat4x4 model){

      for (size_t i = 0; i < nb_planes; i++) {
         all_planes[i]->set_model_matrix(model*all_transf[i]->get_matrix());
      }

      Drawable::set_model_matrix(model);
   }

   virtual void set_view_matrix(glm::mat4x4 view){
      for (size_t i = 0; i < nb_planes; i++) {
         all_planes[i]->set_view_matrix(view);
      }

      Drawable::set_view_matrix(view);
   }

   virtual void set_projection_matrix(glm::mat4x4 projection){
      for (size_t i = 0; i < nb_planes; i++) {
         all_planes[i]->set_projection_matrix(projection);
      }

      Drawable::set_projection_matrix(projection);
   }

   glm::mat4x4 get_matrix_grass_0(){
      return this->model_matrix*transf_0_front.get_matrix();
   }

   glm::mat4x4 get_matrix_grass_1(){
      return this->model_matrix*transf_1_front.get_matrix();

   }

   void draw(){

      for (size_t i = 0; i < nb_planes; i++) {
         all_planes[i]->draw_without_pid();
      }
   }

protected:
   GLuint _pid;

   Plane plane_0_front;
   Plane plane_0_back;
   Plane plane_1_front;
   Plane plane_1_back;
   Plane *all_planes[4];
   const uint nb_planes = 4;

   float wind_dir[2];

   Transform transf_0_front;
   Transform transf_0_back;
   Transform transf_1_front;
   Transform transf_1_back;
   Transform *all_transf[4];

};

#endif
