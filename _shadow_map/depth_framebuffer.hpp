#ifndef DEPTH_FRAMEBUFFER_HPP
#define DEPTH_FRAMEBUFFER_HPP

#include <cmath>
#include <vector>

#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader_helper.h"
#include "../drawable.h"

#include "../camera.h"

class Depth_framebuffer{

public:
   void init(unsigned tex_width, unsigned tex_height){

      camera = NULL;

      this->tex_width = tex_width;
      this->tex_height = tex_height;

      _pid_depth = load_shaders_files("depth_framebuffer_vshader.glsl", "depth_framebuffer_fshader.glsl");

      // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
      glGenFramebuffers(1, &depth_fbo);
      glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);

      // Depth texture. Slower than a depth buffer, but you can sample it later in your shader
      glGenTextures(1, &depth_texture_id);
      glBindTexture(GL_TEXTURE_2D, depth_texture_id);
      glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT32, tex_width, tex_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      center_to_vals[0] = center_to_vals[1] = center_to_vals[2] = 0;

      glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture_id, 0);
      //glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, depth_texture_id, 0);

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      std::cerr << "!!!ERROR: depth Framebuffer not OK :(" << std::endl;

      // Disable color rendering as there are no color attachments
      glDrawBuffer(GL_NONE);

      glBindTexture(GL_TEXTURE_2D, 0);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

   void set_light_pos(GLfloat light_position[3]){
      this->light_position[0] = light_position[0];
      this->light_position[1] = light_position[1];
      this->light_position[2] = light_position[2];
   }

   void set_camera(Camera *cam){
      this->camera = cam;
   }

   glm::mat4x4 get_depth_view_mat(){
      if(camera == NULL){
         glm::vec3 eye(light_position[0]+center_to_vals[0], light_position[1], light_position[2]+center_to_vals[2]);
         glm::vec3 center(center_to_vals[0], center_to_vals[1], center_to_vals[2]);
         glm::vec3 up(0.0f, 1.0f, 0.0f);

         return glm::lookAt(eye, center, up);
      }
      else{
         return this->camera->getMatrix();
      }
   }

   glm::mat4x4 get_depth_perspective_mat(){
      return get_perspective_mat();
   }

   glm::mat4x4 get_shadow_mat(){
      glm::mat4x4 bias_matrix(
         0.5, 0.0, 0.0, 0.0,
         0.0, 0.5, 0.0, 0.0,
         0.0, 0.0, 0.5, 0.0,
         0.5, 0.5, 0.5, 1.0
      );

      return bias_matrix*get_depth_perspective_mat()*get_depth_view_mat();
   }

   void draw_fb(std::vector<Drawable*> *lst_drawable){

      glm::mat4x4 view_mat = get_depth_view_mat();

      glm::mat4x4 projection_mat = get_perspective_mat();

      // Clear
      glClearDepth(1.0f);


      // Bind the "depth only" FBO and set the viewport to the size
      // of the depth texture
      glBindFramebuffer(GL_FRAMEBUFFER, depth_fbo);
      glViewport(0, 0, tex_width, tex_height);

      glClear(GL_DEPTH_BUFFER_BIT);

      // Enable polygon offset to resolve depth-fighting isuses
      glEnable(GL_POLYGON_OFFSET_FILL);
      glPolygonOffset(2.0f, 4.0f);

      // Draw from the light’s point of view
      for (size_t i = 0; i < lst_drawable->size(); i++) {
         //update the view and projection matrices
         lst_drawable->at(i)->set_view_matrix(view_mat);
         lst_drawable->at(i)->set_projection_matrix(projection_mat);

         GLuint shader_before = lst_drawable->at(i)->get_shader();
         lst_drawable->at(i)->set_shader(_pid_depth);

         lst_drawable->at(i)->draw();

         lst_drawable->at(i)->set_shader(shader_before);
      }

      glDisable(GL_POLYGON_OFFSET_FILL);

      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

   GLuint get_texture_id(){
      return depth_texture_id;
   }

   void center_to(float x, float y, float z){
      center_to_vals[0] = x;
      center_to_vals[1] = y;
      center_to_vals[2] = z;
   }

protected:
   GLuint depth_texture_id;
   GLuint _pid_depth;
   unsigned int tex_width;
   unsigned int tex_height;
   GLuint depth_fbo;
   GLfloat light_position[3];
   GLfloat center_to_vals[3];
   glm::mat4x4 projection_matrix;
   glm::mat4x4 view_matrix;
   Camera *camera;

   glm::mat4x4 get_perspective_mat(){
      //FOR SHADOW MAPPING
      if(camera == NULL){
         //orthograhic projection is better for sun type lights
         return glm::ortho(-150.0f, 150.0f, -150.0f, 150.0f, 1.0f, 2048.0f);
      }

      //same perspective as camera
      // return glm::perspective(3.1415f/2.0f, (float)tex_width/(float)tex_height, 0.1f, 2048.0f);
      else{
         return camera->get_perspective_mat();
      }

   }

};

#endif
