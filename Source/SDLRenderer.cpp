//
//  SDLRenderer.cpp
//  GEngine
//
//  Created by Clark Kromenaker on 7/22/17.
//
#include "SDLRenderer.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include "Matrix4.h"
#include "Model.h"
#include "CameraComponent.h"
#include "MeshComponent.h"

GLfloat triangle_vertices[] = {
    0.0f,  0.5f,  0.0f,
    0.5f, -0.5f,  0.0f,
    -0.5f, -0.5f, 0.0f
};

GLfloat triange_colors[] {
    1.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f
};

GLfloat cube_vertices[] = {
    // front
    -0.5, -0.5,  0.5,
    0.5, -0.5, 0.5,
    0.5,  0.5,  0.5,
    -0.5,  0.5,  0.5,
    // back
    -0.5, -0.5, -0.5,
    0.5, -0.5, -0.5,
    0.5,  0.5, -0.5,
    -0.5,  0.5, -0.5
};

GLfloat cube_colors[] = {
    // front colors
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    1.0, 1.0, 1.0,
    // back colors
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0,
    1.0, 1.0, 1.0,
};

GLushort cube_elements[] = {
    // front
    0, 1, 2,
    2, 3, 0,
    // top
    1, 5, 6,
    6, 2, 1,
    // back
    7, 6, 5,
    5, 4, 7,
    // bottom
    4, 0, 3,
    3, 7, 4,
    // left
    4, 5, 1,
    1, 0, 4,
    // right
    3, 2, 6,
    6, 7, 3,
};

bool SDLRenderer::Initialize()
{
    // Init video subsystem.
    if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
    {
        return false;
    }
    
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
    
    // Create a window.
    mWindow = SDL_CreateWindow("GK3", 100, 100, 1024, 768, SDL_WINDOW_OPENGL);
    if(!mWindow) { return false; }
    
    // Create OpenGL context.
    mContext = SDL_GL_CreateContext(mWindow);
    
    // Initialize GLEW.
    glewExperimental = GL_TRUE;
    if(glewInit() != GLEW_OK)
    {
        SDL_Log("Failed to initialize GLEW.");
        return false;
    }
    
    // Clear any GLEW error.
    glGetError();
    
    // Initialize frame buffer.
    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    mShader = new GLShader("Assets/Tut.vert", "Assets/Tut.frag");
    if(!mShader->IsGood()) { return false; }
    
    mVertArray = new GLVertexArray(triangle_vertices, 9);
    //mVertArray = new GLVertexArray(cube_vertices, 24, cube_elements, 36);
    
    // Init succeeded!
    return true;
}

void SDLRenderer::Shutdown()
{
    delete mVertArray;
    delete mShader;
    
    SDL_GL_DeleteContext(mContext);
    SDL_DestroyWindow(mWindow);
    SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

void SDLRenderer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void SDLRenderer::Render()
{
    // Activate, for now, our one and only shader.
    mShader->Activate();
    
    // Set the combined view/projection matrix based on the assigned camera.
    Matrix4 viewProj;
    if(mCameraComponent != nullptr)
    {
        viewProj = mCameraComponent->GetProjectionMatrix() * mCameraComponent->GetLookAtMatrix();
    }
    GLuint view = glGetUniformLocation(mShader->GetProgram(), "uViewProj");
    glUniformMatrix4fv(view, 1, GL_FALSE, viewProj.GetFloatPtr());
    
    // Render all mesh components.
    for(auto meshComponent : mMeshComponents)
    {
        meshComponent->Render();
    }
    
    /*
    // DEFAULT RENDERING: Set identity world transform (so at origin)
    // and draw a thingy there (triangle).
    Matrix4 worldTransform;
    SetWorldTransformMatrix(worldTransform);
    if(mVertArray != nullptr)
    {
        mVertArray->Draw();
    }
    */
}

void SDLRenderer::Present()
{
    SDL_GL_SwapWindow(mWindow);
}

void SDLRenderer::SetWorldTransformMatrix(Matrix4& worldTransform)
{
    GLuint world = glGetUniformLocation(mShader->GetProgram(), "uWorldTransform");
    glUniformMatrix4fv(world, 1, GL_FALSE, worldTransform.GetFloatPtr());
}

void SDLRenderer::AddMeshComponent(MeshComponent *mc)
{
    mMeshComponents.push_back(mc);
}

void SDLRenderer::RemoveMeshComponent(MeshComponent *mc)
{
    auto it = std::find(mMeshComponents.begin(), mMeshComponents.end(), mc);
    if(it != mMeshComponents.end())
    {
        mMeshComponents.erase(it);
    }
}

void SDLRenderer::SetModel(Model *model)
{
    if(mVertArray != nullptr)
    {
        delete mVertArray;
        mVertArray = nullptr;
    }
    
    mModel = model;
    if(mModel != nullptr)
    {
        mVertArray = new GLVertexArray(mModel->GetVertexPositions(), mModel->GetVertexCount(),
                                       mModel->GetIndexes(), mModel->GetIndexCount());
    }
}
