#include <vulkan/vulkan.h>
#include "Instance.h"
#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"
#include "Image.h"
#include <chrono>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <vector>
#include <iostream>

Device* device;
SwapChain* swapChain;
Renderer* renderer;
Camera* camera;
Scene* scene;

// Performance testing
enum class CullingMode {
    NONE,
    ORIENTATION,
    FRUSTUM,
    DISTANCE,
    ALL
};

struct PerformanceData {
    CullingMode mode;
    std::vector<double> frameTimes;
    double averageFrameTime;
    double fps;
};

namespace {
    void resizeCallback(GLFWwindow* window, int width, int height) {
        if (width == 0 || height == 0) return;

        vkDeviceWaitIdle(device->GetVkDevice());
        swapChain->Recreate();
        renderer->RecreateFrameResources();
    }

    bool leftMouseDown = false;
    bool rightMouseDown = false;
    double previousX = 0.0;
    double previousY = 0.0;

    void mouseDownCallback(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            if (action == GLFW_PRESS) {
                leftMouseDown = true;
                glfwGetCursorPos(window, &previousX, &previousY);
            }
            else if (action == GLFW_RELEASE) {
                leftMouseDown = false;
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            if (action == GLFW_PRESS) {
                rightMouseDown = true;
                glfwGetCursorPos(window, &previousX, &previousY);
            }
            else if (action == GLFW_RELEASE) {
                rightMouseDown = false;
            }
        }
    }

    void mouseMoveCallback(GLFWwindow* window, double xPosition, double yPosition) {
        if (leftMouseDown) {
            double sensitivity = 0.5;
            float deltaX = static_cast<float>((previousX - xPosition) * sensitivity);
            float deltaY = static_cast<float>((previousY - yPosition) * sensitivity);

            camera->UpdateOrbit(deltaX, deltaY, 0.0f);

            previousX = xPosition;
            previousY = yPosition;
        }
        else if (rightMouseDown) {
            double deltaZ = static_cast<float>((previousY - yPosition) * 0.05);

            camera->UpdateOrbit(0.0f, 0.0f, deltaZ);

            previousY = yPosition;
        }
    }

    // Performance testing state
    bool isRecording = false;
    int currentTestMode = 0;
    std::vector<PerformanceData> performanceResults;
    std::vector<double> currentFrameTimes;
    int recordingFrames = 0;
    const int FRAMES_TO_RECORD = 300; // Record 300 frames per test

    void setCullingMode(CullingMode mode) {
        switch (mode) {
        case CullingMode::NONE:
            scene->SetAllCulling(false);
            break;
        case CullingMode::ORIENTATION:
            scene->SetOrientationCulling(true);
            scene->SetFrustumCulling(false);
            scene->SetDistanceCulling(false);
            break;
        case CullingMode::FRUSTUM:
            scene->SetOrientationCulling(false);
            scene->SetFrustumCulling(true);
            scene->SetDistanceCulling(false);
            break;
        case CullingMode::DISTANCE:
            scene->SetOrientationCulling(false);
            scene->SetFrustumCulling(false);
            scene->SetDistanceCulling(true);
            break;
        case CullingMode::ALL:
            scene->SetAllCulling(true);
            break;
        }
    }

    std::string getModeString(CullingMode mode) {
        switch (mode) {
        case CullingMode::NONE: return "None";
        case CullingMode::ORIENTATION: return "Orientation";
        case CullingMode::FRUSTUM: return "Frustum";
        case CullingMode::DISTANCE: return "Distance";
        case CullingMode::ALL: return "All";
        default: return "Unknown";
        }
    }

    void savePerformanceResults() {
        std::ofstream file("performance_results.csv");
        file << "Culling Mode,Average Frame Time (ms),FPS\n";

        for (const auto& data : performanceResults) {
            file << getModeString(data.mode) << ","
                << data.averageFrameTime << ","
                << data.fps << "\n";
        }

        file.close();
        std::cout << "Performance results saved to performance_results.csv" << std::endl;

        // Print to console too
        std::cout << "\n=== PERFORMANCE RESULTS ===" << std::endl;
        std::cout << std::setw(15) << "Mode" << std::setw(20) << "Avg Frame Time (ms)" << std::setw(10) << "FPS" << std::endl;
        std::cout << std::string(45, '-') << std::endl;
        for (const auto& data : performanceResults) {
            std::cout << std::setw(15) << getModeString(data.mode)
                << std::setw(20) << std::fixed << std::setprecision(4) << data.averageFrameTime
                << std::setw(10) << std::fixed << std::setprecision(1) << data.fps << std::endl;
        }
        std::cout << "==========================\n" << std::endl;
    }
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // Sphere movement
        float speed = 0.5f;
        glm::vec3 currentPos = glm::vec3(scene->GetSpherePosition());

