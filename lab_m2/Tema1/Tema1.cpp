#include "lab_m2/Tema1/Tema1.h"

#include <vector>
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <stb/stb_image.h>



using namespace std;
using namespace m2;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */
struct Particle
{
    glm::vec4 position;
    glm::vec4 speed;
    glm::vec4 initialPos;
    glm::vec4 initialSpeed;
    float delay;
    float initialDelay;
    float lifetime;
    float initialLifetime;

    Particle() {}

    Particle(const glm::vec4& pos, const glm::vec4& speed)
    {
        SetInitial(pos, speed);
    }

    void SetInitial(const glm::vec4& pos, const glm::vec4& speed,
        float delay = 0, float lifetime = 0)
    {
        position = pos;
        initialPos = pos;

        this->speed = speed;
        initialSpeed = speed;

        this->delay = delay;
        initialDelay = delay;

        this->lifetime = lifetime;
        initialLifetime = lifetime;
    }
};


ParticleEffect<Particle>* particleEffect;

Tema1::Tema1()
{
}


Tema1::~Tema1()
{
}




unsigned int Tema1::UploadCubeMapTexture(const std::string& pos_x, const std::string& pos_y, const std::string& pos_z,
    const std::string& neg_x, const std::string& neg_y, const std::string& neg_z) {
    int width, height, channels;

    unsigned char* data_pos_x = stbi_load(pos_x.c_str(), &width, &height, &channels, 0);
    unsigned char* data_neg_x = stbi_load(neg_x.c_str(), &width, &height, &channels, 0);
    unsigned char* data_pos_y = stbi_load(pos_y.c_str(), &width, &height, &channels, 0);
    unsigned char* data_neg_y = stbi_load(neg_y.c_str(), &width, &height, &channels, 0);
    unsigned char* data_pos_z = stbi_load(pos_z.c_str(), &width, &height, &channels, 0);
    unsigned char* data_neg_z = stbi_load(neg_z.c_str(), &width, &height, &channels, 0);

    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_z);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_z);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    stbi_image_free(data_pos_x);
    stbi_image_free(data_neg_x);
    stbi_image_free(data_pos_y);
    stbi_image_free(data_neg_y);
    stbi_image_free(data_pos_z);
    stbi_image_free(data_neg_z);

    return textureID;
}




