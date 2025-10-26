#pragma once

#include <glm/glm.hpp>
#include <chrono>

#include "Model.h"
#include "Blades.h"

using namespace std::chrono;

struct Time {
    glm::vec4 timeData = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f); // x=deltaTime, y=totalTime, z/w unused
    glm::vec4 spherePosition = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
};

class Scene {
private:
    Device* device;
    
    VkBuffer timeBuffer;
    VkDeviceMemory timeBufferMemory;
    Time time;
    
    void* mappedData;

    std::vector<Model*> models;
    std::vector<Blades*> blades;

high_resolution_clock::time_point startTime = high_resolution_clock::now();

public:
    Scene() = delete;
    Scene(Device* device);
    ~Scene();

    const std::vector<Model*>& GetModels() const;
    const std::vector<Blades*>& GetBlades() const;
    
    void AddModel(Model* model);
    void AddBlades(Blades* blades);

    VkBuffer GetTimeBuffer() const;

    void UpdateTime();
    glm::vec4 GetSpherePosition() const;  
    void SetSpherePosition(const glm::vec3& pos);
};
