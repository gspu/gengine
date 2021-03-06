//
// GLVertexArray.cpp
//
// Clark Kromenaker
//
#include "GLVertexArray.h"

// Some OpenGL calls take in array indexes/offsets as pointers.
// This macro just makes the syntax clearer for the reader.
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

GLVertexArray::GLVertexArray(unsigned int vertexCount, unsigned int vertexSize, MeshUsage usage) :
    mVertexCount(vertexCount)
{
    // Generate and bind VBO.
    glGenBuffers(1, &mVBO);
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    
    // Allocate VBO of needed size, but don't fill with data yet.
    GLenum glUsage = (usage == MeshUsage::Static) ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW;
    glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, NULL, glUsage);
}

GLVertexArray::~GLVertexArray()
{
    glDeleteBuffers(1, &mVBO);
    glDeleteVertexArrays(1, &mVAO);
    glDeleteBuffers(1, &mIBO);
}

void GLVertexArray::SetPositions(float* positions)
{
    mPositions = positions;
    mVboUpdateMask |= 1;
}

void GLVertexArray::SetColors(float* colors)
{
    mColors = colors;
}

void GLVertexArray::SetNormals(float* normals)
{
    mNormals = normals;
}

void GLVertexArray::SetUV1(float* uvs)
{
    mUV1 = uvs;
	mVboUpdateMask |= 8;
}

void GLVertexArray::SetIndexes(unsigned short* indexes, unsigned int count)
{
    mIndexes = indexes;
    mIndexCount = count;
}

void GLVertexArray::DrawTriangles()
{
    DrawTriangles(0, mIndexes != nullptr ? mIndexCount : mVertexCount);
}

void GLVertexArray::DrawTriangles(unsigned int offset, unsigned int count)
{
    Draw(GL_TRIANGLES, offset, count);
}

void GLVertexArray::DrawTriangleStrips()
{
	DrawTriangleStrips(0, mIndexes != nullptr ? mIndexCount : mVertexCount);
}

void GLVertexArray::DrawTriangleStrips(unsigned int offset, unsigned int count)
{
	Draw(GL_TRIANGLE_STRIP, offset, count);
}

void GLVertexArray::DrawTriangleFans()
{
    DrawTriangleFans(0, mIndexes != nullptr ? mIndexCount : mVertexCount);
}

void GLVertexArray::DrawTriangleFans(unsigned int offset, unsigned int count)
{
    Draw(GL_TRIANGLE_FAN, offset, count);
}

void GLVertexArray::DrawLines()
{
    DrawLines(0, mIndexes != nullptr ? mIndexCount : mVertexCount);
}

void GLVertexArray::DrawLines(unsigned int offset, unsigned int count)
{
    Draw(GL_LINES, offset, count);
}

void GLVertexArray::Draw(GLenum mode)
{
    Draw(mode, 0, mIndexes != nullptr ? mIndexCount : mVertexCount);
}

