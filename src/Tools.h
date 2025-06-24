#ifndef TOOLS_H
#define TOOLS_H

#include "raylib.h"
#include "rlgl.h"
#include "raymath.h"

// Generate cubemap texture from HDR texture
TextureCubemap GenTextureCubemap(Shader shader, Texture2D panorama, int size, int format);

#endif // TOOLS_H
