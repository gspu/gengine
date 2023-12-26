#include "GAPI_OpenGL.h"

#include <GL/glew.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>

#include "Matrix4.h"
#include "Window.h"

// Some OpenGL calls take in array indexes/offsets as pointers.
// This macro just makes the syntax clearer for the reader.
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

namespace GLState
{
    // The texture unit that is currently active.
    uint8_t activeTextureUnit = 0;
    void SetTextureUnit(uint8_t textureUnit)
    {
        if(activeTextureUnit != textureUnit)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            activeTextureUnit = textureUnit;
        }
    }

    // The active texture on each texture unit.
    const int kMaxTextureUnits = 8;
    GLuint activeTextureId[kMaxTextureUnits] = { GL_NONE };
    void BindTexture(GLuint textureId)
    {
        if(activeTextureId[activeTextureUnit] != textureId)
        {
            glBindTexture(GL_TEXTURE_2D, textureId);
            activeTextureId[activeTextureUnit] = textureId;
        }
    }

    GLuint activeIndexBufferId = GL_NONE;
    void BindIndexBuffer(GLuint indexBufferId)
    {
        if(activeIndexBufferId != indexBufferId)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferId);
            activeIndexBufferId = indexBufferId;
        }
    }

    GLuint activeVertexArrayId = GL_NONE;
    void BindVertexArray(GLuint vertexArrayId)
    {
        if(activeVertexArrayId != vertexArrayId)
        {
            glBindVertexArray(vertexArrayId);
            activeVertexArrayId = vertexArrayId;
        }
    }
}

namespace
{
    struct VertexBuffer
    {
        // The VBO (vertex buffer object) holds all mesh vertex data (position, color, normals, etc).
        GLuint vbo = GL_NONE;

        // The VAO (vertex array object) provides mapping info for the VBO.
        // The VBO is just a big chunk of memory. The VAO dictates how to interpret the memory to read vertex data.
        GLuint vao = GL_NONE;

        // Number of vertices in the buffer.
        uint32_t count = 0;
    };

    struct IndexBuffer
    {
        // The IBO (index buffer object) holds index values for indexed geometry.
        GLuint ibo = GL_NONE;

        // Number of indexes in the buffer.
        uint32_t count = 0;
    };

    GLenum PrimitiveToDrawMode(GAPI::Primitive primitive)
    {
        switch(primitive)
        {
        case GAPI::Primitive::Points:
            return GL_POINTS;
        case GAPI::Primitive::Lines:
            return GL_LINES;
        case GAPI::Primitive::LineLoop:
            return GL_LINE_LOOP;
        default:
        case GAPI::Primitive::Triangles:
            return GL_TRIANGLES;
        case GAPI::Primitive::TriangleStrip:
            return GL_TRIANGLE_STRIP;
        case GAPI::Primitive::TriangleFan:
            return GL_TRIANGLE_FAN;
        }
    }
}

bool GAPI_OpenGL::Init()
{
    // Tell SDL we want to use OpenGL 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    // Request some GL parameters, just in case
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    // Create OpenGL context.
    mContext = SDL_GL_CreateContext(Window::Get());
    if(mContext == nullptr)
    {
        printf("Failed to create OpenGL context!\n");
        return false;
    }

    // Initialize GLEW.
    glewExperimental = GL_TRUE;
    GLenum glewInitResult = glewInit();
    if(glewInitResult != GLEW_OK)
    {
        printf("glewInit failed! Error: %s\n", glewGetErrorString(glewInitResult));
        return false;
    }

    // Clear any GLEW error.
    glGetError();

    // Init OpenGL for IMGUI.
    ImGui_ImplSDL2_InitForOpenGL(Window::Get(), mContext);
    ImGui_ImplOpenGL3_Init("#version 150");

    // Set a global size for GL_POINTS rendering.
    // This is tuned for the points used in Sidney's Map Analysis - will need to set elsewhere if we need different sizes.
    glPointSize(5.0f);

    // Same for GL_LINES rendering.
    glLineWidth(2.0f);
    return true;
}

void GAPI_OpenGL::Shutdown()
{
    // Shutdown OpenGL for IMGUI.
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();

    // Delete GL context.
    SDL_GL_DeleteContext(mContext);
}

void GAPI_OpenGL::ImGuiNewFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
}

