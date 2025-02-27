#version 330 core

in vec3 frag_normal;
in vec3 frag_position;

out vec4 frag_color;

void main()
{
vec3 light_dir = normalize(vec3(0.0, 1.0, 1.0);
vec3 normal = normalize(frag_normal);
float diff = max(dot(normal, light_dir), 0.0);

float height = frag_position.y;
vec3 color = mix(vec3(0.1, 0.6, 0.2), vec3(0.8, 0.8, 0.5), clamp((height + 2.5) / 7.5, 0.0, 1.0));

vec3 final_color = color * diff + vec3(0.1, 0.1, 0.1);
frag_color = vec4(final_color, 1.0);
}