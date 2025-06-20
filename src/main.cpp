#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"
#include "CelestialBody.h"

static TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format);

int main() 
{
    // Initialize window
    const int screenWidth = 800;
    const int screenHeight = 600;
    
    InitWindow(screenWidth, screenHeight, "Celestial Bodies Renderer");
    
    // Define the camera
    Camera3D camera = { 0 };
    camera.position = Vector3{ -5.0f, 1.0f, -8.0f };  // Camera position
    camera.target = Vector3{ 0.0f, 0.0f, 0.0f };    // Camera looking at point
    camera.up = Vector3{ 0.0f, 1.0f, 0.0f };        // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                            // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;         // Camera projection type
    
    // Define the light position (in world space)
    Vector3 lightPos = { 5.0f, 3.0f, 0.0f };
    
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
        UpdateCamera(&camera, CAMERA_FREE); // Update camera based on user input
        // Update celestial bodies
        float deltaTime = GetFrameTime();
        earth.Update(deltaTime);
        moon.Update(deltaTime);
        
        // Update shader values with current camera and light positions
        earth.UpdateShaderValues(camera, lightPos);
        moon.UpdateShaderValues(camera, lightPos);
        
        // Draw
        BeginDrawing();
        
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
            
            DrawFPS(10, 50);
        
        EndDrawing();
    }
    
    // No need to manually unload textures and models, the CelestialBody destructor will handle it
    CloseWindow();     // Close window and OpenGL context
    
    return 0;
}

// Generate cubemap texture from HDR texture
static TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format)
{
    TextureCubemap cubemap = { 0 };

    rlDisableBackfaceCulling();     // Disable backface culling to render inside the cube

    // STEP 1: Setup framebuffer
    //------------------------------------------------------------------------------------------
    unsigned int rbo = rlLoadTextureDepth(size, size, true);
    cubemap.id = rlLoadTextureCubemap(0, size, format, 1);

    unsigned int fbo = rlLoadFramebuffer();
    rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
    rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

    // Check if framebuffer is complete with attachments (valid)
    if (rlFramebufferComplete(fbo)) TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", fbo);
    //------------------------------------------------------------------------------------------

    // STEP 2: Draw to framebuffer
    //------------------------------------------------------------------------------------------
    // NOTE: Shader is used to convert HDR equirectangular environment map to cubemap equivalent (6 faces)
    rlEnableShader(shader.id);

    // Define projection matrix and send it to shader
    Matrix matFboProjection = MatrixPerspective(90.0*DEG2RAD, 1.0, rlGetCullDistanceNear(), rlGetCullDistanceFar());
    rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

    // Define view matrix for every side of the cubemap
    Matrix fboViews[6] = {
        MatrixLookAt(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{  1.0f,  0.0f,  0.0f }, Vector3{ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{ -1.0f,  0.0f,  0.0f }, Vector3{ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{  0.0f,  1.0f,  0.0f }, Vector3{ 0.0f,  0.0f,  1.0f }),
        MatrixLookAt(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{  0.0f, -1.0f,  0.0f }, Vector3{ 0.0f,  0.0f, -1.0f }),
        MatrixLookAt(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{  0.0f,  0.0f,  1.0f }, Vector3{ 0.0f, -1.0f,  0.0f }),
        MatrixLookAt(Vector3{ 0.0f, 0.0f, 0.0f }, Vector3{  0.0f,  0.0f, -1.0f }, Vector3{ 0.0f, -1.0f,  0.0f })
    };

    rlViewport(0, 0, size, size);   // Set viewport to current fbo dimensions
    
    // Activate and enable texture for drawing to cubemap faces
    rlActiveTextureSlot(0);
    rlEnableTexture(panorama.id);

    for (int i = 0; i < 6; i++)
    {
        // Set the view matrix for the current cube face
        rlSetUniformMatrix(shader.locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);
        
        // Select the current cubemap face attachment for the fbo
        // WARNING: This function by default enables->attach->disables fbo!!!
        rlFramebufferAttach(fbo, cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);
        rlEnableFramebuffer(fbo);

        // Load and draw a cube, it uses the current enabled texture
        rlClearScreenBuffers();
        rlLoadDrawCube();

        // ALTERNATIVE: Try to use internal batch system to draw the cube instead of rlLoadDrawCube
        // for some reason this method does not work, maybe due to cube triangles definition? normals pointing out?
        // TODO: Investigate this issue...
        //rlSetTexture(panorama.id); // WARNING: It must be called after enabling current framebuffer if using internal batch system!
        //rlClearScreenBuffers();
        //DrawCubeV(Vector3Zero(), Vector3One(), WHITE);
        //rlDrawRenderBatchActive();
    }
    //------------------------------------------------------------------------------------------

    // STEP 3: Unload framebuffer and reset state
    //------------------------------------------------------------------------------------------
    rlDisableShader();          // Unbind shader
    rlDisableTexture();         // Unbind texture
    rlDisableFramebuffer();     // Unbind framebuffer
    rlUnloadFramebuffer(fbo);   // Unload framebuffer (and automatically attached depth texture/renderbuffer)

    // Reset viewport dimensions to default
    rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
    rlEnableBackfaceCulling();
    //------------------------------------------------------------------------------------------

    cubemap.width = size;
    cubemap.height = size;
    cubemap.mipmaps = 1;
    cubemap.format = format;

    return cubemap;
}