void GAPI_OpenGL::ImGuiRenderDrawData()
{
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GAPI_OpenGL::Clear(Color32 clearColor)
{
    glClearColor(clearColor.r / 255.0f,
                 clearColor.g / 255.0f,
                 clearColor.b / 255.0f,
                 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void GAPI_OpenGL::Present()
{
    SDL_GL_SwapWindow(Window::Get());
}

void GAPI_OpenGL::SetPolygonCullMode(CullMode cullMode)
{
    switch(cullMode)
    {
    case CullMode::None:
        glDisable(GL_CULL_FACE);
        break;
    case CullMode::Back:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        break;
    case CullMode::Front:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        break;
    case CullMode::All:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT_AND_BACK);
        break;
    }
}

void GAPI_OpenGL::SetPolygonWindingOrder(WindingOrder windingOrder)
{
    if(windingOrder == WindingOrder::CounterClockwise)
    {
        glFrontFace(GL_CCW);
    }
    else
    {
        glFrontFace(GL_CW);
    }
}

void GAPI_OpenGL::SetPolygonFillMode(FillMode fillMode)
{
    switch(fillMode)
    {
    case FillMode::Wireframe:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    case FillMode::Filled:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    }
}

void GAPI_OpenGL::SetViewSpaceHandedness(Handedness handedness)
{
    // OpenGL's depth buffer defaults assume a right-handed view space.
    if(handedness == Handedness::RightHand)
    {
        // The default behavior.
        glClearDepth(1);
        glDepthFunc(GL_LESS);
    }
    else
    {
        // If using left-hand, we essentially "flip" the depth buffer.
        // Clear to 0 (instead of 1) and use GL_GREATER for depth tests (rather than GL_LESS).
        glClearDepth(0);
        glDepthFunc(GL_GREATER);
    }
}

void GAPI_OpenGL::SetViewport(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
    glViewport(static_cast<GLint>(x), static_cast<GLint>(y),
               static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

void GAPI_OpenGL::SetScissorRect(bool enabled, const Rect& rect)
{
    if(enabled)
    {
        glEnable(GL_SCISSOR_TEST);
        glScissor(static_cast<GLint>(rect.x), static_cast<GLint>(rect.y),
                  static_cast<GLint>(rect.width), static_cast<GLint>(rect.height));
    }
    else
    {
        glDisable(GL_SCISSOR_TEST);
    }
}

void GAPI_OpenGL::SetDepthWriteEnabled(bool enabled)
{
    glDepthMask(enabled ? GL_TRUE : GL_FALSE);
}

void GAPI_OpenGL::SetDepthTestEnabled(bool enabled)
{
    if(enabled)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
}

void GAPI_OpenGL::SetBlendEnabled(bool enabled)
{
    if(enabled)
    {
        glEnable(GL_BLEND);
    }
    else
    {
        glDisable(GL_BLEND);
    }
}

void GAPI_OpenGL::SetBlendMode(BlendMode blendMode)
{
    switch(blendMode)
    {
    case BlendMode::AlphaBlend:
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case BlendMode::Multiply:
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
        break;
    }
}

TextureHandle GAPI_OpenGL::CreateTexture(uint32_t width, uint32_t height, uint8_t* pixels)
{
    // Generate and bind a texture ID.
    GLuint textureId = GL_NONE;
    glGenTextures(1, &textureId);
    GLState::BindTexture(textureId);

    // Create the texture, optionally loading pixel data at the same time.
    //      OpenGL assumes the pixel data is from bottom-left, but GK3 pixel arrays are from top left!
    //      You'd think this would be a problem, but GK3 also uses DX style UVs (top-left).
    //      So...having both the texture and UVs upside down, two wrongs make a right!
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    return reinterpret_cast<TextureHandle>(textureId);
}

void GAPI_OpenGL::DestroyTexture(TextureHandle handle)
{
    GLuint textureId = reinterpret_cast<uintptr_t>(handle);
    glDeleteTextures(1, &textureId);
}

void GAPI_OpenGL::SetTexturePixels(TextureHandle handle, uint32_t width, uint32_t height, uint8_t* pixels)
{
    GLState::BindTexture(reinterpret_cast<uintptr_t>(handle));
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
}

void GAPI_OpenGL::GenerateMipmaps(TextureHandle handle)
{
    GLState::BindTexture(reinterpret_cast<uintptr_t>(handle));
    glGenerateMipmap(GL_TEXTURE_2D);
}

void GAPI_OpenGL::SetTextureWrapMode(TextureHandle handle, Texture::WrapMode wrapMode)
{
    GLState::BindTexture(reinterpret_cast<uintptr_t>(handle));

    GLint wrapParam = (wrapMode == Texture::WrapMode::Repeat ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapParam);
}

void GAPI_OpenGL::SetTextureFilterMode(TextureHandle handle, Texture::FilterMode filterMode, bool useMipmaps)
{
    GLState::BindTexture(reinterpret_cast<uintptr_t>(handle));

    // Determine min/mag filters. These are a little complicated, based on desired filter mode and mipmaps.
    // Defaults (GL_NEAREST) correlate to point filtering.
    GLint minFilterParam = GL_NEAREST;
    GLint magFilterParam = GL_NEAREST;
    if(filterMode == Texture::FilterMode::Bilinear)
    {
        // Bilinear filtering uses GL_LINEAR. We can also use mipmaps with bilinear filtering.
        minFilterParam = useMipmaps ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR;
        magFilterParam = GL_LINEAR;
    }
    else if(filterMode == Texture::FilterMode::Trilinear)
    {
        // Trilinear filtering technically requires mipmaps.
        // If we don't have mipmaps, it is really just bilinear filtering!
        minFilterParam = useMipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
        magFilterParam = GL_LINEAR;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minFilterParam);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magFilterParam);
}

void GAPI_OpenGL::ActivateTexture(TextureHandle handle, uint8_t textureUnit)
{
    GLState::SetTextureUnit(textureUnit);
    GLState::BindTexture(reinterpret_cast<uintptr_t>(handle));
}

TextureHandle GAPI_OpenGL::CreateCubemap(const CubemapParams& params)
{
    // Generate cube map texture id.
    GLuint cubemapTextureId = GL_NONE;
    glGenTextures(1, &cubemapTextureId);

    // Bind the texture id.
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTextureId);

    // Create each texture for the cubemap.
    // Note that we MUST create 6 textures, or the cubemap will not display properly.
    // Also, Front is -Z and Back is +Z. Not sure if this indicates a bug, or a conversion done in the original game, or what?
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, params.right.width, params.right.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, params.right.pixels);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, params.left.width, params.left.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, params.left.pixels);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, params.front.width, params.front.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, params.front.pixels);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, params.back.width, params.back.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, params.back.pixels);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, params.top.width, params.top.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, params.top.pixels);

    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, params.bottom.width, params.bottom.height, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, params.bottom.pixels);

    // These settings help to avoid visible seams around the edges of the skybox.
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return reinterpret_cast<TextureHandle>(cubemapTextureId);
}

