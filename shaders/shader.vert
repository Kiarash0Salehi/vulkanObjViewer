#version 460 core

layout(location = 0) out vec3 out_color;
layout(location = 1) out vec3 out_normal;

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;
layout(location = 2) in vec3 normal;

vec3 colors[3] = vec3[3](vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), vec3(0.0, 0.0, 1.0));

layout(push_constant) uniform PushConstants
{
    mat4 ViewProjection;
    mat4 Transform;
} u_PushConstants;


void main() 
{
    vec3 color = vec3(1.0, 1.0, 1.0);
    gl_Position = u_PushConstants.ViewProjection * u_PushConstants.Transform * vec4(position, 1.0);

    vec3 lightDir = vec3(1.0, 1.0, 1.0); // Light direction as a vector
    vec3 AMBIENT = vec3(0.1, 0.1, 0.1); // Ambient light as a color vector
    vec3 Normal = normalize(transpose(inverse(mat3(u_PushConstants.Transform))) * normal); // Normalize the normal vector

    // Normalize the light direction
    vec3 normalizedLightDir = normalize(lightDir);

    // Calculate diffuse lighting using Lambertian reflectance
    float diffuse = max(dot(Normal, normalizedLightDir), 0.0);

    // Add specular highlights (Phong reflection model)
    vec3 viewDir = normalize(vec3(0, 0, 1)); // Assume the viewer is along the Z-axis
    vec3 reflectDir = reflect(-normalizedLightDir, Normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0); // Shininess factor
    vec3 specular = vec3(1.0) * spec; // White specular highlight

    // Combine the base color with ambient, diffuse, and specular lighting
    vec3 shadedColor = AMBIENT + color * diffuse + specular;

    // Clamp the final color to avoid exceeding 1.0
    shadedColor = clamp(shadedColor, 0.0, 1.0);

    out_normal = normalize(transpose(inverse(mat3(u_PushConstants.Transform))) * normal);
    out_color = shadedColor;
}