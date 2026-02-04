/*
  ==============================================================================

   This file is part of the DRX framework examples.
   Copyright (c) DinrusPro

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a DRX project.

 BEGIN_DRX_PIP_METADATA

 name:             OpenGLDemo
 version:          1.0.0
 vendor:           DRX
 website:          http://drx.com
 description:      Simple 3D OpenGL application.

 dependencies:     drx_core, drx_data_structures, drx_events, drx_graphics,
                   drx_gui_basics, drx_gui_extra, drx_opengl
 exporters:        xcode_mac, vs2022, linux_make, androidstudio, xcode_iphone

 moduleFlags:      DRX_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        OpenGLDemo

 useLocalCopy:     1

 END_DRX_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"
#include "../Assets/WavefrontObjParser.h"

struct OpenGLUtils
{
    //==============================================================================
    /** Vertex data to be passed to the shaders.
        For the purposes of this demo, each vertex will have a 3D position, a colour and a
        2D texture co-ordinate. Of course you can ignore these or manipulate them in the
        shader programs but are some useful defaults to work from.
     */
    struct Vertex
    {
        f32 position[3];
        f32 normal[3];
        f32 colour[4];
        f32 texCoord[2];
    };

    //==============================================================================
    // This class just manages the attributes that the demo shaders use.
    struct Attributes
    {
        explicit Attributes (OpenGLShaderProgram& shader)
        {
            position      .reset (createAttribute (shader, "position"));
            normal        .reset (createAttribute (shader, "normal"));
            sourceColor  .reset (createAttribute (shader, "sourceColor"));
            textureCoordIn.reset (createAttribute (shader, "textureCoordIn"));
        }

        z0 enable()
        {
            using namespace ::drx::gl;

            if (position.get() != nullptr)
            {
                glVertexAttribPointer (position->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), nullptr);
                glEnableVertexAttribArray (position->attributeID);
            }

            if (normal.get() != nullptr)
            {
                glVertexAttribPointer (normal->attributeID, 3, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (f32) * 3));
                glEnableVertexAttribArray (normal->attributeID);
            }

            if (sourceColor.get() != nullptr)
            {
                glVertexAttribPointer (sourceColor->attributeID, 4, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (f32) * 6));
                glEnableVertexAttribArray (sourceColor->attributeID);
            }

            if (textureCoordIn.get() != nullptr)
            {
                glVertexAttribPointer (textureCoordIn->attributeID, 2, GL_FLOAT, GL_FALSE, sizeof (Vertex), (GLvoid*) (sizeof (f32) * 10));
                glEnableVertexAttribArray (textureCoordIn->attributeID);
            }
        }

        z0 disable()
        {
            using namespace ::drx::gl;

            if (position != nullptr)        glDisableVertexAttribArray (position->attributeID);
            if (normal != nullptr)          glDisableVertexAttribArray (normal->attributeID);
            if (sourceColor != nullptr)    glDisableVertexAttribArray (sourceColor->attributeID);
            if (textureCoordIn != nullptr)  glDisableVertexAttribArray (textureCoordIn->attributeID);
        }

        std::unique_ptr<OpenGLShaderProgram::Attribute> position, normal, sourceColor, textureCoordIn;

    private:
        static OpenGLShaderProgram::Attribute* createAttribute (OpenGLShaderProgram& shader,
                                                                tukk attributeName)
        {
            using namespace ::drx::gl;

            if (glGetAttribLocation (shader.getProgramID(), attributeName) < 0)
                return nullptr;

            return new OpenGLShaderProgram::Attribute (shader, attributeName);
        }
    };

    //==============================================================================
    // This class just manages the uniform values that the demo shaders use.
    struct Uniforms
    {
        explicit Uniforms (OpenGLShaderProgram& shader)
        {
            projectionMatrix.reset (createUniform (shader, "projectionMatrix"));
            viewMatrix      .reset (createUniform (shader, "viewMatrix"));
            texture         .reset (createUniform (shader, "demoTexture"));
            lightPosition   .reset (createUniform (shader, "lightPosition"));
            bouncingNumber  .reset (createUniform (shader, "bouncingNumber"));
        }

        std::unique_ptr<OpenGLShaderProgram::Uniform> projectionMatrix, viewMatrix, texture, lightPosition, bouncingNumber;

    private:
        static OpenGLShaderProgram::Uniform* createUniform (OpenGLShaderProgram& shader,
                                                            tukk uniformName)
        {
            using namespace ::drx::gl;

            if (glGetUniformLocation (shader.getProgramID(), uniformName) < 0)
                return nullptr;

            return new OpenGLShaderProgram::Uniform (shader, uniformName);
        }
    };

    //==============================================================================
    /** This loads a 3D model from an OBJ file and converts it into some vertex buffers
        that we can draw.
    */
    struct Shape
    {
        Shape()
        {
            if (shapeFile.load (loadEntireAssetIntoString ("teapot.obj")).wasOk())
                for (auto* s : shapeFile.shapes)
                    vertexBuffers.add (new VertexBuffer (*s));
        }

        z0 draw (Attributes& attributes)
        {
            using namespace ::drx::gl;

            for (auto* vertexBuffer : vertexBuffers)
            {
                vertexBuffer->bind();

                attributes.enable();
                glDrawElements (GL_TRIANGLES, vertexBuffer->numIndices, GL_UNSIGNED_INT, nullptr);
                attributes.disable();
            }
        }

    private:
        struct VertexBuffer
        {
            explicit VertexBuffer (WavefrontObjFile::Shape& shape)
            {
                using namespace ::drx::gl;

                numIndices = shape.mesh.indices.size();

                glGenBuffers (1, &vertexBuffer);
                glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);

                Array<Vertex> vertices;
                createVertexListFromMesh (shape.mesh, vertices, Colors::green);

                glBufferData (GL_ARRAY_BUFFER, vertices.size() * (i32) sizeof (Vertex),
                              vertices.getRawDataPointer(), GL_STATIC_DRAW);

                glGenBuffers (1, &indexBuffer);
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                glBufferData (GL_ELEMENT_ARRAY_BUFFER, numIndices * (i32) sizeof (drx::u32),
                                                       shape.mesh.indices.getRawDataPointer(), GL_STATIC_DRAW);
            }

            ~VertexBuffer()
            {
                using namespace ::drx::gl;

                glDeleteBuffers (1, &vertexBuffer);
                glDeleteBuffers (1, &indexBuffer);
            }

            z0 bind()
            {
                using namespace ::drx::gl;

                glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
                glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
            }

            GLuint vertexBuffer, indexBuffer;
            i32 numIndices;

            DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VertexBuffer)
        };

        WavefrontObjFile shapeFile;
        OwnedArray<VertexBuffer> vertexBuffers;

        static z0 createVertexListFromMesh (const WavefrontObjFile::Mesh& mesh, Array<Vertex>& list, Color colour)
        {
            auto scale = 0.2f;
            WavefrontObjFile::TextureCoord defaultTexCoord = { 0.5f, 0.5f };
            WavefrontObjFile::Vertex defaultNormal = { 0.5f, 0.5f, 0.5f };

            for (i32 i = 0; i < mesh.vertices.size(); ++i)
            {
                auto& v = mesh.vertices.getReference (i);

                auto& n = (i < mesh.normals.size() ? mesh.normals.getReference (i)
                                                   : defaultNormal);

                auto& tc = (i < mesh.textureCoords.size() ? mesh.textureCoords.getReference (i)
                                                          : defaultTexCoord);

                list.add ({ { scale * v.x, scale * v.y, scale * v.z, },
                            { scale * n.x, scale * n.y, scale * n.z, },
                            { colour.getFloatRed(), colour.getFloatGreen(), colour.getFloatBlue(), colour.getFloatAlpha() },
                            { tc.x, tc.y } });
            }
        }
    };

    //==============================================================================
    struct ShaderPreset
    {
        tukk name;
        tukk vertexShader;
        tukk fragmentShader;
    };

    static Array<ShaderPreset> getPresets()
    {
        #define SHADER_DEMO_HEADER \
            "/*  This is a live OpenGL Shader demo.\n" \
            "    Edit the shader program below and it will be \n" \
            "    compiled and applied to the model above!\n" \
            "*/\n\n"

        ShaderPreset presets[] =
        {
            {
                "Texture + Lighting",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "attribute vec4 sourceColor;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying vec4 destinationColor;\n"
                "varying vec2 textureCoordOut;\n"
                "varying f32 lightIntensity;\n"
                "\n"
                "z0 main()\n"
                "{\n"
                "    destinationColor = sourceColor;\n"
                "    textureCoordOut = textureCoordIn;\n"
                "\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if DRX_OPENGL_ES
                "varying lowp vec4 destinationColor;\n"
                "varying lowp vec2 textureCoordOut;\n"
                "varying highp f32 lightIntensity;\n"
               #else
                "varying vec4 destinationColor;\n"
                "varying vec2 textureCoordOut;\n"
                "varying f32 lightIntensity;\n"
               #endif
                "\n"
                "uniform sampler2D demoTexture;\n"
                "\n"
                "z0 main()\n"
                "{\n"
               #if DRX_OPENGL_ES
                "   highp f32 l = max (0.3, lightIntensity * 0.3);\n"
                "   highp vec4 colour = vec4 (l, l, l, 1.0);\n"
               #else
                "   f32 l = max (0.3, lightIntensity * 0.3);\n"
                "   vec4 colour = vec4 (l, l, l, 1.0);\n"
               #endif
                "    gl_FragColor = colour * texture2D (demoTexture, textureCoordOut);\n"
                "}\n"
            },

            {
                "Textured",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 sourceColor;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "\n"
                "varying vec4 destinationColor;\n"
                "varying vec2 textureCoordOut;\n"
                "\n"
                "z0 main()\n"
                "{\n"
                "    destinationColor = sourceColor;\n"
                "    textureCoordOut = textureCoordIn;\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if DRX_OPENGL_ES
                "varying lowp vec4 destinationColor;\n"
                "varying lowp vec2 textureCoordOut;\n"
               #else
                "varying vec4 destinationColor;\n"
                "varying vec2 textureCoordOut;\n"
               #endif
                "\n"
                "uniform sampler2D demoTexture;\n"
                "\n"
                "z0 main()\n"
                "{\n"
                "    gl_FragColor = texture2D (demoTexture, textureCoordOut);\n"
                "}\n"
            },

            {
                "Flat Color",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 sourceColor;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "\n"
                "varying vec4 destinationColor;\n"
                "varying vec2 textureCoordOut;\n"
                "\n"
                "z0 main()\n"
                "{\n"
                "    destinationColor = sourceColor;\n"
                "    textureCoordOut = textureCoordIn;\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if DRX_OPENGL_ES
                "varying lowp vec4 destinationColor;\n"
                "varying lowp vec2 textureCoordOut;\n"
               #else
                "varying vec4 destinationColor;\n"
                "varying vec2 textureCoordOut;\n"
               #endif
                "\n"
                "z0 main()\n"
                "{\n"
                "    gl_FragColor = destinationColor;\n"
                "}\n"
            },

            {
                "Rainbow",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 sourceColor;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "\n"
                "varying vec4 destinationColor;\n"
                "varying vec2 textureCoordOut;\n"
                "\n"
                "varying f32 xPos;\n"
                "varying f32 yPos;\n"
                "varying f32 zPos;\n"
                "\n"
                "z0 main()\n"
                "{\n"
                "    vec4 v = vec4 (position);\n"
                "    xPos = clamp (v.x, 0.0, 1.0);\n"
                "    yPos = clamp (v.y, 0.0, 1.0);\n"
                "    zPos = clamp (v.z, 0.0, 1.0);\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}",

                SHADER_DEMO_HEADER
               #if DRX_OPENGL_ES
                "varying lowp vec4 destinationColor;\n"
                "varying lowp vec2 textureCoordOut;\n"
                "varying lowp f32 xPos;\n"
                "varying lowp f32 yPos;\n"
                "varying lowp f32 zPos;\n"
               #else
                "varying vec4 destinationColor;\n"
                "varying vec2 textureCoordOut;\n"
                "varying f32 xPos;\n"
                "varying f32 yPos;\n"
                "varying f32 zPos;\n"
               #endif
                "\n"
                "z0 main()\n"
                "{\n"
                "    gl_FragColor = vec4 (xPos, yPos, zPos, 1.0);\n"
                "}"
            },

            {
                "Changing Color",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec2 textureCoordIn;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "\n"
                "varying vec2 textureCoordOut;\n"
                "\n"
                "z0 main()\n"
                "{\n"
                "    textureCoordOut = textureCoordIn;\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
                "#define PI 3.1415926535897932384626433832795\n"
                "\n"
               #if DRX_OPENGL_ES
                "precision mediump f32;\n"
                "varying lowp vec2 textureCoordOut;\n"
               #else
                "varying vec2 textureCoordOut;\n"
               #endif
                "uniform f32 bouncingNumber;\n"
                "\n"
                "z0 main()\n"
                "{\n"
                "   f32 b = bouncingNumber;\n"
                "   f32 n = b * PI * 2.0;\n"
                "   f32 sn = (sin (n * textureCoordOut.x) * 0.5) + 0.5;\n"
                "   f32 cn = (sin (n * textureCoordOut.y) * 0.5) + 0.5;\n"
                "\n"
                "   vec4 col = vec4 (b, sn, cn, 1.0);\n"
                "   gl_FragColor = col;\n"
                "}\n"
            },

            {
                "Simple Light",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying f32 lightIntensity;\n"
                "\n"
                "z0 main()\n"
                "{\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if DRX_OPENGL_ES
                "varying highp f32 lightIntensity;\n"
               #else
                "varying f32 lightIntensity;\n"
               #endif
                "\n"
                "z0 main()\n"
                "{\n"
               #if DRX_OPENGL_ES
                "   highp f32 l = lightIntensity * 0.25;\n"
                "   highp vec4 colour = vec4 (l, l, l, 1.0);\n"
               #else
                "   f32 l = lightIntensity * 0.25;\n"
                "   vec4 colour = vec4 (l, l, l, 1.0);\n"
               #endif
                "\n"
                "    gl_FragColor = colour;\n"
                "}\n"
            },

            {
                "Flattened",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying f32 lightIntensity;\n"
                "\n"
                "z0 main()\n"
                "{\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "\n"
                "    vec4 v = vec4 (position);\n"
                "    v.z = v.z * 0.1;\n"
                "\n"
                "    gl_Position = projectionMatrix * viewMatrix * v;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if DRX_OPENGL_ES
                "varying highp f32 lightIntensity;\n"
               #else
                "varying f32 lightIntensity;\n"
               #endif
                "\n"
                "z0 main()\n"
                "{\n"
               #if DRX_OPENGL_ES
                "   highp f32 l = lightIntensity * 0.25;\n"
                "   highp vec4 colour = vec4 (l, l, l, 1.0);\n"
               #else
                "   f32 l = lightIntensity * 0.25;\n"
                "   vec4 colour = vec4 (l, l, l, 1.0);\n"
               #endif
                "\n"
                "    gl_FragColor = colour;\n"
                "}\n"
            },

            {
                "Toon Shader",

                SHADER_DEMO_HEADER
                "attribute vec4 position;\n"
                "attribute vec4 normal;\n"
                "\n"
                "uniform mat4 projectionMatrix;\n"
                "uniform mat4 viewMatrix;\n"
                "uniform vec4 lightPosition;\n"
                "\n"
                "varying f32 lightIntensity;\n"
                "\n"
                "z0 main()\n"
                "{\n"
                "    vec4 light = viewMatrix * lightPosition;\n"
                "    lightIntensity = dot (light, normal);\n"
                "\n"
                "    gl_Position = projectionMatrix * viewMatrix * position;\n"
                "}\n",

                SHADER_DEMO_HEADER
               #if DRX_OPENGL_ES
                "varying highp f32 lightIntensity;\n"
               #else
                "varying f32 lightIntensity;\n"
               #endif
                "\n"
                "z0 main()\n"
                "{\n"
               #if DRX_OPENGL_ES
                "    highp f32 intensity = lightIntensity * 0.5;\n"
                "    highp vec4 colour;\n"
               #else
                "    f32 intensity = lightIntensity * 0.5;\n"
                "    vec4 colour;\n"
               #endif
                "\n"
                "    if (intensity > 0.95)\n"
                "        colour = vec4 (1.0, 0.5, 0.5, 1.0);\n"
                "    else if (intensity > 0.5)\n"
                "        colour  = vec4 (0.6, 0.3, 0.3, 1.0);\n"
                "    else if (intensity > 0.25)\n"
                "        colour  = vec4 (0.4, 0.2, 0.2, 1.0);\n"
                "    else\n"
                "        colour  = vec4 (0.2, 0.1, 0.1, 1.0);\n"
                "\n"
                "    gl_FragColor = colour;\n"
                "}\n"
            }
        };

        return Array<ShaderPreset> (presets, numElementsInArray (presets));
    }

    //==============================================================================
    // These classes are used to load textures from the various sources that the demo uses..
    struct DemoTexture
    {
        virtual ~DemoTexture() {}
        virtual b8 applyTo (OpenGLTexture&) = 0;

        Txt name;
    };

    struct DynamicTexture final : public DemoTexture
    {
        DynamicTexture() { name = "Dynamically-generated texture"; }

        Image image;
        BouncingNumber x, y;

        b8 applyTo (OpenGLTexture& texture) override
        {
            i32 size = 128;

            if (! image.isValid())
                image = Image (Image::ARGB, size, size, true);

            {
                Graphics g (image);
                g.fillAll (Colors::lightcyan);

                g.setColor (Colors::darkred);
                g.drawRect (0, 0, size, size, 2);

                g.setColor (Colors::green);
                g.fillEllipse (x.getValue() * (f32) size * 0.9f,
                               y.getValue() * (f32) size * 0.9f,
                               (f32) size * 0.1f,
                               (f32) size * 0.1f);

                g.setColor (Colors::black);
                g.setFont (40);

                g.drawFittedText (Txt (Time::getCurrentTime().getMilliseconds()), image.getBounds(), Justification::centred, 1);
            }

            texture.loadImage (image);
            return true;
        }
    };

    static Image resizeImageToPowerOfTwo (Image image)
    {
        if (! (isPowerOfTwo (image.getWidth()) && isPowerOfTwo (image.getHeight())))
            return image.rescaled (jmin (1024, nextPowerOfTwo (image.getWidth())),
                                   jmin (1024, nextPowerOfTwo (image.getHeight())));

        return image;
    }

    struct BuiltInTexture final : public DemoTexture
    {
        BuiltInTexture (tukk nm, ukk imageData, size_t imageSize)
            : image (resizeImageToPowerOfTwo (ImageFileFormat::loadFrom (imageData, imageSize)))
        {
            name = nm;
        }

        Image image;

        b8 applyTo (OpenGLTexture& texture) override
        {
            texture.loadImage (image);
            return false;
        }
    };

    struct TextureFromFile final : public DemoTexture
    {
        TextureFromFile (const File& file)
        {
            name = file.getFileName();
            image = resizeImageToPowerOfTwo (ImageFileFormat::loadFrom (file));
        }

        Image image;

        b8 applyTo (OpenGLTexture& texture) override
        {
            texture.loadImage (image);
            return false;
        }
    };

    struct TextureFromAsset final : public DemoTexture
    {
        TextureFromAsset (tukk assetName)
        {
            name = assetName;
            image = resizeImageToPowerOfTwo (getImageFromAssets (assetName));
        }

        Image image;

        b8 applyTo (OpenGLTexture& texture) override
        {
            texture.loadImage (image);
            return false;
        }
    };
};

