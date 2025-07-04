#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "CelestialBody.h"
#include "Tools.h"
// Add ImGui headers
#include "imgui.h"
#include "rlImGui.h"

int main() 
{
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    // Initial render texture dimensions (will be adjusted based on ImGui window size)
    int renderTextureWidth = 512;
    int renderTextureHeight = 384;
        
    // Enable window resizing before initialization
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(screenWidth, screenHeight, "Celestial Bodies Renderer");
    
    // Create render texture for the camera view
    RenderTexture2D cameraRenderTexture = LoadRenderTexture(renderTextureWidth, renderTextureHeight);
    
    // Initialize ImGui
    rlImGuiSetup(true);
    
    // Variables to track current window size
    int currentWidth = screenWidth;
    int currentHeight = screenHeight;
    
    // Define the camera
    Camera3D camera = { 0 };
    camera.position = Vector3{ -5.0f, 1.0f, -8.0f };  // Camera position
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };    // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };        // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type
    
    // Define the light position (in world space)
    Vector3 lightPos = { 5.0f, 3.0f, 0.0f };
    
    // Simulation pause state
    bool simulationPaused = false;
    
    // Video recording state
    bool isRecording = false;

    // Create pause button
    Rectangle pauseButton = { screenWidth - 110.0f, 10.0f, 100.0f, 30.0f };
    
    // Create Earth celestial body
    CelestialBody earth("Earth", 1.0f, 10.0f); // Name, radius, rotation speed
    earth.Initialize(
        "resources/model/sphere.glb",
        "resources/images/2k_earth_daymap.png",
        "resources/images/2k_earth_normal_map.png",
        "resources/images/2k_earth_specular_map.png",
        "resources/images/2k_earth_nightmap.png",
        "resources/images/2k_earth_clouds.png"
    );
    
    // Create Moon celestial body
    CelestialBody moon("Moon", 0.27f, 6.0f); // Name, radius (27% of Earth), rotation speed
    moon.Initialize(
        "resources/model/sphere.glb",
       "resources/images/Moon.Diffuse.png",
        "resources/images/Moon.Normal.png"
    );
    moon.SetScale(0.27f); // Set moon scale to 27% of Earth's size
    moon.SetOrbit(&earth, 4.5f, 5.0f, 5.0f); // Parent, distance, speed, tilt
    

    SetTargetFPS(60);  // Set our game to run at 60 frames-per-second
    
    Mesh cube = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model skybox = LoadModelFromMesh(cube);
 
