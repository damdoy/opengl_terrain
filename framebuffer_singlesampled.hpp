#pragma once

#include "framebuffer.h"

#define USING_MULTISAMPLE 1

class Framebuffer_singlesampled : public Framebuffer{
public:
   Framebuffer_singlesampled(){

   }

   void bind() {
      glViewport(0, 0, _width, _height);
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo_single);
   }

   virtual void unbind(){
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
   }

   virtual int init(int image_width, int image_height, bool use_interpolation = false){
      this->_width = image_width;
      this->_height = image_height;

      glGenFramebuffers(1, &_fbo_single);
      glBindFramebuffer(GL_FRAMEBUFFER, _fbo_single);

      glGenTextures(1, &_color_tex_single);
      glBindTexture(GL_TEXTURE_2D, _color_tex_single);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

      if(use_interpolation){
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      } else {
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
         glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      }

      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _width, _height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL); ///< how to load from buffer

      glBindTexture(GL_TEXTURE_2D, 0);

      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 /*location = 0*/, GL_TEXTURE_2D, _color_tex_single, 0 /*level*/);

      glGenRenderbuffers(1, &_depth_rb_single);
      glBindRenderbuffer(GL_RENDERBUFFER, _depth_rb_single);

      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depth_rb_single);

      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, _width, _height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
         std::cerr << "!!!ERROR: Framebuffer not OK :(" << std::endl;
      glBindFramebuffer(GL_FRAMEBUFFER, 0); ///< avoid pollution

      return _color_tex_single;
   }

   virtual void cleanup(){
      glDeleteTextures(1, &_color_tex_single);
      glDeleteRenderbuffers(1, &_depth_rb_single);
      glBindFramebuffer(GL_FRAMEBUFFER, 0 /*UNBIND*/);
      glDeleteFramebuffers(1, &_fbo_single);
   }

   GLuint get_texture(){
      return _color_tex_single;
   }

protected:
   bool _init;
   int _width;
   int _height;
   GLuint _fbo_single;
   GLuint _depth_rb_single;
   GLuint _color_tex_single;
   GLuint _depth_tex_single;
};
