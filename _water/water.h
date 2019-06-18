#ifndef WATER_H
#define WATER_H

#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader_helper.h"
#include "../texture.h"
#include "../drawable.h"

class Water : public Drawable{
public:

   void init(){
      GLuint pid = load_shaders_files("water_vshader.glsl", "water_fshader.glsl");

      init(pid);
   }

   void init(GLuint pid){

      this->_effect = 0;

      this->_pid = pid;
      if(_pid == 0) exit(-1);

      glGenVertexArrays(1, &_vao);
      glBindVertexArray(_vao);

      GLfloat vpoint[] = {
         -1.0f, 0.0f, 1.0f, // 0 bottom left
         1.0f, 0.0f, 1.0f, // 1 bottom right
         1.0f, 0.0f, -1.0f, // 2 top right
         -1.0f,  0.0f, -1.0f, // 3 top left
      };

      GLuint vpoint_index[] = {
         0, 1, 2,
         0, 2, 3
      };

      GLfloat per_surface_normals[] = {0, 1, 0,
         0, 1, 0,
         0, 1, 0,
         0, 1, 0,
      };

      _time = 0.0f;

      glGenBuffers(1, &_vbo);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vpoint), vpoint, GL_STATIC_DRAW);

      GLuint vpoint_id = glGetAttribLocation(_pid, "position");
      glEnableVertexAttribArray(vpoint_id);
      glVertexAttribPointer(vpoint_id, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_sur_norm);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_sur_norm);
      glBufferData(GL_ARRAY_BUFFER, sizeof(per_surface_normals), per_surface_normals, GL_STATIC_DRAW);

      GLuint id_pos = glGetAttribLocation(_pid, "surface_normal");
      glEnableVertexAttribArray(id_pos);
      glVertexAttribPointer(id_pos, 3, GL_FLOAT, GL_FALSE, 0, NULL);

      glGenBuffers(1, &_vbo_idx);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo_idx);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vpoint_index), vpoint_index, GL_STATIC_DRAW);

      //texture coord definition
      const GLfloat vtexcoord[] = { 0.0f, 1.0f,
                                    1.0f, 1.0f,
                                    1.0f, 0.0f,
                                    0.0f, 0.0f};

      glGenBuffers(1, &_vbo_tex);
      glBindBuffer(GL_ARRAY_BUFFER, _vbo_tex);
      glBufferData(GL_ARRAY_BUFFER, sizeof(vtexcoord), vtexcoord, GL_STATIC_DRAW);

      GLuint vtexcoord_id = glGetAttribLocation(_pid, "uv");
      glEnableVertexAttribArray(vtexcoord_id);
      glVertexAttribPointer(vtexcoord_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

      glBindVertexArray(0);
   }

   void init(GLuint pid, GLuint vao){
      this->_pid = pid;
      this->_vao = vao;
   }

   //to move the waves
   void set_time(float time){
      _time = time;
   }

   void set_texture_refraction(GLuint tex){
      _tex_refraction = tex;
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex_refraction);
      GLuint tex_id = glGetUniformLocation(_pid, "tex_refraction");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
      glBindTexture(GL_TEXTURE_2D, 0);
   }

   void set_texture_reflection(GLuint tex){
      _tex_reflection = tex;
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, _tex_reflection);
      GLuint tex_id = glGetUniformLocation(_pid, "tex_reflection");
      glUniform1i(tex_id, 1 /*GL_TEXTURE1*/);
      glBindTexture(GL_TEXTURE_2D, 0);
   }

   void set_texture_refraction_depth(GLuint tex){
      _tex_depth_refraction = tex;
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, _tex_depth_refraction);
      GLuint tex_id = glGetUniformLocation(_pid, "tex_depth_refraction");
      glUniform1i(tex_id, 2 /*GL_TEXTURE2*/);
      glBindTexture(GL_TEXTURE_2D, 0);
   }

   void set_effect(unsigned int effect){
      this->_effect = effect;
   }

   void draw(){
      glUseProgram(_pid);
      glBindVertexArray(_vao);


      glUniform3fv( glGetUniformLocation(_pid, "light_position"), 1, this->light_position);
      glUniform3fv( glGetUniformLocation(_pid, "camera_position"), 1, this->camera_position);
      glUniform3fv( glGetUniformLocation(_pid, "camera_direction"), 1, this->camera_direction);

      glUniform1f( glGetUniformLocation(_pid, "time"), this->_time);

      glUniform1ui( glGetUniformLocation(_pid, "water_effect"), this->_effect);

      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(this->model_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));

      glUniform3fv( glGetUniformLocation(_pid, "sun_dir"), 1, this->sun_dir);
      glUniform3fv( glGetUniformLocation(_pid, "sun_col"), 1, this->sun_col);


      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, _tex_refraction);
      glUniform1i(glGetUniformLocation(_pid, "tex_refraction"), 0 /*GL_TEXTURE0*/);

      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, _tex_reflection);
      glUniform1i(glGetUniformLocation(_pid, "tex_reflection"), 1 /*GL_TEXTURE0*/);

      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, _tex_depth_refraction);
      glUniform1i(glGetUniformLocation(_pid, "tex_depth_refraction"), 2 /*GL_TEXTURE0*/);

      if(enabled){
         glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }
      glBindTexture(GL_TEXTURE_2D, 0);
      glUseProgram(0);
      glBindVertexArray(0);
   }

   void cleanup(){
      glDeleteBuffers(1, &_vbo);
      glDeleteVertexArrays(1, &_vao);
      glDeleteProgram(_pid);
   }

protected:
   GLuint _vao;
   GLuint _vbo;
   GLuint _vbo_idx;
   GLuint _vbo_tex;
   GLuint _vbo_sur_norm;
   GLuint _pid;
   GLuint _tex;
   GLuint _tex1;
   GLuint _tex_refraction;
   GLuint _tex_reflection;
   GLuint _tex_depth_refraction;

   unsigned int _effect;

   float _time;
};

#endif