void GLVertexArray::Draw(GLenum mode, unsigned int offset, unsigned int count)
{
    // If not yet built, build!
    if(mVAO == GL_NONE)
    {
        BuildVBO();
    }
    
    // If need to update the VBO, let's do it.
    if(mVboUpdateMask != 0)
    {
        UpdateVBO();
    }
	
	// If no per-vertex colors are defined, use a default of white.
	if(mColors == nullptr)
	{
		glVertexAttrib4f(1, 1.0f, 1.0f, 1.0f, 1.0f);
	}
    
    // Bind vertex array and buffer.
    // It's OK if the mIBO is NONE here - will have no effect.
    glBindVertexArray(mVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
    
    // Draw method depends on whether we have indexes or not.
    if(mIBO != GL_NONE)
    {
        // Draw "count" indices at offset.
        glDrawElements(mode, count, GL_UNSIGNED_SHORT, BUFFER_OFFSET(offset * sizeof(GLushort)));
    }
    else
    {
        // Draw "count" triangles at offset.
        glDrawArrays(mode, offset, count);
    }
}

void GLVertexArray::BuildVBO()
{
    // Generate VAO object.
    if(mVAO != GL_NONE) { return; }
    glGenVertexArrays(1, &mVAO);
    
    // Bind VBO - we'll be changing it's data.
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    
    // Bind VAO - we'll possibly change vertex attribute layout.
    glBindVertexArray(mVAO);
    
    // Put position data in the VBO.
    // Map position data location in the VAO.
    int offset = 0;
    int positionSize = mVertexCount * 3 * sizeof(GLfloat);
    glBufferSubData(GL_ARRAY_BUFFER, offset, positionSize, mPositions);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(offset));
    offset += positionSize;
    
    // Put color data in VBO and map in VAO.
    if(mColors != nullptr)
    {
        int colorSize = mVertexCount * 4 * sizeof(GLfloat);
        glBufferSubData(GL_ARRAY_BUFFER, offset, colorSize, mColors);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_TRUE, 0, BUFFER_OFFSET(offset));
        offset += colorSize;
    }
    
    // Put normal data in VBO and map in VAO.
    if(mNormals != nullptr)
    {
        int normalsSize = mVertexCount * 3 * sizeof(GLfloat);
        glBufferSubData(GL_ARRAY_BUFFER, offset, normalsSize, mNormals);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_TRUE, 0, BUFFER_OFFSET(offset));
        offset += normalsSize;
    }
    
    // Put UV data in VBO and map in VAO.
    if(mUV1 != nullptr)
    {
        int uvSize = mVertexCount * 2 * sizeof(GLfloat);
        glBufferSubData(GL_ARRAY_BUFFER, offset, uvSize, mUV1);
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 2, GL_FLOAT, GL_TRUE, 0, BUFFER_OFFSET(offset));
        offset += uvSize;
    }
    
    // Also build the index buffer object if needed.
    if(mIndexes != nullptr)
    {
        glGenBuffers(1, &mIBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexCount * sizeof(GLushort), mIndexes, GL_STATIC_DRAW);
    }
	
	// Clear update mask, if any, since we definitely just updated.
	mVboUpdateMask = 0;
}

void GLVertexArray::UpdateVBO()
{
    // No updates needed!
    if(mVboUpdateMask == 0) { return; }
    
    // Bind VBO - we'll be changing it's data.
    glBindBuffer(GL_ARRAY_BUFFER, mVBO);
    
    // Check and update position data.
    int offset = 0;
    int positionSize = mVertexCount * 3 * sizeof(GLfloat);
    if((mVboUpdateMask & 1) != 0)
    {
        glBufferSubData(GL_ARRAY_BUFFER, offset, positionSize, mPositions);
    }
    offset += positionSize;
	
	// Check and update color data.
	if(mColors != nullptr)
	{
		int colorSize = mVertexCount * 4 * sizeof(GLfloat);
		if((mVboUpdateMask & 2) != 0)
		{
			glBufferSubData(GL_ARRAY_BUFFER, offset, colorSize, mColors);
		}
		offset += colorSize;
	}
	
	// Check and update normal data.
	if(mNormals != nullptr)
	{
		int normalsSize = mVertexCount * 3 * sizeof(GLfloat);
		if((mVboUpdateMask & 4) != 0)
		{
			glBufferSubData(GL_ARRAY_BUFFER, offset, normalsSize, mNormals);
		}
		offset += normalsSize;
	}
	
	// Check and update UV1 data.
	if(mUV1 != nullptr)
	{
		int uvSize = mVertexCount * 2 * sizeof(GLfloat);
		if((mVboUpdateMask & 8) != 0)
		{
			glBufferSubData(GL_ARRAY_BUFFER, offset, uvSize, mUV1);
		}
		offset += uvSize;
	}
	
	// Reset update mask.
    mVboUpdateMask = 0;
}