void GAPI_OpenGL::DestroyCubemap(TextureHandle handle)
{
    // Same as destroying any other texture.
    DestroyTexture(handle);
}

void GAPI_OpenGL::ActivateCubemap(TextureHandle handle)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, reinterpret_cast<uintptr_t>(handle));
}

BufferHandle GAPI_OpenGL::CreateVertexBuffer(uint32_t vertexCount, const VertexDefinition& vertexDefinition, void* data, MeshUsage usage)
{
    // Create vertex buffer of appropriate size.
    GLuint vertexBufferId = GL_NONE;
    {
        // Generate vertex buffer id.
        glGenBuffers(1, &vertexBufferId);

        // Bind the vertex buffer id.
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

        // Calculate needed size for buffer based on vertex count and size of each vertex.
        uint32_t size = vertexCount * vertexDefinition.CalculateSize();

        // Create buffer associated with the vertex buffer id.
        // It's OK if data is null - this will then just create a properly sized empty buffer.
        glBufferData(GL_ARRAY_BUFFER, size, data, (usage == MeshUsage::Static) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW);
    }

    // A "vertex array object" (VAO) is also needed, which maps the data in the vertex buffer to specific vertex attributes.
    // The VAO state depends on the VBO state, so it's important that the VBO associated with this VAO is already bound (we just created it above).
    GLuint vertexArrayId = GL_NONE;
    {
        // Generate and bind VAO object
        glGenVertexArrays(1, &vertexArrayId);
        GLState::BindVertexArray(vertexArrayId);

        // Stride can be calculated once and used over and over.
        // For packed data, stride is zero. For interleaved data, stride is size of vertex.
        GLsizei stride = vertexDefinition.CalculateStride();

        // Iterate vertex attributes to define the vertex data layout in the VAO.
        size_t attributeIndex = 0;
        for(auto& attribute : vertexDefinition.attributes)
        {
            int attributeId = static_cast<int>(attribute.semantic);

            // Must enable the attribute to use it in shader code.
            glEnableVertexAttribArray(attributeId);

            // Convert attribute values to GL types.
            GLint count = attribute.count;
            GLenum type = GL_FLOAT; //TODO: We can assume float for now...but when supporting other attribute types, we'll need some conversion logic.
            GLboolean normalize = attribute.normalize ? GL_TRUE : GL_FALSE;
            int offset = vertexDefinition.CalculateAttributeOffset(attributeIndex, vertexCount);

            // Define vertex attribute in VAO.
            // Note that this links the VAO to the bound VBO, which is why we only need to bind the VAO when drawing.
            glVertexAttribPointer(attributeId, count, type, normalize, stride, BUFFER_OFFSET(offset));
            ++attributeIndex;
        }
    }

    // Return result.
    VertexBuffer* vertexBuffer = new VertexBuffer();
    vertexBuffer->vbo = vertexBufferId;
    vertexBuffer->vao = vertexArrayId;
    vertexBuffer->count = vertexCount;
    return vertexBuffer;
}

