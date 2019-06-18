#version 330 core

in vec3 position;
in vec3 vertex_normal;
uniform vec3 light_position;
uniform vec3 camera_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec4 clip_coord;

uniform mat4 shadow_matrix; //bias*P*V

out vec3 frag_position;
out vec3 frag_surface_normal_color;
out vec3 frag_normal_transformed;

//to draw the height, without having it modified by the model matrix
out float frag_nontransfheight;
out vec4 shadow_coord;

void main(){
   gl_Position = projection*view*model*vec4(position, 1.0);
   gl_ClipDistance[0] = dot(model*vec4(position, 1.0), clip_coord);

   //matrix to adapts the normals to the application of matrix on the terrain
   //otherwise the terrain is bad
   mat3 normalMat = mat3(model);
   normalMat = transpose(inverse(normalMat));

   vec3 normal_transformed = vec3(0.0);
   normal_transformed = normalize(normalMat*vertex_normal);

   shadow_coord = shadow_matrix * model*vec4(position, 1.0);

   frag_normal_transformed = normal_transformed;
   frag_position = vec3(model*vec4(position, 1.0));

   frag_nontransfheight = position[1];
}