//==============================================================================
/** This is the main demo component - the GL context gets attached to it, and
    it implements the OpenGLRenderer callback so that it can do real GL work.
*/
class OpenGLDemo final : public Component,
                         private OpenGLRenderer,
                         private AsyncUpdater
{
public:
    OpenGLDemo()
    {
        if (auto* peer = getPeer())
            peer->setCurrentRenderingEngine (0);

        setOpaque (true);
        controlsOverlay.reset (new DemoControlsOverlay (*this));
        addAndMakeVisible (controlsOverlay.get());

        openGLContext.setOpenGLVersionRequired (OpenGLContext::openGL3_2);
        openGLContext.setRenderer (this);
        openGLContext.attachTo (*this);
        openGLContext.setContinuousRepainting (true);

        controlsOverlay->initialise();

        setSize (500, 500);
    }

    ~OpenGLDemo() override
    {
        openGLContext.detach();
    }

    z0 newOpenGLContextCreated() override
    {
        // nothing to do in this case - we'll initialise our shaders + textures
        // on demand, during the render callback.
        freeAllContextObjects();

        if (controlsOverlay != nullptr)
            controlsOverlay->updateShader();
    }

    z0 openGLContextClosing() override
    {
        // When the context is about to close, you must use this callback to delete
        // any GPU resources while the context is still current.
        freeAllContextObjects();

        if (lastTexture != nullptr)
            setTexture (lastTexture);
    }

    z0 freeAllContextObjects()
    {
        shape     .reset();
        shader    .reset();
        attributes.reset();
        uniforms  .reset();
        texture   .release();
    }

    // This is a virtual method in OpenGLRenderer, and is called when it's time
    // to do your GL rendering.
    z0 renderOpenGL() override
    {
        using namespace ::drx::gl;

        const ScopedLock lock (mutex);

        jassert (OpenGLHelpers::isContextActive());

        auto desktopScale = (f32) openGLContext.getRenderingScale();

        OpenGLHelpers::clear (getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                                      Colors::lightblue));

        if (textureToUse != nullptr)
            if (! textureToUse->applyTo (texture))
                textureToUse = nullptr;

        // First draw our background graphics to demonstrate the OpenGLGraphicsContext class
        if (doBackgroundDrawing)
            drawBackground2DStuff (desktopScale);

        updateShader();   // Check whether we need to compile a new shader

        if (shader.get() == nullptr)
            return;

        // Having used the drx 2D renderer, it will have messed-up a whole load of GL state, so
        // we need to initialise some important settings before doing our normal GL 3D drawing..
        glEnable (GL_DEPTH_TEST);
        glDepthFunc (GL_LESS);
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glActiveTexture (GL_TEXTURE0);

        if (! openGLContext.isCoreProfile())
            glEnable (GL_TEXTURE_2D);

        glViewport (0, 0,
                    roundToInt (desktopScale * (f32) bounds.getWidth()),
                    roundToInt (desktopScale * (f32) bounds.getHeight()));

        texture.bind();

        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

        shader->use();

        if (uniforms->projectionMatrix != nullptr)
            uniforms->projectionMatrix->setMatrix4 (getProjectionMatrix().mat, 1, false);

        if (uniforms->viewMatrix != nullptr)
            uniforms->viewMatrix->setMatrix4 (getViewMatrix().mat, 1, false);

        if (uniforms->texture != nullptr)
            uniforms->texture->set ((GLint) 0);

        if (uniforms->lightPosition != nullptr)
            uniforms->lightPosition->set (-15.0f, 10.0f, 15.0f, 0.0f);

        if (uniforms->bouncingNumber != nullptr)
            uniforms->bouncingNumber->set (bouncingNumber.getValue());

        shape->draw (*attributes);

        // Reset the element buffers so child Components draw correctly
        glBindBuffer (GL_ARRAY_BUFFER, 0);
        glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);

        if (! controlsOverlay->isMouseButtonDownThreadsafe())
            rotation += (f32) rotationSpeed;
    }

    Matrix3D<f32> getProjectionMatrix() const
    {
        const ScopedLock lock (mutex);

        auto w = 1.0f / (scale + 0.1f);
        auto h = w * bounds.toFloat().getAspectRatio (false);

        return Matrix3D<f32>::fromFrustum (-w, w, -h, h, 4.0f, 30.0f);
    }

    Matrix3D<f32> getViewMatrix() const
    {
        const ScopedLock lock (mutex);

        auto viewMatrix = Matrix3D<f32>::fromTranslation ({ 0.0f, 1.0f, -10.0f }) * draggableOrientation.getRotationMatrix();
        auto rotationMatrix = Matrix3D<f32>::rotation ({ rotation, rotation, -0.3f });

        return viewMatrix * rotationMatrix;
    }

    z0 setTexture (OpenGLUtils::DemoTexture* t)
    {
        lastTexture = textureToUse = t;
    }

    z0 setShaderProgram (const Txt& vertexShader, const Txt& fragmentShader)
    {
        const ScopedLock lock (shaderMutex); // Prevent concurrent access to shader strings and status
        newVertexShader = vertexShader;
        newFragmentShader = fragmentShader;
    }

    z0 paint (Graphics&) override {}

    z0 resized() override
    {
        const ScopedLock lock (mutex);

        bounds = getLocalBounds();
        controlsOverlay->setBounds (bounds);
        draggableOrientation.setViewport (bounds);
    }

    Rectangle<i32> bounds;
    Draggable3DOrientation draggableOrientation;
    b8 doBackgroundDrawing = false;
    f32 scale = 0.5f, rotationSpeed = 0.0f;
    BouncingNumber bouncingNumber;
    CriticalSection mutex;