float RandomNoise(int seed) {
    seed = (seed << 13) ^ seed;
    return (1.0f - ((seed * (seed * seed * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
}


Mesh* Tema1::GenerateTerrainMesh(int m, int n, float hmax, float r_cascada) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    glm::vec2 center = glm::vec2(m / 2.0f, n / 2.0f);

    // Punctele de control pentru curba Bezier
    std::vector<glm::vec3> bezierControlPoints = {
        glm::vec3(5, 0, 5),
        glm::vec3(10, -5, 10),
        glm::vec3(15, -5, 5),
        glm::vec3(20, 0, 10)
    };

    auto bezierPoint = [&](float t) -> glm::vec3 {
        // Calcularea unui punct pe curba Bezier cubică
        float u = 1.0f - t;
        return u * u * u * bezierControlPoints[0] +
            3 * u * u * t * bezierControlPoints[1] +
            3 * u * t * t * bezierControlPoints[2] +
            t * t * t * bezierControlPoints[3];
        };

    // Generăm vertecșii
    for (int i = 0; i <= m; ++i) {
        for (int j = 0; j <= n; ++j) {
            float x = (float)i;
            float z = (float)j;
            glm::vec2 vXZ = glm::vec2(x, z);

            // Noise pentru variații aleatorii
            int seed = static_cast<int>(x) * 73856093 ^ static_cast<int>(z) * 19349663;
            float noise = RandomNoise(seed) * 0.5f;

            // Înălțimea inițială cu noise
            float d = glm::length(vXZ - center) / r_cascada;
            d = glm::clamp(d, 0.0f, 1.0f);
            float y = (d < 1.0f) ? -hmax * (1 - d * d) : hmax * (1 - d * d);
            y += noise;

            // --- Calcularea curburii Bezier ---
            float closestT = 0.0f;
            float minDistance = FLT_MAX;
            glm::vec3 closestBezierPoint;

            for (float t = 0.0f; t <= 1.0f; t += 0.01f) {
                glm::vec3 pointOnCurve = bezierPoint(t);
                float distance = glm::distance(glm::vec2(pointOnCurve.x, pointOnCurve.z), vXZ);

                if (distance < minDistance) {
                    minDistance = distance;
                    closestT = t;
                    closestBezierPoint = pointOnCurve;
                }
            }

            float scobitura_scalare = 2.0f; // Crește acest factor pentru o scobitură mai adâncă

            // Calculăm distanța și aplicăm scobitura
            if (minDistance < r_cascada) {
                float influence = 1.0f - glm::sin(glm::pi<float>() / 2.0f * glm::clamp(minDistance / r_cascada, 0.0f, 1.0f));
                float y_closest_scaled = closestBezierPoint.y * scobitura_scalare; // Scalăm adâncimea curbei Bezier
                y = glm::mix(y, y_closest_scaled, influence);
            }

            // Culoare bazată pe înălțime
            glm::vec3 color = glm::vec3(0.0f, 0.5f + y / (2.0f * hmax), 0.0f);

            // Adăugăm vertecșii cu poziții, normale și culoare
            vertices.emplace_back(glm::vec3(x, y, z), glm::vec3(0, 1, 0), color);
        }
    }

    // Generăm indecșii pentru triunghiuri
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < n; ++j) {
            int topLeft = i * (n + 1) + j;
            int topRight = topLeft + 1;
            int bottomLeft = (i + 1) * (n + 1) + j;
            int bottomRight = bottomLeft + 1;

            indices.push_back(topLeft);
            indices.push_back(bottomLeft);
            indices.push_back(topRight);

            indices.push_back(topRight);
            indices.push_back(bottomLeft);
            indices.push_back(bottomRight);
        }
    }

    // Calculăm normalele pentru fiecare vertex
    for (int i = 0; i <= m; ++i) {
        for (int j = 0; j <= n; ++j) {
            glm::vec3 normal(0.0f);
            int currentIndex = i * (n + 1) + j;

            // Verificăm vecinii
            if (i > 0 && j > 0) {
                glm::vec3 v1 = vertices[currentIndex].position - vertices[currentIndex - 1].position;
                glm::vec3 v2 = vertices[currentIndex].position - vertices[currentIndex - (n + 1)].position;
                normal += glm::cross(v2, v1);
            }
            if (i < m && j < n) {
                glm::vec3 v1 = vertices[currentIndex + 1].position - vertices[currentIndex].position;
                glm::vec3 v2 = vertices[currentIndex + (n + 1)].position - vertices[currentIndex].position;
                normal += glm::cross(v1, v2);
            }

            normal = glm::normalize(normal);
            vertices[currentIndex].normal = normal;
        }
    }

    // Cream mesh-ul
    Mesh* terrainMesh = new Mesh("terrain");
    terrainMesh->InitFromData(vertices, indices);
    return terrainMesh;
}



void Tema1::ResetParticlesAlongBezier(std::vector<glm::vec3>& bezierControlPoints, unsigned int nrParticles) {
    particleEffect = new ParticleEffect<Particle>();
    particleEffect->Generate(nrParticles, true);

    auto particleSSBO = particleEffect->GetParticleBuffer();
    Particle* data = const_cast<Particle*>(particleSSBO->GetBuffer());

    // Funcție pentru rotația unui punct în jurul axei Y
    auto RotatePointY = [](const glm::vec3& point, float angle) -> glm::vec3 {
        float rad = glm::radians(angle);
        float cosAngle = cos(rad);
        float sinAngle = sin(rad);
        return glm::vec3(
            point.x * cosAngle - point.z * sinAngle,
            point.y,
            point.x * sinAngle + point.z * cosAngle
        );
        };

    // Rotim punctele de control ale curbei Bezier cu 40 de grade la stânga
    for (auto& point : bezierControlPoints) {
        point = RotatePointY(point, -40.0f);
    }

    auto bezierPoint = [&](float t) -> glm::vec3 {
        float u = 1.0f - t;
        return u * u * u * bezierControlPoints[0] +
            3 * u * u * t * bezierControlPoints[1] +
            3 * u * t * t * bezierControlPoints[2] +
            t * t * t * bezierControlPoints[3];
        };

    auto bezierTangent = [&](float t) -> glm::vec3 {
        float u = 1.0f - t;
        glm::vec3 derivative = -3.0f * u * u * bezierControlPoints[0] +
            (3.0f * u * u - 6.0f * u * t) * bezierControlPoints[1] +
            (6.0f * u * t - 3.0f * t * t) * bezierControlPoints[2] +
            3.0f * t * t * bezierControlPoints[3];
        return glm::normalize(derivative);
        };

    // Poziția de start a particulelor (capătul curbei Bezier)
    glm::vec3 startPosition = bezierPoint(0.0f);  // t = 0.0f pentru începutul curbei
    glm::vec3 startTangent = bezierTangent(0.0f); // Tangenta în punctul de start

    // Vector perpendicular pe tangenta curbei (pentru variație laterală)
    glm::vec3 perpendicularDirection = glm::normalize(glm::vec3(-startTangent.z, 0.0f, startTangent.x));

    // Adăugăm un offset pentru a muta particulele mai la stânga și mai în spate
    glm::vec3 positionOffset = glm::vec3(-2.0f, 0.0f, -3.0f); // Offset: -2 pe X (stânga), -3 pe Z (spate)

    for (unsigned int i = 0; i < nrParticles; i++) {
        // Variem poziția de start lateral, pe perpendiculara tangentei
        float lateralOffset = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 3.0f; // Offset între -1 și 1
        glm::vec3 position = startPosition + perpendicularDirection * lateralOffset + positionOffset;

        // Direcția particulelor: combinație între jos (-Y) și opus tangentei curbei
        glm::vec3 downwardDirection = glm::vec3(0.0f, -1.0f, 0.0f);
        glm::vec3 oppositeTangentDirection = -startTangent;

        glm::vec3 combinedDirection = glm::normalize(downwardDirection + oppositeTangentDirection);

        glm::vec4 pos(position.x, position.y + 1.0f, position.z, 1.0f); // Poziție deasupra curbei
        glm::vec4 speed(combinedDirection * 0.5f, 0.0f); // Viteză pe direcția combinată

        float delay = (rand() % 100 / 100.0f) * 3.0f;
        float lifetime = 5.0f + (rand() % 100 / 100.0f) * 3.0f; // Durată de viață între 5 și 8 secunde

        data[i].SetInitial(pos, speed, delay, lifetime);
    }

    particleSSBO->SetBufferData(data);
}








void Tema1::Init()
{
    auto camera = GetSceneCamera();
    camera->SetPositionAndRotation(glm::vec3(0, 2, 3.5), glm::quat(glm::vec3(-20 * TO_RADIANS, 0, 0))); // Ajustează înălțimea și unghiul camerei
    camera->Update();

    ToggleGroundPlane();
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders");



    // Shader pentru teren
    {
        Shader* shader = new Shader("SurfaceGeneration");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders", "VertexShader.glsl"), GL_VERTEX_SHADER);
        // shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders", "GeometryShader.glsl"), GL_GEOMETRY_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders", "FragmentShader.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
    // Create a shader program for standard rendering
    {
        Shader* shader = new Shader("ShaderNormal");
        shader->AddShader(PATH_JOIN(shaderPath, "Normal.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Normal.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Create a shader program for creating a CUBEMAP
    {
        Shader* shader = new Shader("Framebuffer");
        shader->AddShader(PATH_JOIN(shaderPath, "Framebuffer.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Framebuffer.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "Framebuffer.GS.glsl"), GL_GEOMETRY_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        Shader* shader = new Shader("CubeMap");
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders", "CubeMap.VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders", "CubeMap.FS.glsl"), GL_FRAGMENT_SHADER);
        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    {
        Mesh* mesh = new Mesh("cube");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("box");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    

    {
		Mesh* mesh = new Mesh("plane");
		mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
		mesh->UseMaterials(false);
		meshes[mesh->GetMeshID()] = mesh;
	}

    // Încarcă texturile cubemap-ului
    std::string texturePath = PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cubemap_night");
    cubeMapTextureID = UploadCubeMapTexture(
        PATH_JOIN(texturePath, "pos_x.png"),
        PATH_JOIN(texturePath, "pos_y.png"),
        PATH_JOIN(texturePath, "pos_z.png"),
        PATH_JOIN(texturePath, "neg_x.png"),
        PATH_JOIN(texturePath, "neg_y.png"),
        PATH_JOIN(texturePath, "neg_z.png")
    );

    // Încarcă mesh-ul cubului
    Mesh* cube = new Mesh("cube");
    cube->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
    meshes["cube"] = cube;


    // Generăm terenul procedural
    int m = 20; // Dimensiunea grid-ului pe axa X
    int n = 20; // Dimensiunea grid-ului pe axa Z
    float hmax = 10.0f;     // Înălțimea maximă
    float r = 10.0f;        // Raza de influență

    meshes["terrain"] = GenerateTerrainMesh(m, n, hmax, r);


    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "rain.png");

    LoadShader("RainSnow", "Particle_rain_snow", "Particle_simple", "Particle", true);

    //ResetParticlesRainSnow(10, 10, 10);
    generator_position = glm::vec3(7.1, -2, -1.5);
    scene = 0;
    offset = 0.1;

    std::vector<glm::vec3> bezierControlPoints = {
        glm::vec3(5, 0, 5),
        glm::vec3(10, -3, 15),
        glm::vec3(15, -7, 20),
        glm::vec3(20, -10, 25)
    };

    // Generează particule de-a lungul curbei Bezier
    ResetParticlesAlongBezier(bezierControlPoints, 50000);
}


void Tema1::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}


void Tema1::RenderMeshInstanced(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, int instances, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader
    glUseProgram(shader->program);

    // Bind model matrix
    GLint loc_model_matrix = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Bind view matrix
    glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
    int loc_view_matrix = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(loc_view_matrix, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    // Bind projection matrix
    glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
    int loc_projection_matrix = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    // Draw the object instanced
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElementsInstanced(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, (void*)0, instances);
}


void Tema1::Update(float deltaTimeSeconds)
{
    //ClearScreen(glm::vec3(0.0f, 0.0f, 0.0f)); // Fundal alb
    //glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Mod wireframe

    angle += 0.5f * deltaTimeSeconds;

    auto camera = GetSceneCamera();

    // Draw the scene in Framebuffer
    if (framebuffer_object)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);
        // Set the clear color for the color buffer
        glClearColor(0, 0, 0, 1);
        // Clears the color buffer (using the previously set color) and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glViewport(0, 0, 1024, 1024);

        Shader* shader = shaders["Framebuffer"];
        shader->Use();

        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);

        {
            glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(30));

            glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
            glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
            glUniform1i(glGetUniformLocation(shader->program, "texture_cubemap"), 1);

            glUniform1i(glGetUniformLocation(shader->program, "cube_draw"), 1);

            meshes["cube"]->Render();
        }



        glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        //reset drawing to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, window->GetResolution().x, window->GetResolution().y);

    // Draw the cubemap
    {
        Shader* shader = shaders["ShaderNormal"];
        shader->Use();

        glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(30));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
        int loc_texture = shader->GetUniformLocation("texture_cubemap");
        glUniform1i(loc_texture, 0);

        meshes["cube"]->Render();
    }


    // Draw the reflection on the mesh
    {
        Shader* shader = shaders["CubeMap"];
        shader->Use();

        glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(0.1f));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        auto cameraPosition = camera->m_transform->GetWorldPosition();

        if (!color_texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
            int loc_texture = shader->GetUniformLocation("texture_cubemap");
            glUniform1i(loc_texture, 0);
        }

        if (color_texture) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);
            int loc_texture2 = shader->GetUniformLocation("texture_cubemap");
            glUniform1i(loc_texture2, 1);
        }


        int loc_camera = shader->GetUniformLocation("camera_position");
        glUniform3f(loc_camera, cameraPosition.x, cameraPosition.y, cameraPosition.z);

        glUniform1i(shader->GetUniformLocation("type"), type);
        glm::mat4 modelMatrix2 = glm::scale(glm::mat4(1), glm::vec3(30));
        modelMatrix2 = glm::translate(glm::mat4(1), glm::vec3(0, -7, 0));
        glUniformMatrix4fv(shader->GetUniformLocation("Model"), 1, GL_FALSE, glm::value_ptr(modelMatrix2));

        meshes["plane"]->Render();
    }

    /*glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(glm::mat4(1), glm::vec3(-12, -3, 0));
    Shader* shader = shaders["SurfaceGeneration"];
    shader->Use();

    RenderMesh(meshes["terrain"], shader, modelMatrix);*/

    glm::mat4 modelMatrix = glm::mat4(1);

    // Scalare pentru a face terenul să fie la fel de mare ca și cubemapul
    float cubemapSize = 30.0f; // Dimensiunea cubemapului
    float terrainScaleFactor = cubemapSize / 20.0f; // Asumând că terenul are dimensiunea de 20x20 (m și n)

    // Aplicăm scalarea și poziționarea
    modelMatrix = glm::translate(glm::mat4(1), glm::vec3(-cubemapSize / 2.0f, 0, -cubemapSize / 2.0f)); // Poziționare centrală
    modelMatrix = glm::scale(modelMatrix, glm::vec3(terrainScaleFactor)); // Scalare

    Shader* shader = shaders["SurfaceGeneration"];
    shader->Use();

    RenderMesh(meshes["terrain"], shader, modelMatrix);


    glEnable(GL_DEPTH_TEST);
    Shader * particleShader= shaders["RainSnow"];
    
        particleShader->Use();

        // TODO(student): Send correct texture for rain
        TextureManager::GetTexture("rain.png")->BindToTextureUnit(GL_TEXTURE0);
        particleEffect->Render(GetSceneCamera(), particleShader);

        // TODO(student): Send uniforms generator_position,
        // deltaTime and offset to the shader
        int location = glGetUniformLocation(particleShader->program, "deltaTime");
        glUniform1f(location, deltaTimeSeconds);

        location = glGetUniformLocation(particleShader->program, "offset");
        glUniform1f(location, offset);

        location = glGetUniformLocation(particleShader->program, "generator_position");
        //glUniform3f(location, generator_position.x, generator_position.y, generator_position.z);
        glUniform3fv(location, 1, glm::value_ptr(generator_position));

    


}

void Tema1::FrameEnd()
{

    // DrawCoordinateSystem();

}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Tema1::OnInputUpdate(float deltaTime, int mods)
{
   
}


void Tema1::LoadShader(const std::string& name, const std::string& VS, const std::string& FS, const std::string& GS, bool hasGeomtery)
{
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "Tema1", "shaders");

    // Create a shader program for particle system
    {
        Shader* shader = new Shader(name);
        shader->AddShader(PATH_JOIN(shaderPath, VS + ".VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, FS + ".FS.glsl"), GL_FRAGMENT_SHADER);
        if (hasGeomtery)
        {
            shader->AddShader(PATH_JOIN(shaderPath, GS + ".GS.glsl"), GL_GEOMETRY_SHADER);
        }

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}


void Tema1::OnKeyPress(int key, int mods)
{
    // TODO(student): Use keys to change the number of instances and the
    // number of generated points. Avoid the camera keys, and avoid the
    // the keys from `OnInputUpdate`.

}


void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Tema1::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