void GAPI_OpenGL::DestroyVertexBuffer(BufferHandle handle)
{
    // It's valid to destroy a null handle.
    if(handle != nullptr)
    {
        VertexBuffer* vertexBuffer = static_cast<VertexBuffer*>(handle);
        glDeleteBuffers(1, &vertexBuffer->vbo);
        glDeleteVertexArrays(1, &vertexBuffer->vao);
        delete vertexBuffer;
    }
}

void GAPI_OpenGL::SetVertexBufferData(BufferHandle handle, uint32_t offset, uint32_t size, void* data)
{
    glBindBuffer(GL_ARRAY_BUFFER, static_cast<VertexBuffer*>(handle)->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
}

BufferHandle GAPI_OpenGL::CreateIndexBuffer(uint32_t indexCount, uint16_t* indexData, MeshUsage usage)
{
    // Generate the buffer id.
    GLuint indexBufferId = GL_NONE;
    glGenBuffers(1, &indexBufferId);

    // Bind the buffer id.
    GLState::BindIndexBuffer(indexBufferId);

    // Create the buffer associated with this buffer id.
    // Even if index data is null, it's fine - this just creates an empty (but properly sized) buffer in that case.
    GLenum glUsage = (usage == MeshUsage::Static) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLushort), indexData, glUsage);

    // Return handle.
    IndexBuffer* indexBuffer = new IndexBuffer();
    indexBuffer->ibo = indexBufferId;
    indexBuffer->count = indexCount;
    return indexBuffer;
}

void GAPI_OpenGL::DestroyIndexBuffer(BufferHandle handle)
{
    // It's valid to destroy a null handle.
    if(handle != nullptr)
    {
        IndexBuffer* indexBuffer = static_cast<IndexBuffer*>(handle);
        glDeleteBuffers(1, &indexBuffer->ibo);
        delete indexBuffer;
    }
}

void GAPI_OpenGL::SetIndexBufferData(BufferHandle handle, uint32_t indexCount, uint16_t* indexData)
{
    // Bind the buffer.
    GLState::BindIndexBuffer(static_cast<IndexBuffer*>(handle)->ibo);

    // Update its data.
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indexCount * sizeof(GLushort), indexData);
}

namespace
{
    GLuint CompileShader(const char* source, GLuint shaderType)
    {
        // Create shader.
        GLuint shaderId = glCreateShader(shaderType);

        // Load source code.
        glShaderSource(shaderId, 1, &source, nullptr);

        // Compile it.
        glCompileShader(shaderId);

        // Ask GL whether compile succeeded.
        GLint compileSucceeded = 0;
        glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileSucceeded);

        // If not, we'll output the error log and clean up.
        if(compileSucceeded == GL_FALSE)
        {
            GLint errorLength = 0;
            glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &errorLength);

            GLchar* errorLog = new GLchar[errorLength];
            glGetShaderInfoLog(shaderId, errorLength, &errorLength, errorLog);