private:
    z0 handleAsyncUpdate() override
    {
        const ScopedLock lock (shaderMutex); // Prevent concurrent access to shader strings and status
        controlsOverlay->statusLabel.setText (statusText, dontSendNotification);
    }

    z0 drawBackground2DStuff (f32 desktopScale)
    {
        // Create an OpenGLGraphicsContext that will draw into this GL window..
        std::unique_ptr<LowLevelGraphicsContext> glRenderer (createOpenGLGraphicsContext (openGLContext,
                                                                                          roundToInt (desktopScale * (f32) bounds.getWidth()),
                                                                                          roundToInt (desktopScale * (f32) bounds.getHeight())));

        if (glRenderer.get() != nullptr)
        {
            Graphics g (*glRenderer);
            g.addTransform (AffineTransform::scale (desktopScale));

            for (const auto& s : stars)
            {
                auto size = 0.25f;

                // This stuff just creates a spinning star shape and fills it..
                Path p;
                p.addStar ({ (f32) bounds.getWidth()  * s.x.getValue(),
                             (f32) bounds.getHeight() * s.y.getValue() },
                           7,
                           (f32) bounds.getHeight() * size * 0.5f,
                           (f32) bounds.getHeight() * size,
                           s.angle.getValue());

                auto hue = s.hue.getValue();

                g.setGradientFill (ColorGradient (Colors::green.withRotatedHue (hue).withAlpha (0.8f),
                                                   0, 0,
                                                   Colors::red.withRotatedHue (hue).withAlpha (0.5f),
                                                   0, (f32) bounds.getHeight(), false));
                g.fillPath (p);
            }
        }
    }

    OpenGLContext openGLContext;

    //==============================================================================
    /**
        This component sits on top of the main GL demo, and contains all the sliders
        and widgets that control things.
    */
    class DemoControlsOverlay final : public Component,
                                      private CodeDocument::Listener,
                                      private Slider::Listener,
                                      private Timer
    {
    public:
        DemoControlsOverlay (OpenGLDemo& d)
            : demo (d)
        {
            addAndMakeVisible (statusLabel);
            statusLabel.setJustificationType (Justification::topLeft);
            statusLabel.setFont (FontOptions (14.0f));

            addAndMakeVisible (sizeSlider);
            sizeSlider.setRange (0.0, 1.0, 0.001);
            sizeSlider.addListener (this);

            addAndMakeVisible (zoomLabel);
            zoomLabel.attachToComponent (&sizeSlider, true);

            addAndMakeVisible (speedSlider);
            speedSlider.setRange (0.0, 0.5, 0.001);
            speedSlider.addListener (this);
            speedSlider.setSkewFactor (0.5f);

            addAndMakeVisible (speedLabel);
            speedLabel.attachToComponent (&speedSlider, true);

            addAndMakeVisible (showBackgroundToggle);
            showBackgroundToggle.onClick = [this] { demo.doBackgroundDrawing = showBackgroundToggle.getToggleState(); };

            addAndMakeVisible (tabbedComp);
            tabbedComp.setTabBarDepth (25);
            tabbedComp.setColor (TabbedButtonBar::tabTextColorId, Colors::grey);
            tabbedComp.addTab ("Vertex", Colors::transparentBlack, &vertexEditorComp, false);
            tabbedComp.addTab ("Fragment", Colors::transparentBlack, &fragmentEditorComp, false);

            vertexDocument.addListener (this);
            fragmentDocument.addListener (this);

            textures.add (new OpenGLUtils::TextureFromAsset ("portmeirion.jpg"));
            textures.add (new OpenGLUtils::TextureFromAsset ("tile_background.png"));
            textures.add (new OpenGLUtils::TextureFromAsset ("drx_icon.png"));
            textures.add (new OpenGLUtils::DynamicTexture());

            addAndMakeVisible (textureBox);
            textureBox.onChange = [this] { selectTexture (textureBox.getSelectedId()); };
            updateTexturesList();

            addAndMakeVisible (presetBox);
            presetBox.onChange = [this] { selectPreset (presetBox.getSelectedItemIndex()); };

            auto presets = OpenGLUtils::getPresets();

            for (i32 i = 0; i < presets.size(); ++i)
                presetBox.addItem (presets[i].name, i + 1);

            addAndMakeVisible (presetLabel);
            presetLabel.attachToComponent (&presetBox, true);

            addAndMakeVisible (textureLabel);
            textureLabel.attachToComponent (&textureBox, true);
        }

        z0 initialise()
        {
            lookAndFeelChanged();

            showBackgroundToggle.setToggleState (false, sendNotification);
            textureBox.setSelectedItemIndex (0);
            presetBox .setSelectedItemIndex (0);
            speedSlider.setValue (0.01);
            sizeSlider .setValue (0.5);
        }

        z0 resized() override
        {
            auto area = getLocalBounds().reduced (4);

            auto top = area.removeFromTop (75);

            auto sliders = top.removeFromRight (area.getWidth() / 2);
            showBackgroundToggle.setBounds (sliders.removeFromBottom (25));
            speedSlider         .setBounds (sliders.removeFromBottom (25));
            sizeSlider          .setBounds (sliders.removeFromBottom (25));

            top.removeFromRight (70);
            statusLabel.setBounds (top);

            auto shaderArea = area.removeFromBottom (area.getHeight() / 2);

            auto presets = shaderArea.removeFromTop (25);
            presets.removeFromLeft (100);
            presetBox.setBounds (presets.removeFromLeft (150));
            presets.removeFromLeft (100);
            textureBox.setBounds (presets);

            shaderArea.removeFromTop (4);
            tabbedComp.setBounds (shaderArea);
        }

        b8 isMouseButtonDownThreadsafe() const { return buttonDown; }

        z0 mouseDown (const MouseEvent& e) override
        {
            const ScopedLock lock (demo.mutex);
            demo.draggableOrientation.mouseDown (e.getPosition());

            buttonDown = true;
        }

        z0 mouseDrag (const MouseEvent& e) override
        {
            const ScopedLock lock (demo.mutex);
            demo.draggableOrientation.mouseDrag (e.getPosition());
        }

        z0 mouseUp (const MouseEvent&) override
        {
            buttonDown = false;
        }

        z0 mouseWheelMove (const MouseEvent&, const MouseWheelDetails& d) override
        {
            sizeSlider.setValue (sizeSlider.getValue() + d.deltaY);
        }

        z0 mouseMagnify (const MouseEvent&, f32 magnifyAmmount) override
        {
            sizeSlider.setValue (sizeSlider.getValue() + magnifyAmmount - 1.0f);
        }

        z0 selectPreset (i32 preset)
        {
            const auto presets = OpenGLUtils::getPresets();
            const auto& p = presets[preset];

            vertexDocument  .replaceAllContent (p.vertexShader);
            fragmentDocument.replaceAllContent (p.fragmentShader);

            startTimer (1);
        }

        z0 selectTexture (i32 itemID)
        {
            if (itemID == 1000)
            {
                textureFileChooser = std::make_unique<FileChooser> ("Choose an image to open...",
                                                                    File::getSpecialLocation (File::userPicturesDirectory),
                                                                    "*.jpg;*.jpeg;*.png;*.gif");
                auto chooserFlags = FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles;

                textureFileChooser->launchAsync (chooserFlags, [this] (const FileChooser& fc)
                {
                    if (fc.getResult() == File{})
                        return;

                    textures.add (new OpenGLUtils::TextureFromFile (fc.getResult()));
                    updateTexturesList();

                    textureBox.setSelectedId (textures.size());
                });
            }
            else
            {
                if (auto* t = textures[itemID - 1])
                    demo.setTexture (t);
            }
        }

        z0 updateTexturesList()
        {
            textureBox.clear();

            for (i32 i = 0; i < textures.size(); ++i)
                textureBox.addItem (textures.getUnchecked (i)->name, i + 1);

            textureBox.addSeparator();
            textureBox.addItem ("Load from a file...", 1000);
        }

        z0 updateShader()
        {
            startTimer (10);
        }

        Label statusLabel;

    private:
        z0 sliderValueChanged (Slider*) override
        {
            const ScopedLock lock (demo.mutex);

            demo.scale         = (f32) sizeSlider .getValue();
            demo.rotationSpeed = (f32) speedSlider.getValue();
        }

        enum { shaderLinkDelay = 500 };

        z0 codeDocumentTextInserted (const Txt& /*newText*/, i32 /*insertIndex*/) override
        {
            startTimer (shaderLinkDelay);
        }

        z0 codeDocumentTextDeleted (i32 /*startIndex*/, i32 /*endIndex*/) override
        {
            startTimer (shaderLinkDelay);
        }

        z0 timerCallback() override
        {
            stopTimer();
            demo.setShaderProgram (vertexDocument  .getAllContent(),
                                   fragmentDocument.getAllContent());
        }

        z0 lookAndFeelChanged() override
        {
            auto editorBackground = getUIColorIfAvailable (LookAndFeel_V4::ColorScheme::UIColor::windowBackground,
                                                            Colors::white);

            for (i32 i = tabbedComp.getNumTabs(); i >= 0; --i)
                tabbedComp.setTabBackgroundColor (i, editorBackground);

            vertexEditorComp  .setColor (CodeEditorComponent::backgroundColorId, editorBackground);
            fragmentEditorComp.setColor (CodeEditorComponent::backgroundColorId, editorBackground);
        }

        OpenGLDemo& demo;

        Label speedLabel  { {}, "Speed:" },
              zoomLabel   { {}, "Zoom:" };

        CodeDocument vertexDocument, fragmentDocument;
        CodeEditorComponent vertexEditorComp    { vertexDocument,   nullptr },
                            fragmentEditorComp  { fragmentDocument, nullptr };

        TabbedComponent tabbedComp              { TabbedButtonBar::TabsAtLeft };

        ComboBox presetBox, textureBox;

        Label presetLabel   { {}, "Shader Preset:" },
              textureLabel  { {}, "Texture:" };

        Slider speedSlider, sizeSlider;

        ToggleButton showBackgroundToggle  { "Draw 2D graphics in background" };

        OwnedArray<OpenGLUtils::DemoTexture> textures;

        std::unique_ptr<FileChooser> textureFileChooser;

        std::atomic<b8> buttonDown { false };

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DemoControlsOverlay)
    };

    std::unique_ptr<DemoControlsOverlay> controlsOverlay;

    f32 rotation = 0.0f;

    std::unique_ptr<OpenGLShaderProgram> shader;
    std::unique_ptr<OpenGLUtils::Shape> shape;
    std::unique_ptr<OpenGLUtils::Attributes> attributes;
    std::unique_ptr<OpenGLUtils::Uniforms> uniforms;

    OpenGLTexture texture;
    OpenGLUtils::DemoTexture* textureToUse = nullptr;
    OpenGLUtils::DemoTexture* lastTexture  = nullptr;

    CriticalSection shaderMutex;
    Txt newVertexShader, newFragmentShader, statusText;

    struct BackgroundStar
    {
        SlowerBouncingNumber x, y, hue, angle;
    };

    BackgroundStar stars[3];

    //==============================================================================
    z0 updateShader()
    {
        const ScopedLock lock (shaderMutex); // Prevent concurrent access to shader strings and status

        if (newVertexShader.isNotEmpty() || newFragmentShader.isNotEmpty())
        {
            std::unique_ptr<OpenGLShaderProgram> newShader (new OpenGLShaderProgram (openGLContext));

            if (newShader->addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (newVertexShader))
                  && newShader->addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (newFragmentShader))
                  && newShader->link())
            {
                shape     .reset();
                attributes.reset();
                uniforms  .reset();

                shader.reset (newShader.release());
                shader->use();

                shape     .reset (new OpenGLUtils::Shape());
                attributes.reset (new OpenGLUtils::Attributes (*shader));
                uniforms  .reset (new OpenGLUtils::Uniforms   (*shader));

                statusText = "GLSL: v" + Txt (OpenGLShaderProgram::getLanguageVersion(), 2);
            }
            else
            {
                statusText = newShader->getLastError();
            }

            triggerAsyncUpdate();

            newVertexShader   = {};
            newFragmentShader = {};
        }
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLDemo)
};
