#version 330

// Input vertex data, different for all executions of this shader.
in vec3 text_coord;
in vec3 world_normal;
in vec3 world_position;

//Uniform properties
uniform samplerCube texture_cubemap;
uniform vec3 camera_position;

// Output data
layout(location = 0) out vec4 out_color;

// functia pentru a calcula reflexia pe plan orizontal
vec3 myReflect()
{
vec3 toCamera = camera_position - world_position;
vec3 normalizedToCamera = normalize(toCamera);
vec3 normalizedNormal = normalize(world_normal);

vec3 reflectionDir = reflect(normalizedToCamera, normalizedNormal);

reflectionDir.y = -reflectionDir.y;

return texture(texture_cubemap, reflectionDir).xyz;
}

void main()
{
vec3 reflectedColor = myReflect();

out_color = vec4(reflectedColor, 1.0);
}