            printf("Failed to compile shader: %s\n", errorLog);
            delete[] errorLog;
            glDeleteShader(shaderId);
            return GL_NONE;
        }
        return shaderId;
    }

    GLuint LinkShaderProgram(GLuint vertexShaderId, GLuint fragmentShaderId)
    {
        // Create a new program and attach the two shaders to it.
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShaderId);
        glAttachShader(program, fragmentShaderId);

        // Bind shader attribute names to attribute indexes.
        // This must be done before linking the program.
        int semanticCount = static_cast<int>(VertexAttribute::Semantic::SemanticCount);
        for(int i = 0; i < semanticCount; ++i)
        {
            glBindAttribLocation(program, i, gAttributeNames[i]);
        }

        // Link the shader program.
        glLinkProgram(program);

        // Check whether link succeeded.
        GLint linkSucceeded = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &linkSucceeded);

        // If not, we'll output the error log.
        if(linkSucceeded == GL_FALSE)
        {
            GLint errorLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &errorLength);

            GLchar* errorLog = new GLchar[errorLength];
            glGetProgramInfoLog(program, errorLength, &errorLength, errorLog);

            printf("Failed to link shader program: %s\n", errorLog);
            delete[] errorLog;
        }

        // Whether link succeeded or not, detach shaders from program after link attempt.
        glDetachShader(program, vertexShaderId);
        glDetachShader(program, fragmentShaderId);

        // If link failed, delete the program.
        if(linkSucceeded == GL_FALSE)
        {
            glDeleteProgram(program);
            program = GL_NONE;
        }

        // Delete the shader objects. They aren't needed anymore after linking.
        glDeleteShader(vertexShaderId);
        glDeleteShader(fragmentShaderId);
        return program;
    }
}

ShaderHandle GAPI_OpenGL::CreateShader(const uint8_t* vertSource, const uint8_t* fragSource)
{
    // Compile the shaders, or fail.
    GLuint vertexShaderId = CompileShader(reinterpret_cast<const char*>(vertSource), GL_VERTEX_SHADER);
    GLuint fragmentShaderId = CompileShader(reinterpret_cast<const char*>(fragSource), GL_FRAGMENT_SHADER);
    if(vertexShaderId == GL_NONE || fragmentShaderId == GL_NONE)
    {
        glDeleteShader(vertexShaderId);
        glDeleteShader(fragmentShaderId);
        return nullptr;
    }

    // Link the shaders to create a program.
    GLuint program = LinkShaderProgram(vertexShaderId, fragmentShaderId);

    // This is not *strictly* related to creating the shader program, but it is currently required.
    // For any 2D texture uniforms in the shader, we need to specify which "texture unit" that sampler should use.
    // To do that, we can use reflection on the shader data to see which texture uniforms exist.
    {
        // We must activate the program, since we may modify uniforms below.
        glUseProgram(program);

        // Info obtained about each uniform.
        const GLsizei kMaxUniformNameLength = 32;
        GLchar uniformNameBuffer[kMaxUniformNameLength];
        GLsizei uniformNameLength = 0;
        GLsizei uniformSize = 0;
        GLenum uniformType = GL_NONE;

        // Determine count of uniforms in this shader program.
        GLint uniformCount = 0;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &uniformCount);

        // Iterate uniforms and process each one.
        int textureUnitCounter = 0;
        for(GLint i = 0; i < uniformCount; ++i)
        {
            // Grab uniform i.
            glGetActiveUniform(program, i, kMaxUniformNameLength, &uniformNameLength, &uniformSize, &uniformType, uniformNameBuffer);

            // If returned name length is 0, that means the uniform is not valid (compile/link failed?).
            if(uniformNameLength <= 0) { continue; }

            // For texture samplers, you must tell OpenGL which "texture unit" to use.
            // If the shader only uses one texture sampler, this works automatically.
            // But you must manually specify the unit if more than one texture is used.
            if(uniformType == GL_SAMPLER_2D)
            {
                SetShaderUniformInt(reinterpret_cast<ShaderHandle>(program), uniformNameBuffer, textureUnitCounter);
                ++textureUnitCounter;
            }
        }
    }

    // Finally return the shader handle.
    return reinterpret_cast<ShaderHandle>(program);
}

void GAPI_OpenGL::DestroyShader(ShaderHandle handle)
{
    glDeleteProgram(reinterpret_cast<uintptr_t>(handle));
}

void GAPI_OpenGL::ActivateShader(ShaderHandle handle)
{
    glUseProgram(reinterpret_cast<uintptr_t>(handle));
}