// Load skybox shader and set required locations
    // NOTE: Some locations are automatically set at shader loading
    skybox.materials[0].shader = LoadShader(TextFormat("resources/shaders/skybox.vs"),
                                            TextFormat("resources/shaders/skybox.fs"));

    bool useHDR = false; // Use HDR for skybox rendering
    int MATERIAL_MAP_CUBEMAPL = MATERIAL_MAP_CUBEMAP;
    int useHDRValue = useHDR ? 1 : 0;
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "environmentMap"), &MATERIAL_MAP_CUBEMAPL, SHADER_UNIFORM_INT);
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "doGamma"), &useHDRValue, SHADER_UNIFORM_INT);
    SetShaderValue(skybox.materials[0].shader, GetShaderLocation(skybox.materials[0].shader, "vflipped"), &useHDRValue, SHADER_UNIFORM_INT);

       // Load cubemap shader and setup required shader locations
    Shader shdrCubemap = LoadShader(TextFormat("resources/shaders/cubemap.vs"),
                                    TextFormat("resources/shaders/cubemap.fs"));

    int none = 0;
    SetShaderValue(shdrCubemap, GetShaderLocation(shdrCubemap, "equirectangularMap"), &none, SHADER_UNIFORM_INT);

    const char* skyboxFileName = "resources/images/starmap_2020_4k.hdr"; // Path to the panorama image
    Texture2D panorama = LoadTexture(skyboxFileName);
    skybox.materials[0].maps[MATERIAL_MAP_CUBEMAP].texture = GenTextureCubemap(shdrCubemap, panorama, 1024, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8);

    UnloadTexture(panorama);        // Texture not required anymore, cubemap already generated
    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {        
        // Check if window has been resized
        if (IsWindowResized())
        {
            // Update current window dimensions
            currentWidth = GetScreenWidth();
            currentHeight = GetScreenHeight();
            
            // Update UI element positions based on new window size
            pauseButton.x = currentWidth - 110.0f;
            
            // Note: We don't need to update the render texture size here
            // It will be updated when ImGui window size changes
        }
        
        UpdateCamera(&camera, CAMERA_ORBITAL); // Update camera based on user input
        
        // Update celestial bodies
        float deltaTime = GetFrameTime();
        if (!simulationPaused) {
            earth.Update(deltaTime);
            moon.Update(deltaTime);
        }
        
        // Update shader values with current camera and light positions
        earth.UpdateShaderValues(camera, lightPos);
        moon.UpdateShaderValues(camera, lightPos);
        
        // Draw
        BeginDrawing();
        
            ClearBackground(GRAY);
            
            // Only render the 3D scene to the render texture
            BeginTextureMode(cameraRenderTexture);
                ClearBackground(BLACK);
                BeginMode3D(camera);
                    rlDisableBackfaceCulling();
                    rlDisableDepthMask();
                        DrawModel(skybox, Vector3{0, 0, 0}, 1.0f, BLACK);
                    rlEnableBackfaceCulling();
                    rlEnableDepthMask();

                    earth.Draw(camera);
                    moon.Draw(camera);            
                EndMode3D();
            EndTextureMode();
            
            // Begin ImGui frame
            rlImGuiBegin();
            
            // Create ImGui window for the camera render texture
            if (ImGui::Begin("Camera View"))
            {
                // Get the ImGui window content region dimensions
                ImVec2 contentSize = ImGui::GetContentRegionAvail();
                
                // Check if window size has changed significantly (more than 10 pixels in either dimension)
                // to avoid recreating the texture too frequently
                if (abs((int)contentSize.x - renderTextureWidth) > 10 || abs((int)contentSize.y - renderTextureHeight) > 10)
                {
                    // Update render texture dimensions
                    renderTextureWidth = (int)contentSize.x;
                    renderTextureHeight = (int)contentSize.y;
                    
                    // Make sure dimensions are at least 64x64
                    renderTextureWidth = (renderTextureWidth < 64) ? 64 : renderTextureWidth;
                    renderTextureHeight = (renderTextureHeight < 64) ? 64 : renderTextureHeight;
                    
                    // Unload existing texture and create a new one with the updated size
                    UnloadRenderTexture(cameraRenderTexture);
                    cameraRenderTexture = LoadRenderTexture(renderTextureWidth, renderTextureHeight);
                }
                
                // Display the render texture filling the entire content area
                ImGui::Image((ImTextureID)(intptr_t)cameraRenderTexture.texture.id, 
                            ImVec2(contentSize.x, contentSize.y), 
                            ImVec2(0, 1),  // UV0: flip vertically
                            ImVec2(1, 0)); // UV1: flip vertically
            }
            ImGui::End();
            
            // Create ImGui windows and controls
            if (ImGui::Begin("Simulation Controls"))
            {
                ImGui::Checkbox("Pause Simulation", &simulationPaused);
                
                ImGui::Separator();
                
                if (ImGui::TreeNode("Earth Properties"))
                {
                    float earthRotationSpeed = earth.GetRotationSpeed();
                    if (ImGui::SliderFloat("Rotation Speed", &earthRotationSpeed, 0.0f, 20.0f))
                    {
                        earth.SetRotationSpeed(earthRotationSpeed);
                    }
                    
                    ImGui::TreePop();
                }
                
                if (ImGui::TreeNode("Moon Properties"))
                {
                    float moonRotationSpeed = moon.GetRotationSpeed();
                    if (ImGui::SliderFloat("Rotation Speed", &moonRotationSpeed, 0.0f, 20.0f))
                    {
                        moon.SetRotationSpeed(moonRotationSpeed);
                    }
                    
                    float moonOrbitSpeed = moon.GetOrbitSpeed();
                    if (ImGui::SliderFloat("Orbit Speed", &moonOrbitSpeed, 0.0f, 10.0f))
                    {
                        moon.SetOrbitSpeed(moonOrbitSpeed);
                    }
                    
                    ImGui::TreePop();
                }
                
                ImGui::Separator();
                
                if (ImGui::Button("Reset Camera"))
                {
                    camera.position = Vector3{ -5.0f, 1.0f, -8.0f };
                    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
                }
            }
            ImGui::End();
            
            // End ImGui frame
            rlImGuiEnd();
            
            DrawFPS(5, 5);
            
        EndDrawing();
    }
    
    // Shutdown ImGui before closing
    rlImGuiShutdown();
    
    // Unload the render texture
    UnloadRenderTexture(cameraRenderTexture);
    
    earth = CelestialBody(); // Clean up Earth celestial body
    moon = CelestialBody();   // Clean up Moon celestial body
    // No need to manually unload textures and models, the CelestialBody destructor will handle it
    CloseWindow();     // Close window and OpenGL context
    
    return 0;
}