        if (key == GLFW_KEY_W) currentPos.z -= speed;
        if (key == GLFW_KEY_S) currentPos.z += speed;
        if (key == GLFW_KEY_A) currentPos.x -= speed;
        if (key == GLFW_KEY_D) currentPos.x += speed;
        if (key == GLFW_KEY_Q) currentPos.y -= speed;
        if (key == GLFW_KEY_E) currentPos.y += speed;

        scene->SetSpherePosition(currentPos);

        // Performance testing controls
        if (key == GLFW_KEY_P && !isRecording) {
            // Start performance test
            std::cout << "\n=== STARTING PERFORMANCE TEST ===" << std::endl;
            std::cout << "Testing 5 culling modes with " << FRAMES_TO_RECORD << " frames each..." << std::endl;
            std::cout << "Please don't move the camera during testing!\n" << std::endl;

            isRecording = true;
            currentTestMode = 0;
            performanceResults.clear();
            currentFrameTimes.clear();
            recordingFrames = 0;

            setCullingMode(CullingMode::NONE);
            std::cout << "Testing: None" << std::endl;
        }

        // Manual culling toggles (for manual testing)
        if (key == GLFW_KEY_1) {
            scene->SetAllCulling(false);
            std::cout << "Culling: NONE" << std::endl;
        }
        if (key == GLFW_KEY_2) {
            setCullingMode(CullingMode::ORIENTATION);
            std::cout << "Culling: ORIENTATION only" << std::endl;
        }
        if (key == GLFW_KEY_3) {
            setCullingMode(CullingMode::FRUSTUM);
            std::cout << "Culling: FRUSTUM only" << std::endl;
        }
        if (key == GLFW_KEY_4) {
            setCullingMode(CullingMode::DISTANCE);
            std::cout << "Culling: DISTANCE only" << std::endl;
        }
        if (key == GLFW_KEY_5) {
            scene->SetAllCulling(true);
            std::cout << "Culling: ALL" << std::endl;
        }
    }
}

