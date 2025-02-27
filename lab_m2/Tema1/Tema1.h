#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"
#include "core/gpu/particle_effect.h"


namespace m2
{
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        Tema1();
        ~Tema1();


        void ResetParticlesAlongBezier(std::vector<glm::vec3>& bezierControlPoints, unsigned int nrParticles);

        void Init() override;
        std::unordered_map<std::string, Texture2D*> mapTextures;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;
        void CreateFramebuffer(int width, int height);

        void RenderMeshInstanced(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, int instances, const glm::vec3& color = glm::vec3(1));
        unsigned int UploadCubeMapTexture(const std::string& pos_x, const std::string& pos_y, const std::string& pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z);

        void OnInputUpdate(float deltaTime, int mods) override;
        void LoadShader(const std::string& name, const std::string& VS, const std::string& FS, const std::string& GS, bool hasGeomtery);
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;
        Mesh* GenerateTerrainMesh(int m, int n, float hmax, float r);
        

    protected:
        // Info about the generated surfaces
        glm::vec3 control_p0, control_p1, control_p2, control_p3;
        unsigned int no_of_generated_points, no_of_instances;
        float max_translate, max_rotate;
        int cubeMapTextureID;
        float angle;
        unsigned int framebuffer_object;
        unsigned int color_texture;
        unsigned int depth_texture;
        unsigned int type;

        glm::mat4 modelMatrix;
        glm::vec3 generator_position;
        GLenum polygonMode;
        int scene;
        float offset;
        int location;
        bool isPaused;
    };
}   // namespace m2