void GAPI_OpenGL::SetShaderUniformInt(ShaderHandle handle, const char* name, int value)
{
    GLuint program = reinterpret_cast<uintptr_t>(handle);
    if(program != GL_NONE)
    {
        GLint loc = glGetUniformLocation(program, name);
        if(loc >= 0)
        {
            glUniform1i(loc, value);
        }
    }
}

void GAPI_OpenGL::SetShaderUniformFloat(ShaderHandle handle, const char* name, float value)
{
    GLuint program = reinterpret_cast<uintptr_t>(handle);
    if(program != GL_NONE)
    {
        GLint loc = glGetUniformLocation(program, name);
        if(loc >= 0)
        {
            glUniform1f(loc, value);
        }
    }
}

void GAPI_OpenGL::SetShaderUniformVector3(ShaderHandle handle, const char* name, const Vector3& value)
{
    GLuint program = reinterpret_cast<uintptr_t>(handle);
    if(program != GL_NONE)
    {
        GLint loc = glGetUniformLocation(program, name);
        if(loc >= 0)
        {
            glUniform3f(loc, value.x, value.y, value.z);
        }
    }
}

void GAPI_OpenGL::SetShaderUniformVector4(ShaderHandle handle, const char* name, const Vector4& value)
{
    GLuint program = reinterpret_cast<uintptr_t>(handle);
    if(program != GL_NONE)
    {
        GLint loc = glGetUniformLocation(program, name);
        if(loc >= 0)
        {
            glUniform4f(loc, value.x, value.y, value.z, value.w);
        }
    }
}

void GAPI_OpenGL::SetShaderUniformMatrix4(ShaderHandle handle, const char* name, const Matrix4& mat)
{
    GLuint program = reinterpret_cast<uintptr_t>(handle);
    if(program != GL_NONE)
    {
        GLint loc = glGetUniformLocation(program, name);
        if(loc >= 0)
        {
            glUniformMatrix4fv(loc, 1, GL_FALSE, mat);
        }
    }
}

void GAPI_OpenGL::SetShaderUniformColor(ShaderHandle handle, const char* name, const Color32& color)
{
    GLuint program = reinterpret_cast<uintptr_t>(handle);
    if(program != GL_NONE)
    {
        GLint loc = glGetUniformLocation(program, name);
        if(loc >= 0)
        {
            glUniform4f(loc, color.GetR() / 255.0f, color.GetG() / 255.0f, color.GetB() / 255.0f, color.GetA() / 255.0f);
        }
    }
}

void GAPI_OpenGL::Draw(Primitive primitive, BufferHandle vertexBuffer)
{
    // Draw all vertices in the vertex buffer.
    Draw(primitive, vertexBuffer, 0, static_cast<VertexBuffer*>(vertexBuffer)->count);
}

void GAPI_OpenGL::Draw(Primitive primitive, BufferHandle vertexBuffer, uint32_t vertexOffset, uint32_t vertexCount)
{
    // Bind the VAO.
    // The VAO internally tracks which VBO it is associated with, so we don't need to bind the VBO as well.
    GLState::BindVertexArray(static_cast<VertexBuffer*>(vertexBuffer)->vao);

    // Draw "count" vertices at offset.
    glDrawArrays(PrimitiveToDrawMode(primitive), vertexOffset, vertexCount);
}

void GAPI_OpenGL::Draw(Primitive primitive, BufferHandle vertexBuffer, BufferHandle indexBuffer)
{
    // Draw all indexes in the index buffer.
    Draw(primitive, vertexBuffer, indexBuffer, 0, static_cast<IndexBuffer*>(indexBuffer)->count);
}

void GAPI_OpenGL::Draw(Primitive primitive, BufferHandle vertexBuffer, BufferHandle indexBuffer, uint32_t indexOffset, uint32_t indexCount)
{
    // Bind the VAO.
    // The VAO internally tracks which VBO it is associated with, so we don't need to bind the VBO as well.
    GLState::BindVertexArray(static_cast<VertexBuffer*>(vertexBuffer)->vao);

    // Bind the VAO, since we're drawing with an index buffer here.
    GLState::BindIndexBuffer(static_cast<IndexBuffer*>(indexBuffer)->ibo);

    // Draw "count" indices at offset.
    glDrawElements(PrimitiveToDrawMode(primitive), indexCount, GL_UNSIGNED_SHORT, BUFFER_OFFSET(indexOffset * sizeof(GLushort)));
}