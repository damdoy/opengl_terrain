#version 330 core

in vec3 position;

in mat4 model_mat;
// uniform mat4 model;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 light_position;
uniform vec4 clip_coord;
uniform vec3 sun_dir;
uniform vec3 sun_col;
in vec3 normals;
in vec2 uv;

uniform mat4 shadow_matrix; //bias*P*V

out vec3 frag_position;
out vec3 frag_normal;
out float red;
out float green;
out float blue;
out vec2 tex_coord;

out vec4 shadow_coord;
out float gl_ClipDistance[1];

float get_diffuse_strength(vec3 light_dir, vec3 normal){
   return clamp(dot(normal, light_dir), 0.0, 1.0);
}

void main(){

   mat4 model = model_mat;

   gl_Position = projection*view*model*vec4(position, 1.0);
   gl_ClipDistance[0] = dot(model*vec4(position, 1.0), clip_coord);
   frag_position = vec3(model*vec4(position, 1.0));

   mat3 normalMat = mat3(model);
   normalMat = transpose(inverse(normalMat));

   vec3 light_dir = normalize(light_position-vec3(model*vec4(position, 1.0)));

   vec3 normal_transformed = vec3(0.0);
   normal_transformed = normalize(normalMat*normals);

   float diffuse_light = 0.0;
   diffuse_light = dot(normal_transformed, light_dir);
   float diffuse_sun = 0.0;
   diffuse_sun = get_diffuse_strength(-sun_dir, normal_transformed);


   tex_coord = uv;

   shadow_coord = shadow_matrix * model*vec4(position, 1.0);
   frag_normal = normal_transformed;
}
