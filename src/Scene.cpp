#include "Scene.h"
#include "BufferUtils.h"

Scene::Scene(Device* device) : device(device) {
    BufferUtils::CreateBuffer(device, sizeof(Time), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, timeBuffer, timeBufferMemory);
    vkMapMemory(device->GetVkDevice(), timeBufferMemory, 0, sizeof(Time), 0, &mappedData);

    // Initialize with all culling enabled
    time.cullingFlags = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);

    memcpy(mappedData, &time, sizeof(Time));
}

const std::vector<Model*>& Scene::GetModels() const {
    return models;
}

const std::vector<Blades*>& Scene::GetBlades() const {
    return blades;
}

void Scene::AddModel(Model* model) {
    models.push_back(model);
}

void Scene::AddBlades(Blades* blades) {
    this->blades.push_back(blades);
}

void Scene::UpdateTime() {
    high_resolution_clock::time_point currentTime = high_resolution_clock::now();
    duration<float> nextDeltaTime = duration_cast<duration<float>>(currentTime - startTime);
    startTime = currentTime;

    time.timeData.x = nextDeltaTime.count();  // deltaTime
    time.timeData.y += time.timeData.x;        // totalTime

    memcpy(mappedData, &time, sizeof(Time));
}

VkBuffer Scene::GetTimeBuffer() const {
    return timeBuffer;
}

glm::vec4 Scene::GetSpherePosition() const {
    return time.spherePosition;
}

void Scene::SetSpherePosition(const glm::vec3& pos) {
    time.spherePosition = glm::vec4(pos, time.spherePosition.w);
    memcpy(mappedData, &time, sizeof(Time));
}

void Scene::SetOrientationCulling(bool enabled) {
    time.cullingFlags.x = enabled ? 1.0f : 0.0f;
    memcpy(mappedData, &time, sizeof(Time));
}

void Scene::SetFrustumCulling(bool enabled) {
    time.cullingFlags.y = enabled ? 1.0f : 0.0f;
    memcpy(mappedData, &time, sizeof(Time));
}

void Scene::SetDistanceCulling(bool enabled) {
    time.cullingFlags.z = enabled ? 1.0f : 0.0f;
    memcpy(mappedData, &time, sizeof(Time));
}

void Scene::SetAllCulling(bool enabled) {
    time.cullingFlags = glm::vec4(enabled ? 1.0f : 0.0f);
    memcpy(mappedData, &time, sizeof(Time));
}

Scene::~Scene() {
    vkUnmapMemory(device->GetVkDevice(), timeBufferMemory);
    vkDestroyBuffer(device->GetVkDevice(), timeBuffer, nullptr);
    vkFreeMemory(device->GetVkDevice(), timeBufferMemory, nullptr);
}