int main() {
    static constexpr char* applicationName = "Vulkan Grass Rendering";
    InitializeWindow(640, 480, applicationName);

    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    Instance* instance = new Instance(applicationName, glfwExtensionCount, glfwExtensions);

    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(instance->GetVkInstance(), GetGLFWWindow(), nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface");
    }

    instance->PickPhysicalDevice({ VK_KHR_SWAPCHAIN_EXTENSION_NAME }, QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit, surface);

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.tessellationShader = VK_TRUE;
    deviceFeatures.fillModeNonSolid = VK_TRUE;
    deviceFeatures.samplerAnisotropy = VK_TRUE;

    device = instance->CreateDevice(QueueFlagBit::GraphicsBit | QueueFlagBit::TransferBit | QueueFlagBit::ComputeBit | QueueFlagBit::PresentBit, deviceFeatures);

    swapChain = device->CreateSwapChain(surface, 5);

    camera = new Camera(device, 640.f / 480.f);

    VkCommandPoolCreateInfo transferPoolInfo = {};
    transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    transferPoolInfo.queueFamilyIndex = device->GetInstance()->GetQueueFamilyIndices()[QueueFlags::Transfer];
    transferPoolInfo.flags = 0;

    VkCommandPool transferCommandPool;
    if (vkCreateCommandPool(device->GetVkDevice(), &transferPoolInfo, nullptr, &transferCommandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }

    VkImage grassImage;
    VkDeviceMemory grassImageMemory;
    Image::FromFile(device,
        transferCommandPool,
        "images/grass.jpg",
        VK_FORMAT_R8G8B8A8_UNORM,
        VK_IMAGE_TILING_OPTIMAL,
        VK_IMAGE_USAGE_SAMPLED_BIT,
        VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        grassImage,
        grassImageMemory
    );

    float planeDim = 15.f;
    float halfWidth = planeDim * 0.5f;
    Model* plane = new Model(device, transferCommandPool,
        {
            { { -halfWidth, 0.0f, halfWidth }, { 1.0f, 0.0f, 0.0f },{ 1.0f, 0.0f } },
            { { halfWidth, 0.0f, halfWidth }, { 0.0f, 1.0f, 0.0f },{ 0.0f, 0.0f } },
            { { halfWidth, 0.0f, -halfWidth }, { 0.0f, 0.0f, 1.0f },{ 0.0f, 1.0f } },
            { { -halfWidth, 0.0f, -halfWidth }, { 1.0f, 1.0f, 1.0f },{ 1.0f, 1.0f } }
        },
        { 0, 1, 2, 2, 3, 0 }
    );
    plane->SetTexture(grassImage);

    Blades* blades = new Blades(device, transferCommandPool, planeDim);

    vkDestroyCommandPool(device->GetVkDevice(), transferCommandPool, nullptr);

    scene = new Scene(device);
    scene->AddModel(plane);
    scene->AddBlades(blades);

    renderer = new Renderer(device, swapChain, scene, camera);

    glfwSetWindowSizeCallback(GetGLFWWindow(), resizeCallback);
    glfwSetMouseButtonCallback(GetGLFWWindow(), mouseDownCallback);
    glfwSetCursorPosCallback(GetGLFWWindow(), mouseMoveCallback);
    glfwSetKeyCallback(GetGLFWWindow(), keyCallback);

    // FPS tracking variables
    using namespace std::chrono;
    auto lastTime = high_resolution_clock::now();
    int frameCount = 0;
    double fps = 0.0;
    double updateInterval = 0.5;
    double timeSinceLastUpdate = 0.0;

    std::cout << "\n=== CONTROLS ===" << std::endl;
    std::cout << "Press P: Start automatic performance test" << std::endl;
    std::cout << "Press 1-5: Manual culling mode switch (1=None, 2=Orientation, 3=Frustum, 4=Distance, 5=All)" << std::endl;
    std::cout << "WASDQE: Move sphere" << std::endl;
    std::cout << "================\n" << std::endl;

    while (!ShouldQuit()) {
        glfwPollEvents();

        // Calculate delta time and FPS
        auto currentTime = high_resolution_clock::now();
        duration<double> deltaTime = duration_cast<duration<double>>(currentTime - lastTime);
        lastTime = currentTime;

        double frameTimeMs = deltaTime.count() * 1000.0;

        // Performance recording
        if (isRecording) {
            currentFrameTimes.push_back(frameTimeMs);
            recordingFrames++;

            if (recordingFrames >= FRAMES_TO_RECORD) {
                // Calculate average
                double sum = 0.0;
                for (double ft : currentFrameTimes) {
                    sum += ft;
                }
                double avgFrameTime = sum / currentFrameTimes.size();
                double avgFPS = 1000.0 / avgFrameTime;

                // Store results
                PerformanceData data;
                data.mode = static_cast<CullingMode>(currentTestMode);
                data.frameTimes = currentFrameTimes;
                data.averageFrameTime = avgFrameTime;
                data.fps = avgFPS;
                performanceResults.push_back(data);

                std::cout << getModeString(data.mode) << " - Avg: "
                    << std::fixed << std::setprecision(4) << avgFrameTime
                    << " ms (" << std::setprecision(1) << avgFPS << " FPS)" << std::endl;

                // Move to next test
                currentTestMode++;
                currentFrameTimes.clear();
                recordingFrames = 0;

                if (currentTestMode < 5) {
                    setCullingMode(static_cast<CullingMode>(currentTestMode));
                    std::cout << "Testing: " << getModeString(static_cast<CullingMode>(currentTestMode)) << std::endl;
                }
                else {
                    // Done with all tests
                    isRecording = false;
                    savePerformanceResults();

                    // Reset to all culling
                    scene->SetAllCulling(true);
                }
            }
        }

        frameCount++;
        timeSinceLastUpdate += deltaTime.count();

        // Update FPS display
        if (timeSinceLastUpdate >= updateInterval) {
            fps = frameCount / timeSinceLastUpdate;

            std::ostringstream titleStream;
            titleStream << "Vulkan Grass Rendering - FPS: " << std::fixed << std::setprecision(1) << fps;
            if (isRecording) {
                titleStream << " [RECORDING " << getModeString(static_cast<CullingMode>(currentTestMode))
                    << " " << recordingFrames << "/" << FRAMES_TO_RECORD << "]";
            }
            glfwSetWindowTitle(GetGLFWWindow(), titleStream.str().c_str());

            frameCount = 0;
            timeSinceLastUpdate = 0.0;
        }

        scene->UpdateTime();
        renderer->Frame();
    }

    vkDeviceWaitIdle(device->GetVkDevice());

    vkDestroyImage(device->GetVkDevice(), grassImage, nullptr);
    vkFreeMemory(device->GetVkDevice(), grassImageMemory, nullptr);

    delete scene;
    delete plane;
    delete blades;
    delete camera;
    delete renderer;
    delete swapChain;
    delete device;
    delete instance;
    DestroyWindow();
    return 0;
}
