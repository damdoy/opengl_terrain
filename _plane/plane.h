#ifndef PLANE_H
#define PLANE_H

#include <GL/glew.h>
#include <GL/glfw.h>
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "../shader_helper.h"
#include "../texture.h"
#include "../drawable.h"

class Plane : public Drawable{
public:

   void init(){
      GLuint pid = load_shaders_files("plane_vshader.glsl", "plane_fshader.glsl");

      init(pid);
   }

   void init(GLuint pid){

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

   void set_texture(const Texture *tex){
      if(_tex != 0){
         //delete current texture
         glDeleteTextures(1, &_tex);
         _tex = 0;
      }

      //texture data definition
      glGenTextures(1, &_tex);
      glBindTexture(GL_TEXTURE_2D, _tex);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, tex->get_width(), tex->get_height(), 0, GL_RED, GL_FLOAT, tex->get_tex_data());
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      GLuint tex_id = glGetUniformLocation(_pid, "tex");
      glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);

   }

   void set_texture(GLuint tex){
      _tex = tex;
   }

   void set_texture1(GLuint tex){
      _tex1 = tex;
   }

   void draw_without_pid(){
      glBindVertexArray(_vao);
      glUniformMatrix4fv( glGetUniformLocation(_pid, "view"), 1, GL_FALSE, glm::value_ptr(this->view_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "projection"), 1, GL_FALSE, glm::value_ptr(this->projection_matrix));
      glUniformMatrix4fv( glGetUniformLocation(_pid, "model"), 1, GL_FALSE, glm::value_ptr(this->model_matrix));

      if(enabled){
         glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      }

      glBindVertexArray(0);
   }

   void draw_instanced(uint nb_instances){
      glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, nb_instances);
   }

   void draw(){
      glUseProgram(_pid);

      draw_without_pid();

      glUseProgram(0);
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
};

#endif
