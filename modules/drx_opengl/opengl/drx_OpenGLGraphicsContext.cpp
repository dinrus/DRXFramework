/*
  ==============================================================================

   This file is part of the DRX framework.
   Copyright (c) DinrusPro

   DRX is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the DRX framework, or combining the
   DRX framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the DRX End User Licence
   Agreement, and all incorporated terms including the DRX Privacy Policy and
   the DRX Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the DRX
   framework to you, and you must discontinue the installation or download
   process and cease use of the DRX framework.

   DRX End User Licence Agreement: https://drx.com/legal/drx-8-licence/
   DRX Privacy Policy: https://drx.com/drx-privacy-policy
   DRX Website Terms of Service: https://drx.com/drx-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE DRX FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace drx
{

extern z0 (*clearOpenGLGlyphCache)(); // declared in drx_graphics

namespace OpenGLRendering
{

struct TextureInfo
{
    GLuint textureID;
    i32 imageWidth, imageHeight;
    f32 fullWidthProportion, fullHeightProportion;
};

//==============================================================================
// This list persists in the OpenGLContext, and will re-use cached textures which
// are created from Images.
struct CachedImageList final : public ReferenceCountedObject,
                               private ImagePixelData::Listener
{
    CachedImageList (OpenGLContext& c) noexcept
        : context (c), maxCacheSize (c.getImageCacheSize()) {}

    static CachedImageList* get (OpenGLContext& c)
    {
        const t8 cacheValueID[] = "CachedImages";
        auto list = static_cast<CachedImageList*> (c.getAssociatedObject (cacheValueID));

        if (list == nullptr)
        {
            list = new CachedImageList (c);
            c.setAssociatedObject (cacheValueID, list);
        }

        return list;
    }

    TextureInfo getTextureFor (const Image& image)
    {
        auto pixelData = image.getPixelData();
        auto* c = findCachedImage (pixelData.get());

        if (c == nullptr)
        {
            if (auto fb = OpenGLImageType::getFrameBufferFrom (image))
            {
                TextureInfo t;
                t.textureID = fb->getTextureID();
                t.imageWidth = image.getWidth();
                t.imageHeight = image.getHeight();
                t.fullWidthProportion  = 1.0f;
                t.fullHeightProportion = 1.0f;

                return t;
            }

            c = images.add (new CachedImage (*this, pixelData.get()));
            totalSize += c->imageSize;

            while (totalSize > maxCacheSize && images.size() > 1 && totalSize > 0)
                removeOldestItem();
        }

        return c->getTextureInfo();
    }

    struct CachedImage
    {
        CachedImage (CachedImageList& list, ImagePixelData* im)
            : owner (list), pixelData (im),
              lastUsed (Time::getCurrentTime()),
              imageSize ((size_t) (im->width * im->height))
        {
            pixelData->listeners.add (&owner);
        }

        ~CachedImage()
        {
            if (pixelData != nullptr)
                pixelData->listeners.remove (&owner);
        }

        TextureInfo getTextureInfo()
        {
            if (pixelData == nullptr)
                return {};

            TextureInfo t;

            if (textureNeedsReloading)
            {
                textureNeedsReloading = false;
                texture.loadImage (Image (*pixelData));
            }

            t.textureID = texture.getTextureID();
            t.imageWidth = pixelData->width;
            t.imageHeight = pixelData->height;
            t.fullWidthProportion  = (f32) t.imageWidth  / (f32) texture.getWidth();
            t.fullHeightProportion = (f32) t.imageHeight / (f32) texture.getHeight();

            lastUsed = Time::getCurrentTime();
            return t;
        }

        CachedImageList& owner;
        ImagePixelData* pixelData;
        OpenGLTexture texture;
        Time lastUsed;
        const size_t imageSize;
        b8 textureNeedsReloading = true;

        DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedImage)
    };

    using Ptr = ReferenceCountedObjectPtr<CachedImageList>;

private:
    OpenGLContext& context;
    OwnedArray<CachedImage> images;
    size_t totalSize = 0;
    const size_t maxCacheSize;

    b8 canUseContext() const noexcept
    {
        return OpenGLContext::getCurrentContext() == &context;
    }

    z0 imageDataChanged (ImagePixelData* im) override
    {
        if (auto* c = findCachedImage (im))
            c->textureNeedsReloading = true;
    }

    z0 imageDataBeingDeleted (ImagePixelData* im) override
    {
        for (i32 i = images.size(); --i >= 0;)
        {
            auto& ci = *images.getUnchecked (i);

            if (ci.pixelData == im)
            {
                if (canUseContext())
                {
                    totalSize -= ci.imageSize;
                    images.remove (i);
                }
                else
                {
                    ci.pixelData = nullptr;
                }

                break;
            }
        }
    }

    CachedImage* findCachedImage (ImagePixelData* pixelData) const
    {
        for (auto& i : images)
            if (i->pixelData == pixelData)
                return i;

        return {};
    }

    z0 removeOldestItem()
    {
        CachedImage* oldest = nullptr;

        for (auto& i : images)
            if (oldest == nullptr || i->lastUsed < oldest->lastUsed)
                oldest = i;

        if (oldest != nullptr)
        {
            totalSize -= oldest->imageSize;
            images.removeObject (oldest);
        }
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedImageList)
};


//==============================================================================
struct Target
{
    Target (OpenGLContext& c, GLuint fbID, i32 width, i32 height) noexcept
        : context (c), frameBufferID (fbID), bounds (width, height)
    {}

    Target (OpenGLContext& c, OpenGLFrameBuffer& fb, Point<i32> origin) noexcept
        : context (c), frameBufferID (fb.getFrameBufferID()),
          bounds (origin.x, origin.y, fb.getWidth(), fb.getHeight())
    {
        jassert (frameBufferID != 0); // trying to render into an uninitialised framebuffer object.
    }

    Target (const Target& other) noexcept
        : context (other.context), frameBufferID (other.frameBufferID), bounds (other.bounds)
    {}

    Target& operator= (const Target& other) noexcept
    {
        frameBufferID = other.frameBufferID;
        bounds = other.bounds;
        return *this;
    }

    z0 makeActive() const noexcept
    {
       #if DRX_WINDOWS
        if (context.extensions.glBindFramebuffer != nullptr)
       #endif
            context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, frameBufferID);

        glViewport (0, 0, bounds.getWidth(), bounds.getHeight());
        glDisable (GL_DEPTH_TEST);
    }

    OpenGLContext& context;
    GLuint frameBufferID;
    Rectangle<i32> bounds;
};

//==============================================================================
struct PositionedTexture
{
    PositionedTexture (OpenGLTexture& texture, const EdgeTable& et, Rectangle<i32> clipRegion)
        : clip (clipRegion.getIntersection (et.getMaximumBounds()))
    {
        if (clip.contains (et.getMaximumBounds()))
        {
            createMap (texture, et);
        }
        else
        {
            EdgeTable et2 (clip);
            et2.clipToEdgeTable (et);
            createMap (texture, et2);
        }
    }

    PositionedTexture (GLuint texture, Rectangle<i32> r, Rectangle<i32> clipRegion) noexcept
        : textureID (texture), area (r), clip (clipRegion)
    {}

    GLuint textureID;
    Rectangle<i32> area, clip;

private:
    z0 createMap (OpenGLTexture& texture, const EdgeTable& et)
    {
        EdgeTableAlphaMap alphaMap (et);
        texture.loadAlpha (alphaMap.data, alphaMap.area.getWidth(), alphaMap.area.getHeight());
        textureID = texture.getTextureID();
        area = alphaMap.area;
    }

    struct EdgeTableAlphaMap
    {
        EdgeTableAlphaMap (const EdgeTable& et)
            : area (et.getMaximumBounds().withSize (nextPowerOfTwo (et.getMaximumBounds().getWidth()),
                                                    nextPowerOfTwo (et.getMaximumBounds().getHeight())))
        {
            data.calloc (area.getWidth() * area.getHeight());
            et.iterate (*this);
        }

        inline z0 setEdgeTableYPos (i32k y) noexcept
        {
            currentLine = data + (area.getBottom() - 1 - y) * area.getWidth() - area.getX();
        }

        inline z0 handleEdgeTablePixel (i32k x, i32k alphaLevel) const noexcept
        {
            currentLine[x] = (u8) alphaLevel;
        }

        inline z0 handleEdgeTablePixelFull (i32k x) const noexcept
        {
            currentLine[x] = 255;
        }

        inline z0 handleEdgeTableLine (i32 x, i32 width, i32k alphaLevel) const noexcept
        {
            memset (currentLine + x, (u8) alphaLevel, (size_t) width);
        }

        inline z0 handleEdgeTableLineFull (i32 x, i32 width) const noexcept
        {
            memset (currentLine + x, 255, (size_t) width);
        }

        z0 handleEdgeTableRectangle (i32 x, i32 y, i32 width, i32 height, i32 alphaLevel) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLine (x, width, alphaLevel);
            }
        }

        z0 handleEdgeTableRectangleFull (i32 x, i32 y, i32 width, i32 height) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLineFull (x, width);
            }
        }

        HeapBlock<u8> data;
        const Rectangle<i32> area;

    private:
        u8* currentLine;

        DRX_DECLARE_NON_COPYABLE (EdgeTableAlphaMap)
    };
};

//==============================================================================
struct ShaderPrograms final : public ReferenceCountedObject
{
    ShaderPrograms (OpenGLContext& context)
        : solidColorProgram (context),
          solidColorMasked (context),
          radialGradient (context),
          radialGradientMasked (context),
          linearGradient1 (context),
          linearGradient1Masked (context),
          linearGradient2 (context),
          linearGradient2Masked (context),
          image (context),
          imageMasked (context),
          tiledImage (context),
          tiledImageMasked (context),
          copyTexture (context),
          maskTexture (context)
    {}

    using Ptr = ReferenceCountedObjectPtr<ShaderPrograms>;

    //==============================================================================
    struct ShaderProgramHolder
    {
        ShaderProgramHolder (OpenGLContext& context, tukk fragmentShader, tukk vertexShader)
            : program (context)
        {
            DRX_CHECK_OPENGL_ERROR

            if (vertexShader == nullptr)
                vertexShader = "attribute vec2 position;"
                               "attribute vec4 colour;"
                               "uniform vec4 screenBounds;"
                               "varying " DRX_MEDIUMP " vec4 frontColor;"
                               "varying " DRX_HIGHP " vec2 pixelPos;"
                               "z0 main()"
                               "{"
                                 "frontColor = colour;"
                                 "vec2 adjustedPos = position - screenBounds.xy;"
                                 "pixelPos = adjustedPos;"
                                 "vec2 scaledPos = adjustedPos / screenBounds.zw;"
                                 "gl_Position = vec4 (scaledPos.x - 1.0, 1.0 - scaledPos.y, 0, 1.0);"
                               "}";

            if (program.addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (vertexShader))
                 && program.addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
                 && program.link())
            {
                DRX_CHECK_OPENGL_ERROR
            }
            else
            {
                lastError = program.getLastError();
            }
        }

        virtual ~ShaderProgramHolder() = default;

        OpenGLShaderProgram program;
        Txt lastError;
    };

    struct ShaderBase : public ShaderProgramHolder
    {
        ShaderBase (OpenGLContext& context, tukk fragmentShader, tukk vertexShader = nullptr)
            : ShaderProgramHolder (context, fragmentShader, vertexShader),
              positionAttribute (program, "position"),
              colourAttribute (program, "colour"),
              screenBounds (program, "screenBounds")
        {}

        z0 set2DBounds (Rectangle<f32> bounds)
        {
            screenBounds.set (bounds.getX(), bounds.getY(), 0.5f * bounds.getWidth(), 0.5f * bounds.getHeight());
        }

        z0 bindAttributes()
        {
            gl::glVertexAttribPointer ((GLuint) positionAttribute.attributeID, 2, GL_SHORT, GL_FALSE, 8, nullptr);
            gl::glVertexAttribPointer ((GLuint) colourAttribute.attributeID, 4, GL_UNSIGNED_BYTE, GL_TRUE, 8, (uk) 4);
            gl::glEnableVertexAttribArray ((GLuint) positionAttribute.attributeID);
            gl::glEnableVertexAttribArray ((GLuint) colourAttribute.attributeID);
        }

        z0 unbindAttributes()
        {
            gl::glDisableVertexAttribArray ((GLuint) positionAttribute.attributeID);
            gl::glDisableVertexAttribArray ((GLuint) colourAttribute.attributeID);
        }

        OpenGLShaderProgram::Attribute positionAttribute, colourAttribute;
        OpenGLShaderProgram::Uniform screenBounds;
        std::function<z0 (OpenGLShaderProgram&)> onShaderActivated;
    };

    struct MaskedShaderParams
    {
        MaskedShaderParams (OpenGLShaderProgram& program)
            : maskTexture (program, "maskTexture"),
              maskBounds  (program, "maskBounds")
        {}

        z0 setBounds (Rectangle<i32> area, const Target& target, GLint textureIndex) const
        {
            maskTexture.set (textureIndex);
            maskBounds.set (area.getX() - target.bounds.getX(),
                            area.getY() - target.bounds.getY(),
                            area.getWidth(), area.getHeight());
        }

        OpenGLShaderProgram::Uniform maskTexture, maskBounds;
    };

    //==============================================================================
    #define DRX_DECLARE_VARYING_COLOUR   "varying " DRX_MEDIUMP " vec4 frontColor;"
    #define DRX_DECLARE_VARYING_PIXELPOS "varying " DRX_HIGHP " vec2 pixelPos;"

    struct SolidColorProgram final : public ShaderBase
    {
        SolidColorProgram (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_VARYING_COLOUR
                          "z0 main() { gl_FragColor = frontColor; }")
        {}
    };

    #define DRX_DECLARE_MASK_UNIFORMS  "uniform sampler2D maskTexture;" \
                                        "uniform ivec4 maskBounds;"
    #define DRX_FRAGCOORD_TO_MASK_POS  "vec2 ((pixelPos.x - f32 (maskBounds.x)) / f32 (maskBounds.z)," \
                                              "1.0 - (pixelPos.y - f32 (maskBounds.y)) / f32 (maskBounds.w))"
    #define DRX_GET_MASK_ALPHA         "texture2D (maskTexture, " DRX_FRAGCOORD_TO_MASK_POS ").a"

    struct SolidColorMaskedProgram final : public ShaderBase
    {
        SolidColorMaskedProgram (OpenGLContext& context)
            : ShaderBase (context,
                          DRX_DECLARE_MASK_UNIFORMS DRX_DECLARE_VARYING_COLOUR DRX_DECLARE_VARYING_PIXELPOS
                          "z0 main() {"
                            "gl_FragColor = frontColor * " DRX_GET_MASK_ALPHA ";"
                          "}"),
              maskParams (program)
        {}

        MaskedShaderParams maskParams;
    };

    //==============================================================================
    struct RadialGradientParams
    {
        RadialGradientParams (OpenGLShaderProgram& program)
            : gradientTexture (program, "gradientTexture"),
              matrix (program, "matrix")
        {}

        z0 setMatrix (Point<f32> p1, Point<f32> p2, Point<f32> p3)
        {
            auto t = AffineTransform::fromTargetPoints (p1, Point<f32>(),
                                                        p2, Point<f32> (1.0f, 0.0f),
                                                        p3, Point<f32> (0.0f, 1.0f));
            const GLfloat m[] = { t.mat00, t.mat01, t.mat02, t.mat10, t.mat11, t.mat12 };
            matrix.set (m, 6);
        }

        OpenGLShaderProgram::Uniform gradientTexture, matrix;
    };

    #define DRX_DECLARE_MATRIX_UNIFORM   "uniform " DRX_HIGHP " f32 matrix[6];"
    #define DRX_DECLARE_RADIAL_UNIFORMS  "uniform sampler2D gradientTexture;" DRX_DECLARE_MATRIX_UNIFORM
    #define DRX_MATRIX_TIMES_FRAGCOORD   "(mat2 (matrix[0], matrix[3], matrix[1], matrix[4]) * pixelPos" \
                                          " + vec2 (matrix[2], matrix[5]))"
    #define DRX_GET_TEXTURE_COLOUR       "(frontColor.a * texture2D (gradientTexture, vec2 (gradientPos, 0.5)))"

    struct RadialGradientProgram final : public ShaderBase
    {
        RadialGradientProgram (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_VARYING_PIXELPOS
                          DRX_DECLARE_RADIAL_UNIFORMS DRX_DECLARE_VARYING_COLOUR
                          "z0 main()"
                          "{"
                            DRX_MEDIUMP " f32 gradientPos = length (" DRX_MATRIX_TIMES_FRAGCOORD ");"
                            "gl_FragColor = " DRX_GET_TEXTURE_COLOUR ";"
                          "}"),
              gradientParams (program)
        {}

        RadialGradientParams gradientParams;
    };

    struct RadialGradientMaskedProgram final : public ShaderBase
    {
        RadialGradientMaskedProgram (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_VARYING_PIXELPOS
                          DRX_DECLARE_RADIAL_UNIFORMS DRX_DECLARE_VARYING_COLOUR
                          DRX_DECLARE_MASK_UNIFORMS
                          "z0 main()"
                          "{"
                            DRX_MEDIUMP " f32 gradientPos = length (" DRX_MATRIX_TIMES_FRAGCOORD ");"
                            "gl_FragColor = " DRX_GET_TEXTURE_COLOUR " * " DRX_GET_MASK_ALPHA ";"
                          "}"),
              gradientParams (program),
              maskParams (program)
        {}

        RadialGradientParams gradientParams;
        MaskedShaderParams maskParams;
    };

    //==============================================================================
    struct LinearGradientParams
    {
        LinearGradientParams (OpenGLShaderProgram& program)
            : gradientTexture (program, "gradientTexture"),
              gradientInfo (program, "gradientInfo")
        {}

        OpenGLShaderProgram::Uniform gradientTexture, gradientInfo;
    };

    #define DRX_DECLARE_LINEAR_UNIFORMS  "uniform sampler2D gradientTexture;" \
                                          "uniform " DRX_MEDIUMP " vec4 gradientInfo;" \
                                          DRX_DECLARE_VARYING_COLOUR DRX_DECLARE_VARYING_PIXELPOS
    #define DRX_CALC_LINEAR_GRAD_POS1    DRX_MEDIUMP " f32 gradientPos = (pixelPos.y - (gradientInfo.y + (gradientInfo.z * (pixelPos.x - gradientInfo.x)))) / gradientInfo.w;"
    #define DRX_CALC_LINEAR_GRAD_POS2    DRX_MEDIUMP " f32 gradientPos = (pixelPos.x - (gradientInfo.x + (gradientInfo.z * (pixelPos.y - gradientInfo.y)))) / gradientInfo.w;"

    struct LinearGradient1Program final : public ShaderBase
    {
        LinearGradient1Program (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_LINEAR_UNIFORMS  // gradientInfo: x = x1, y = y1, z = (y2 - y1) / (x2 - x1), w = length
                          "z0 main()"
                          "{"
                            DRX_CALC_LINEAR_GRAD_POS1
                            "gl_FragColor = " DRX_GET_TEXTURE_COLOUR ";"
                          "}"),
              gradientParams (program)
        {}

        LinearGradientParams gradientParams;
    };

    struct LinearGradient1MaskedProgram final : public ShaderBase
    {
        LinearGradient1MaskedProgram (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_LINEAR_UNIFORMS  // gradientInfo: x = x1, y = y1, z = (y2 - y1) / (x2 - x1), w = length
                          DRX_DECLARE_MASK_UNIFORMS
                          "z0 main()"
                          "{"
                            DRX_CALC_LINEAR_GRAD_POS1
                            "gl_FragColor = " DRX_GET_TEXTURE_COLOUR " * " DRX_GET_MASK_ALPHA ";"
                          "}"),
              gradientParams (program),
              maskParams (program)
        {}

        LinearGradientParams gradientParams;
        MaskedShaderParams maskParams;
    };

    struct LinearGradient2Program final : public ShaderBase
    {
        LinearGradient2Program (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_LINEAR_UNIFORMS  // gradientInfo: x = x1, y = y1, z = (x2 - x1) / (y2 - y1), y = y1, w = length
                          "z0 main()"
                          "{"
                            DRX_CALC_LINEAR_GRAD_POS2
                            "gl_FragColor = " DRX_GET_TEXTURE_COLOUR ";"
                          "}"),
              gradientParams (program)
        {}

        LinearGradientParams gradientParams;
    };

    struct LinearGradient2MaskedProgram final : public ShaderBase
    {
        LinearGradient2MaskedProgram (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_LINEAR_UNIFORMS  // gradientInfo: x = x1, y = y1, z = (x2 - x1) / (y2 - y1), y = y1, w = length
                          DRX_DECLARE_MASK_UNIFORMS
                          "z0 main()"
                          "{"
                            DRX_CALC_LINEAR_GRAD_POS2
                            "gl_FragColor = " DRX_GET_TEXTURE_COLOUR " * " DRX_GET_MASK_ALPHA ";"
                          "}"),
              gradientParams (program),
              maskParams (program)
        {}

        LinearGradientParams gradientParams;
        MaskedShaderParams maskParams;
    };

    //==============================================================================
    struct ImageParams
    {
        ImageParams (OpenGLShaderProgram& program)
            : imageTexture (program, "imageTexture"),
              matrix (program, "matrix"),
              imageLimits (program, "imageLimits")
        {}

        z0 setMatrix (const AffineTransform& trans, i32 imageWidth, i32 imageHeight,
                        f32 fullWidthProportion, f32 fullHeightProportion,
                        f32 targetX, f32 targetY, b8 isForTiling) const
        {
            auto t = trans.translated (-targetX, -targetY)
                          .inverted().scaled (fullWidthProportion  / (f32) imageWidth,
                                              fullHeightProportion / (f32) imageHeight);

            const GLfloat m[] = { t.mat00, t.mat01, t.mat02, t.mat10, t.mat11, t.mat12 };
            matrix.set (m, 6);

            if (isForTiling)
            {
                fullWidthProportion  -= 0.5f / (f32) imageWidth;
                fullHeightProportion -= 0.5f / (f32) imageHeight;
            }

            imageLimits.set (fullWidthProportion, fullHeightProportion);
        }

        z0 setMatrix (const AffineTransform& trans, const TextureInfo& textureInfo,
                        f32 targetX, f32 targetY, b8 isForTiling) const
        {
            setMatrix (trans,
                       textureInfo.imageWidth, textureInfo.imageHeight,
                       textureInfo.fullWidthProportion, textureInfo.fullHeightProportion,
                       targetX, targetY, isForTiling);
        }

        OpenGLShaderProgram::Uniform imageTexture, matrix, imageLimits;
    };

    #define DRX_DECLARE_IMAGE_UNIFORMS "uniform sampler2D imageTexture;" \
                                        "uniform " DRX_MEDIUMP " vec2 imageLimits;" \
                                        DRX_DECLARE_MATRIX_UNIFORM DRX_DECLARE_VARYING_COLOUR DRX_DECLARE_VARYING_PIXELPOS
    #define DRX_GET_IMAGE_PIXEL        "texture2D (imageTexture, vec2 (texturePos.x, 1.0 - texturePos.y))"
    #define DRX_CLAMP_TEXTURE_COORD    DRX_HIGHP " vec2 texturePos = clamp (" DRX_MATRIX_TIMES_FRAGCOORD ", vec2 (0, 0), imageLimits);"
    #define DRX_MOD_TEXTURE_COORD      DRX_HIGHP " vec2 texturePos = mod (" DRX_MATRIX_TIMES_FRAGCOORD ", imageLimits);"

    struct ImageProgram final : public ShaderBase
    {
        ImageProgram (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_VARYING_COLOUR
                          "uniform sampler2D imageTexture;"
                          "varying " DRX_HIGHP " vec2 texturePos;"
                          "z0 main()"
                          "{"
                            "gl_FragColor = frontColor.a * " DRX_GET_IMAGE_PIXEL ";"
                          "}",
                          "uniform " DRX_MEDIUMP " vec2 imageLimits;"
                          DRX_DECLARE_MATRIX_UNIFORM
                          "attribute vec2 position;"
                          "attribute vec4 colour;"
                          "uniform vec4 screenBounds;"
                          "varying " DRX_MEDIUMP " vec4 frontColor;"
                          "varying " DRX_HIGHP " vec2 texturePos;"
                          "z0 main()"
                          "{"
                            "frontColor = colour;"
                            "vec2 adjustedPos = position - screenBounds.xy;"
                            "vec2 pixelPos = adjustedPos;"
                            "texturePos = clamp (" DRX_MATRIX_TIMES_FRAGCOORD ", vec2 (0, 0), imageLimits);"
                            "vec2 scaledPos = adjustedPos / screenBounds.zw;"
                            "gl_Position = vec4 (scaledPos.x - 1.0, 1.0 - scaledPos.y, 0, 1.0);"
                          "}"),
              imageParams (program)
        {}

        ImageParams imageParams;
    };

    struct ImageMaskedProgram final : public ShaderBase
    {
        ImageMaskedProgram (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_IMAGE_UNIFORMS DRX_DECLARE_MASK_UNIFORMS
                          "z0 main()"
                          "{"
                            DRX_CLAMP_TEXTURE_COORD
                            "gl_FragColor = frontColor.a * " DRX_GET_IMAGE_PIXEL " * " DRX_GET_MASK_ALPHA ";"
                          "}"),
              imageParams (program),
              maskParams (program)
        {}

        ImageParams imageParams;
        MaskedShaderParams maskParams;
    };

    struct TiledImageProgram final : public ShaderBase
    {
        TiledImageProgram (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_IMAGE_UNIFORMS
                          "z0 main()"
                          "{"
                            DRX_MOD_TEXTURE_COORD
                            "gl_FragColor = frontColor.a * " DRX_GET_IMAGE_PIXEL ";"
                          "}"),
              imageParams (program)
        {}

        ImageParams imageParams;
    };

    struct TiledImageMaskedProgram final : public ShaderBase
    {
        TiledImageMaskedProgram (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_IMAGE_UNIFORMS DRX_DECLARE_MASK_UNIFORMS
                          "z0 main()"
                          "{"
                            DRX_MOD_TEXTURE_COORD
                            "gl_FragColor = frontColor.a * " DRX_GET_IMAGE_PIXEL " * " DRX_GET_MASK_ALPHA ";"
                          "}"),
              imageParams (program),
              maskParams (program)
        {}

        ImageParams imageParams;
        MaskedShaderParams maskParams;
    };

    struct CopyTextureProgram final : public ShaderBase
    {
        CopyTextureProgram (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_IMAGE_UNIFORMS
                          "z0 main()"
                          "{"
                            DRX_MOD_TEXTURE_COORD
                            "gl_FragColor = frontColor.a * " DRX_GET_IMAGE_PIXEL ";"
                          "}"),
              imageParams (program)
        {}

        ImageParams imageParams;
    };

    struct MaskTextureProgram final : public ShaderBase
    {
        MaskTextureProgram (OpenGLContext& context)
            : ShaderBase (context, DRX_DECLARE_IMAGE_UNIFORMS
                          "z0 main()"
                          "{"
                            DRX_HIGHP " vec2 texturePos = " DRX_MATRIX_TIMES_FRAGCOORD ";"
                            DRX_HIGHP " f32 roundingError = 0.00001;"
                            "if (texturePos.x >= -roundingError"
                                 "&& texturePos.y >= -roundingError"
                                 "&& texturePos.x <= imageLimits.x + roundingError"
                                 "&& texturePos.y <= imageLimits.y + roundingError)"
                             "gl_FragColor = frontColor * " DRX_GET_IMAGE_PIXEL ".a;"
                            "else "
                             "gl_FragColor = vec4 (0, 0, 0, 0);"
                          "}"),
              imageParams (program)
        {}

        ImageParams imageParams;
    };

    SolidColorProgram solidColorProgram;
    SolidColorMaskedProgram solidColorMasked;
    RadialGradientProgram radialGradient;
    RadialGradientMaskedProgram radialGradientMasked;
    LinearGradient1Program linearGradient1;
    LinearGradient1MaskedProgram linearGradient1Masked;
    LinearGradient2Program linearGradient2;
    LinearGradient2MaskedProgram linearGradient2Masked;
    ImageProgram image;
    ImageMaskedProgram imageMasked;
    TiledImageProgram tiledImage;
    TiledImageMaskedProgram tiledImageMasked;
    CopyTextureProgram copyTexture;
    MaskTextureProgram maskTexture;
};

//==============================================================================
struct TraitsVAO
{
    static b8 isCoreProfile()
    {
       #if DRX_OPENGL_ES
        return true;
       #else
        clearGLError();
        GLint mask = 0;
        glGetIntegerv (GL_CONTEXT_PROFILE_MASK, &mask);

        // The context isn't aware of the profile mask, so it pre-dates the core profile
        if (glGetError() == GL_INVALID_ENUM)
            return false;

        // Also assumes a compatibility profile if the mask is completely empty for some reason
        return (mask & (GLint) GL_CONTEXT_CORE_PROFILE_BIT) != 0;
       #endif
    }

    /*  Возвращает true, если the context requires a non-zero vertex array object (VAO) to be bound.

        If the context is a compatibility context, we can just pretend that VAOs don't exist,
        and use the default VAO all the time instead. This provides a more consistent experience
        in user code, which might make calls (like glVertexPointer()) that only work when VAO 0 is
        bound in OpenGL 3.2+.
    */
    static b8 shouldUseCustomVAO()
    {
       #if DRX_OPENGL_ES
        return false;
       #else
        return isCoreProfile();
       #endif
    }

    static constexpr auto value = GL_VERTEX_ARRAY_BINDING;
    static constexpr auto& gen = glGenVertexArrays;
    static constexpr auto& del = glDeleteVertexArrays;
    template <typename T>
    static z0 bind (T x) { gl::glBindVertexArray (static_cast<GLuint> (x)); }
    static constexpr auto predicate = shouldUseCustomVAO;
};

struct TraitsArrayBuffer
{
    static constexpr auto value = GL_ARRAY_BUFFER_BINDING;
    static constexpr auto& gen = glGenBuffers;
    static constexpr auto& del = glDeleteBuffers;
    template <typename T>
    static z0 bind (T x) { gl::glBindBuffer (GL_ARRAY_BUFFER, static_cast<GLuint> (x)); }
    static b8 predicate() { return true; }
};

struct TraitsElementArrayBuffer
{
    static constexpr auto value = GL_ELEMENT_ARRAY_BUFFER_BINDING;
    static constexpr auto& gen = glGenBuffers;
    static constexpr auto& del = glDeleteBuffers;
    template <typename T>
    static z0 bind (T x) { gl::glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, static_cast<GLuint> (x)); }
    static b8 predicate() { return true; }
};

template <typename Traits>
class SavedBinding
{
public:
    SavedBinding() = default;

    ~SavedBinding()
    {
        if (! Traits::predicate())
            return;

        Traits::bind (values.previous);
        Traits::del (1, &values.current);
    }

    z0 bind() const { Traits::bind (values.current); }

    DRX_DECLARE_NON_COPYABLE (SavedBinding)
    DRX_DECLARE_NON_MOVEABLE (SavedBinding)

private:
    struct Values
    {
        GLint previous{};
        GLuint current{};
    };

    Values values = []
    {
        if (! Traits::predicate())
            return Values{};

        GLint previous{};
        glGetIntegerv (Traits::value, &previous);

        GLuint current{};
        Traits::gen (1, &current);
        Traits::bind (current);

        return Values { previous, current };
    }();
};

//==============================================================================
struct StateHelpers
{
    struct BlendingMode
    {
        BlendingMode() noexcept {}

        z0 resync() noexcept
        {
            glDisable (GL_BLEND);
            srcFunction = dstFunction = 0;
        }

        template <typename QuadQueueType>
        z0 setPremultipliedBlendingMode (QuadQueueType& quadQueue) noexcept
        {
            setBlendFunc (quadQueue, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }

        template <typename QuadQueueType>
        z0 setBlendFunc (QuadQueueType& quadQueue, GLenum src, GLenum dst)
        {
            if (! blendingEnabled)
            {
                quadQueue.flush();
                blendingEnabled = true;
                glEnable (GL_BLEND);
            }

            if (srcFunction != src || dstFunction != dst)
            {
                quadQueue.flush();
                srcFunction = src;
                dstFunction = dst;
                glBlendFunc (src, dst);
            }
        }

        template <typename QuadQueueType>
        z0 disableBlend (QuadQueueType& quadQueue) noexcept
        {
            if (blendingEnabled)
            {
                quadQueue.flush();
                blendingEnabled = false;
                glDisable (GL_BLEND);
            }
        }

        template <typename QuadQueueType>
        z0 setBlendMode (QuadQueueType& quadQueue, b8 replaceExistingContents) noexcept
        {
            if (replaceExistingContents)
                disableBlend (quadQueue);
            else
                setPremultipliedBlendingMode (quadQueue);
        }

    private:
        b8 blendingEnabled = false;
        GLenum srcFunction = 0, dstFunction = 0;
    };

    //==============================================================================
    template <typename QuadQueueType>
    struct EdgeTableRenderer
    {
        EdgeTableRenderer (QuadQueueType& q, PixelARGB c) noexcept
            : quadQueue (q), colour (c)
        {}

        z0 setEdgeTableYPos (i32 y) noexcept
        {
            currentY = y;
        }

        z0 handleEdgeTablePixel (i32 x, i32 alphaLevel) noexcept
        {
            auto c = colour;
            c.multiplyAlpha (alphaLevel);
            quadQueue.add (x, currentY, 1, 1, c);
        }

        z0 handleEdgeTablePixelFull (i32 x) noexcept
        {
            quadQueue.add (x, currentY, 1, 1, colour);
        }

        z0 handleEdgeTableLine (i32 x, i32 width, i32 alphaLevel) noexcept
        {
            auto c = colour;
            c.multiplyAlpha (alphaLevel);
            quadQueue.add (x, currentY, width, 1, c);
        }

        z0 handleEdgeTableLineFull (i32 x, i32 width) noexcept
        {
            quadQueue.add (x, currentY, width, 1, colour);
        }

        z0 handleEdgeTableRectangle (i32 x, i32 y, i32 width, i32 height, i32 alphaLevel) noexcept
        {
            auto c = colour;
            c.multiplyAlpha (alphaLevel);
            quadQueue.add (x, y, width, height, c);
        }

        z0 handleEdgeTableRectangleFull (i32 x, i32 y, i32 width, i32 height) noexcept
        {
            quadQueue.add (x, y, width, height, colour);
        }

    private:
        QuadQueueType& quadQueue;
        const PixelARGB colour;
        i32 currentY;

        DRX_DECLARE_NON_COPYABLE (EdgeTableRenderer)
    };

    template <typename QuadQueueType>
    struct FloatRectangleRenderer
    {
        FloatRectangleRenderer (QuadQueueType& q, PixelARGB c) noexcept
            : quadQueue (q), colour (c)
        {}

        z0 operator() (i32 x, i32 y, i32 w, i32 h, i32 alpha) noexcept
        {
            if (w > 0 && h > 0)
            {
                PixelARGB c (colour);
                c.multiplyAlpha (alpha);
                quadQueue.add (x, y, w, h, c);
            }
        }

    private:
        QuadQueueType& quadQueue;
        const PixelARGB colour;

        DRX_DECLARE_NON_COPYABLE (FloatRectangleRenderer)
    };

    //==============================================================================
    struct ActiveTextures
    {
        explicit ActiveTextures (const OpenGLContext& c) noexcept
            : context (c)
        {
        }

        z0 clear() noexcept
        {
            zeromem (currentTextureID, sizeof (currentTextureID));
        }

        template <typename QuadQueueType>
        z0 setTexturesEnabled (QuadQueueType& quadQueue, i32 textureIndexMask) noexcept
        {
            if (texturesEnabled != textureIndexMask)
            {
                quadQueue.flush();

                for (i32 i = numTextures; --i >= 0;)
                {
                    if ((texturesEnabled & (1 << i)) != (textureIndexMask & (1 << i)))
                    {
                        setActiveTexture (i);
                        DRX_CHECK_OPENGL_ERROR

                        const auto thisTextureEnabled = (textureIndexMask & (1 << i)) != 0;

                        if (! thisTextureEnabled)
                            currentTextureID[i] = 0;

                       #if ! DRX_ANDROID
                        if (needsToEnableTexture)
                        {
                            if (thisTextureEnabled)
                                glEnable (GL_TEXTURE_2D);
                            else
                                glDisable (GL_TEXTURE_2D);

                            DRX_CHECK_OPENGL_ERROR
                        }
                       #endif
                    }
                }

                texturesEnabled = textureIndexMask;
            }
        }

        template <typename QuadQueueType>
        z0 disableTextures (QuadQueueType& quadQueue) noexcept
        {
            setTexturesEnabled (quadQueue, 0);
        }

        template <typename QuadQueueType>
        z0 setSingleTextureMode (QuadQueueType& quadQueue) noexcept
        {
            setTexturesEnabled (quadQueue, 1);
            setActiveTexture (0);
        }

        template <typename QuadQueueType>
        z0 setTwoTextureMode (QuadQueueType& quadQueue, GLuint texture1, GLuint texture2)
        {
            DRX_CHECK_OPENGL_ERROR
            setTexturesEnabled (quadQueue, 3);

            if (currentActiveTexture == 0)
            {
                bindTexture (texture1);
                setActiveTexture (1);
                bindTexture (texture2);
            }
            else
            {
                setActiveTexture (1);
                bindTexture (texture2);
                setActiveTexture (0);
                bindTexture (texture1);
            }

            DRX_CHECK_OPENGL_ERROR
        }

        z0 setActiveTexture (i32 index) noexcept
        {
            if (currentActiveTexture != index)
            {
                currentActiveTexture = index;
                context.extensions.glActiveTexture (GL_TEXTURE0 + (GLenum) index);
                DRX_CHECK_OPENGL_ERROR
            }
        }

        z0 bindTexture (GLuint textureID) noexcept
        {
            if (currentActiveTexture < 0 || numTextures <= currentActiveTexture)
            {
                jassertfalse;
                return;
            }

            if (currentTextureID[currentActiveTexture] != textureID)
            {
                currentTextureID[currentActiveTexture] = textureID;
                glBindTexture (GL_TEXTURE_2D, textureID);
                DRX_CHECK_OPENGL_ERROR
            }
            else
            {
               #if DRX_DEBUG
                GLint t = 0;
                glGetIntegerv (GL_TEXTURE_BINDING_2D, &t);
                jassert (t == (GLint) textureID);
               #endif
            }
        }

    private:
        static constexpr auto numTextures = 3;
        GLuint currentTextureID[numTextures];
        i32 texturesEnabled = 0, currentActiveTexture = -1;
        const OpenGLContext& context;
        const b8 needsToEnableTexture = ! context.isCoreProfile();

        ActiveTextures& operator= (const ActiveTextures&);
    };

    //==============================================================================
    struct TextureCache
    {
        TextureCache() noexcept {}

        OpenGLTexture* getTexture (ActiveTextures& activeTextures, i32 w, i32 h)
        {
            if (textures.size() < numTexturesToCache)
            {
                activeTextures.clear();
                return new OpenGLTexture();
            }

            for (i32 i = 0; i < numTexturesToCache - 2; ++i)
            {
                auto* t = textures.getUnchecked (i);

                if (t->getWidth() == w && t->getHeight() == h)
                    return textures.removeAndReturn (i);
            }

            return textures.removeAndReturn (0);
        }

        z0 resetGradient() noexcept
        {
            gradientNeedsRefresh = true;
        }

        z0 bindTextureForGradient (ActiveTextures& activeTextures, const ColorGradient& gradient)
        {
            if (gradientNeedsRefresh)
            {
                gradientNeedsRefresh = false;

                if (gradientTextures.size() < numGradientTexturesToCache)
                {
                    activeGradientIndex = gradientTextures.size();
                    activeTextures.clear();
                    gradientTextures.add (new OpenGLTexture());
                }
                else
                {
                    activeGradientIndex = (activeGradientIndex + 1) % numGradientTexturesToCache;
                }

                DRX_CHECK_OPENGL_ERROR;
                PixelARGB lookup[gradientTextureSize];
                gradient.createLookupTable (lookup);
                gradientTextures.getUnchecked (activeGradientIndex)->loadARGB (lookup, gradientTextureSize, 1);
            }

            activeTextures.bindTexture (gradientTextures.getUnchecked (activeGradientIndex)->getTextureID());
        }

        enum { gradientTextureSize = 256 };

    private:
        enum { numTexturesToCache = 8, numGradientTexturesToCache = 10 };
        OwnedArray<OpenGLTexture> textures, gradientTextures;
        i32 activeGradientIndex = 0;
        b8 gradientNeedsRefresh = true;
    };

    //==============================================================================
    struct ShaderQuadQueue
    {
        ShaderQuadQueue (const OpenGLContext& c) noexcept  : context (c)
        {}

        ~ShaderQuadQueue() noexcept
        {
            static_assert (sizeof (VertexInfo) == 8, "Sanity check VertexInfo size");
        }

        z0 initialise() noexcept
        {
            DRX_CHECK_OPENGL_ERROR

           #if DRX_ANDROID || DRX_IOS
            i32 numQuads = maxNumQuads;
           #else
            GLint maxIndices = 0;
            glGetIntegerv (GL_MAX_ELEMENTS_INDICES, &maxIndices);
            auto numQuads = jmin ((i32) maxNumQuads, (i32) maxIndices / 6);
            maxVertices = numQuads * 4 - 4;
           #endif

            for (i32 i = 0, v = 0; i < numQuads * 6; i += 6, v += 4)
            {
                indexData[i] = (GLushort) v;
                indexData[i + 1] = indexData[i + 3] = (GLushort) (v + 1);
                indexData[i + 2] = indexData[i + 4] = (GLushort) (v + 2);
                indexData[i + 5] = (GLushort) (v + 3);
            }

            savedElementArrayBuffer.bind();
            context.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (indexData), indexData, GL_STATIC_DRAW);

            savedArrayBuffer.bind();
            context.extensions.glBufferData (GL_ARRAY_BUFFER, sizeof (vertexData), vertexData, GL_STREAM_DRAW);
            DRX_CHECK_OPENGL_ERROR
        }

        z0 add (i32 x, i32 y, i32 w, i32 h, PixelARGB colour) noexcept
        {
            jassert (w > 0 && h > 0);

            auto* v = vertexData + numVertices;
            v[0].x = v[2].x = (GLshort) x;
            v[0].y = v[1].y = (GLshort) y;
            v[1].x = v[3].x = (GLshort) (x + w);
            v[2].y = v[3].y = (GLshort) (y + h);

           #if DRX_BIG_ENDIAN
            auto rgba = (GLuint) ((colour.getRed() << 24) | (colour.getGreen() << 16)
                                | (colour.getBlue() << 8) |  colour.getAlpha());
           #else
            auto rgba = (GLuint) ((colour.getAlpha() << 24) | (colour.getBlue() << 16)
                                | (colour.getGreen() << 8) |  colour.getRed());
           #endif

            v[0].colour = rgba;
            v[1].colour = rgba;
            v[2].colour = rgba;
            v[3].colour = rgba;

            numVertices += 4;

            if (numVertices > maxVertices)
                draw();
        }

        z0 add (Rectangle<i32> r, PixelARGB colour) noexcept
        {
            add (r.getX(), r.getY(), r.getWidth(), r.getHeight(), colour);
        }

        z0 add (Rectangle<f32> r, PixelARGB colour) noexcept
        {
            FloatRectangleRenderer<ShaderQuadQueue> frr (*this, colour);
            RenderingHelpers::FloatRectangleRasterisingInfo (r).iterate (frr);
        }

        z0 add (const RectangleList<i32>& list, PixelARGB colour) noexcept
        {
            for (auto& i : list)
                add (i, colour);
        }

        z0 add (const RectangleList<i32>& list, Rectangle<i32> clip, PixelARGB colour) noexcept
        {
            for (auto& i : list)
            {
                auto r = i.getIntersection (clip);

                if (! r.isEmpty())
                    add (r, colour);
            }
        }

        template <typename IteratorType>
        z0 add (const IteratorType& et, PixelARGB colour)
        {
            EdgeTableRenderer<ShaderQuadQueue> etr (*this, colour);
            et.iterate (etr);
        }

        z0 flush() noexcept
        {
            if (numVertices > 0)
                draw();
        }

    private:
        struct VertexInfo
        {
            GLshort x, y;
            GLuint colour;
        };

        enum { maxNumQuads = 256 };

        SavedBinding<TraitsArrayBuffer> savedArrayBuffer;
        SavedBinding<TraitsElementArrayBuffer> savedElementArrayBuffer;
        VertexInfo vertexData[maxNumQuads * 4];
        GLushort indexData[maxNumQuads * 6];
        const OpenGLContext& context;
        i32 numVertices = 0;

       #if DRX_ANDROID || DRX_IOS
        enum { maxVertices = maxNumQuads * 4 - 4 };
       #else
        i32 maxVertices = 0;
       #endif

        z0 draw() noexcept
        {
            context.extensions.glBufferSubData (GL_ARRAY_BUFFER, 0, (GLsizeiptr) ((size_t) numVertices * sizeof (VertexInfo)), vertexData);
            // NB: If you get a random crash in here and are running in a Parallels VM, it seems to be a bug in
            // their driver.. Can't find a workaround unfortunately.
            glDrawElements (GL_TRIANGLES, (numVertices * 3) / 2, GL_UNSIGNED_SHORT, nullptr);
            DRX_CHECK_OPENGL_ERROR
            numVertices = 0;
        }

        DRX_DECLARE_NON_COPYABLE (ShaderQuadQueue)
    };

    //==============================================================================
    struct CurrentShader
    {
        CurrentShader (OpenGLContext& c) noexcept  : context (c)
        {
            auto programValueID = "GraphicsContextPrograms";
            programs = static_cast<ShaderPrograms*> (context.getAssociatedObject (programValueID));

            if (programs == nullptr)
            {
                programs = new ShaderPrograms (context);
                context.setAssociatedObject (programValueID, programs.get());
            }
        }

        ~CurrentShader()
        {
            jassert (activeShader == nullptr);
        }

        z0 setShader (Rectangle<i32> bounds, ShaderQuadQueue& quadQueue, ShaderPrograms::ShaderBase& shader)
        {
            if (activeShader != &shader)
            {
                clearShader (quadQueue);

                activeShader = &shader;
                shader.program.use();
                shader.bindAttributes();

                if (shader.onShaderActivated)
                    shader.onShaderActivated (shader.program);

                currentBounds = bounds;
                shader.set2DBounds (bounds.toFloat());

                DRX_CHECK_OPENGL_ERROR
            }
            else if (bounds != currentBounds)
            {
                currentBounds = bounds;
                shader.set2DBounds (bounds.toFloat());
            }
        }

        z0 setShader (Target& target, ShaderQuadQueue& quadQueue, ShaderPrograms::ShaderBase& shader)
        {
            setShader (target.bounds, quadQueue, shader);
        }

        z0 clearShader (ShaderQuadQueue& quadQueue)
        {
            if (activeShader != nullptr)
            {
                quadQueue.flush();
                activeShader->unbindAttributes();
                activeShader = nullptr;
                context.extensions.glUseProgram (0);
            }
        }

        OpenGLContext& context;
        ShaderPrograms::Ptr programs;

    private:
        ShaderPrograms::ShaderBase* activeShader = nullptr;
        Rectangle<i32> currentBounds;

        CurrentShader& operator= (const CurrentShader&);
    };
};

//==============================================================================
struct GLState
{
    GLState (const Target& t) noexcept
        : target (t),
          activeTextures (t.context),
          currentShader (t.context),
          shaderQuadQueue (t.context),
          previousFrameBufferTarget (OpenGLFrameBuffer::getCurrentFrameBufferTarget())
    {
        // This object can only be created and used when the current thread has an active OpenGL context.
        jassert (OpenGLHelpers::isContextActive());

        DRX_CHECK_OPENGL_ERROR
        target.makeActive();
        blendMode.resync();
        DRX_CHECK_OPENGL_ERROR
        activeTextures.clear();
        shaderQuadQueue.initialise();
        cachedImageList = CachedImageList::get (t.context);
        DRX_CHECK_OPENGL_ERROR
    }

    ~GLState()
    {
        flush();
        target.context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, previousFrameBufferTarget);
    }

    z0 flush()
    {
        shaderQuadQueue.flush();
        currentShader.clearShader (shaderQuadQueue);
        DRX_CHECK_OPENGL_ERROR
    }

    z0 setShader (ShaderPrograms::ShaderBase& shader)
    {
        currentShader.setShader (target, shaderQuadQueue, shader);
        DRX_CHECK_OPENGL_ERROR
    }

    z0 setShaderForGradientFill (const ColorGradient& g, const AffineTransform& transform,
                                   i32 maskTextureID, const Rectangle<i32>* maskArea)
    {
        DRX_CHECK_OPENGL_ERROR
        activeTextures.disableTextures (shaderQuadQueue);
        blendMode.setPremultipliedBlendingMode (shaderQuadQueue);
        DRX_CHECK_OPENGL_ERROR

        if (maskArea != nullptr)
        {
            activeTextures.setTexturesEnabled (shaderQuadQueue, 3);
            activeTextures.setActiveTexture (1);
            activeTextures.bindTexture ((GLuint) maskTextureID);
            activeTextures.setActiveTexture (0);
            textureCache.bindTextureForGradient (activeTextures, g);
        }
        else
        {
            activeTextures.setSingleTextureMode (shaderQuadQueue);
            textureCache.bindTextureForGradient (activeTextures, g);
        }

        auto t = transform.translated (0.5f - (f32) target.bounds.getX(),
                                       0.5f - (f32) target.bounds.getY());
        auto p1 = g.point1.transformedBy (t);
        auto p2 = g.point2.transformedBy (t);
        auto p3 = Point<f32> (g.point1.x + (g.point2.y - g.point1.y),
                                g.point1.y - (g.point2.x - g.point1.x)).transformedBy (t);

        auto programs = currentShader.programs;
        const ShaderPrograms::MaskedShaderParams* maskParams = nullptr;

        if (g.isRadial)
        {
            ShaderPrograms::RadialGradientParams* gradientParams;

            if (maskArea == nullptr)
            {
                setShader (programs->radialGradient);
                gradientParams = &programs->radialGradient.gradientParams;
            }
            else
            {
                setShader (programs->radialGradientMasked);
                gradientParams = &programs->radialGradientMasked.gradientParams;
                maskParams = &programs->radialGradientMasked.maskParams;
            }

            gradientParams->setMatrix (p1, p2, p3);
        }
        else
        {
            p1 = Line<f32> (p1, p3).findNearestPointTo (p2);
            const Point<f32> delta (p2.x - p1.x, p1.y - p2.y);
            const ShaderPrograms::LinearGradientParams* gradientParams;
            f32 grad, length;

            if (std::abs (delta.x) < std::abs (delta.y))
            {
                if (maskArea == nullptr)
                {
                    setShader (programs->linearGradient1);
                    gradientParams = &(programs->linearGradient1.gradientParams);
                }
                else
                {
                    setShader (programs->linearGradient1Masked);
                    gradientParams = &(programs->linearGradient1Masked.gradientParams);
                    maskParams = &programs->linearGradient1Masked.maskParams;
                }

                grad = delta.x / delta.y;
                length = (p2.y - grad * p2.x) - (p1.y - grad * p1.x);
            }
            else
            {
                if (maskArea == nullptr)
                {
                    setShader (programs->linearGradient2);
                    gradientParams = &(programs->linearGradient2.gradientParams);
                }
                else
                {
                    setShader (programs->linearGradient2Masked);
                    gradientParams = &(programs->linearGradient2Masked.gradientParams);
                    maskParams = &programs->linearGradient2Masked.maskParams;
                }

                grad = delta.y / delta.x;
                length = (p2.x - grad * p2.y) - (p1.x - grad * p1.y);
            }

            gradientParams->gradientInfo.set (p1.x, p1.y, grad, length);
        }

        if (maskParams != nullptr)
            maskParams->setBounds (*maskArea, target, 1);

        DRX_CHECK_OPENGL_ERROR
    }

    z0 setShaderForTiledImageFill (const TextureInfo& textureInfo, const AffineTransform& transform,
                                     i32 maskTextureID, const Rectangle<i32>* maskArea, b8 isTiledFill)
    {
        blendMode.setPremultipliedBlendingMode (shaderQuadQueue);

        auto programs = currentShader.programs;

        const ShaderPrograms::MaskedShaderParams* maskParams = nullptr;
        const ShaderPrograms::ImageParams* imageParams;

        if (maskArea != nullptr)
        {
            activeTextures.setTwoTextureMode (shaderQuadQueue, textureInfo.textureID, (GLuint) maskTextureID);

            if (isTiledFill)
            {
                setShader (programs->tiledImageMasked);
                imageParams = &programs->tiledImageMasked.imageParams;
                maskParams  = &programs->tiledImageMasked.maskParams;
            }
            else
            {
                setShader (programs->imageMasked);
                imageParams = &programs->imageMasked.imageParams;
                maskParams  = &programs->imageMasked.maskParams;
            }
        }
        else
        {
            activeTextures.setSingleTextureMode (shaderQuadQueue);
            activeTextures.bindTexture (textureInfo.textureID);

            if (isTiledFill)
            {
                setShader (programs->tiledImage);
                imageParams = &programs->tiledImage.imageParams;
            }
            else
            {
                setShader (programs->image);
                imageParams = &programs->image.imageParams;
            }
        }

        imageParams->setMatrix (transform, textureInfo, (f32) target.bounds.getX(), (f32) target.bounds.getY(), isTiledFill);

        if (maskParams != nullptr)
            maskParams->setBounds (*maskArea, target, 1);
    }

    Target target;

    StateHelpers::BlendingMode blendMode;
    StateHelpers::ActiveTextures activeTextures;
    StateHelpers::TextureCache textureCache;
    StateHelpers::CurrentShader currentShader;
    StateHelpers::ShaderQuadQueue shaderQuadQueue;

    CachedImageList::Ptr cachedImageList;

private:
    GLuint previousFrameBufferTarget;
    SavedBinding<TraitsVAO> savedVAOBinding;
};

//==============================================================================
struct SavedState final : public RenderingHelpers::SavedStateBase<SavedState>
{
    using BaseClass = RenderingHelpers::SavedStateBase<SavedState>;

    SavedState (GLState* s)  : BaseClass (s->target.bounds), state (s)
    {}

    SavedState (const SavedState& other)
        : BaseClass (other), font (other.font), state (other.state),
          transparencyLayer (other.transparencyLayer),
          previousTarget (createCopyIfNotNull (other.previousTarget.get()))
    {}

    SavedState* beginTransparencyLayer (f32 opacity)
    {
        auto* s = new SavedState (*this);

        if (clip != nullptr)
        {
            auto clipBounds = clip->getClipBounds();

            state->flush();
            s->transparencyLayer = Image (OpenGLImageType().create (Image::ARGB, clipBounds.getWidth(), clipBounds.getHeight(), true));
            s->previousTarget.reset (new Target (state->target));
            state->target = Target (state->target.context, *OpenGLImageType::getFrameBufferFrom (s->transparencyLayer), clipBounds.getPosition());
            s->transparencyLayerAlpha = opacity;
            s->cloneClipIfMultiplyReferenced();

            s->state->target.makeActive();
        }

        return s;
    }

    z0 endTransparencyLayer (SavedState& finishedLayerState)
    {
        if (clip != nullptr)
        {
            jassert (finishedLayerState.previousTarget != nullptr);

            state->flush();
            state->target = *finishedLayerState.previousTarget;
            finishedLayerState.previousTarget.reset();

            state->target.makeActive();
            auto clipBounds = clip->getClipBounds();

            clip->renderImageUntransformed (*this, finishedLayerState.transparencyLayer,
                                            (i32) (finishedLayerState.transparencyLayerAlpha * 255.0f),
                                            clipBounds.getX(), clipBounds.getY(), false);
        }
    }

    Rectangle<i32> getMaximumBounds() const     { return state->target.bounds; }

    z0 setFillType (const FillType& newFill)
    {
        BaseClass::setFillType (newFill);
        state->textureCache.resetGradient();
    }

    //==============================================================================
    template <typename IteratorType>
    z0 renderImageTransformed (IteratorType& iter, const Image& src, i32 alpha,
                                 const AffineTransform& trans, Graphics::ResamplingQuality, b8 tiledFill) const
    {
        state->shaderQuadQueue.flush();
        state->setShaderForTiledImageFill (state->cachedImageList->getTextureFor (src), trans, 0, nullptr, tiledFill);

        state->shaderQuadQueue.add (iter, PixelARGB ((u8) alpha, (u8) alpha, (u8) alpha, (u8) alpha));
        state->shaderQuadQueue.flush();

        state->currentShader.clearShader (state->shaderQuadQueue);
    }

    template <typename IteratorType>
    z0 renderImageUntransformed (IteratorType& iter, const Image& src, i32 alpha, i32 x, i32 y, b8 tiledFill) const
    {
        renderImageTransformed (iter, src, alpha, AffineTransform::translation ((f32) x, (f32) y),
                                Graphics::lowResamplingQuality, tiledFill);
    }

    template <typename IteratorType>
    z0 fillWithSolidColor (IteratorType& iter, PixelARGB colour, b8 replaceContents) const
    {
        if (! isUsingCustomShader)
        {
            state->activeTextures.disableTextures (state->shaderQuadQueue);
            state->blendMode.setBlendMode (state->shaderQuadQueue, replaceContents);
            state->setShader (state->currentShader.programs->solidColorProgram);
        }

        state->shaderQuadQueue.add (iter, colour);
    }

    template <typename IteratorType>
    z0 fillWithGradient (IteratorType& iter, ColorGradient& gradient, const AffineTransform& trans, b8 /*isIdentity*/) const
    {
        state->setShaderForGradientFill (gradient, trans, 0, nullptr);
        state->shaderQuadQueue.add (iter, fillType.colour.getPixelARGB());
    }

    z0 fillRectWithCustomShader (OpenGLRendering::ShaderPrograms::ShaderBase& shader, Rectangle<i32> area)
    {
        state->setShader (shader);
        isUsingCustomShader = true;

        fillRect (area, true);

        isUsingCustomShader = false;
        state->currentShader.clearShader (state->shaderQuadQueue);
    }

    //==============================================================================
    Font font { FontOptions{} };
    GLState* state;
    b8 isUsingCustomShader = false;

private:
    Image transparencyLayer;
    std::unique_ptr<Target> previousTarget;

    SavedState& operator= (const SavedState&);
};


//==============================================================================
struct ShaderContext final : public RenderingHelpers::StackBasedLowLevelGraphicsContext<SavedState>
{
    ShaderContext (const Target& target)  : glState (target)
    {
        stack.initialise (new SavedState (&glState));
    }

    z0 fillRectWithCustomShader (ShaderPrograms::ShaderBase& shader, Rectangle<i32> area)
    {
        static_cast<SavedState&> (*stack).fillRectWithCustomShader (shader, area);
    }

    GLState glState;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShaderContext)
};

struct NonShaderContext final : public LowLevelGraphicsSoftwareRenderer
{
    NonShaderContext (const Target& t, const Image& im)
        : LowLevelGraphicsSoftwareRenderer (im), target (t), image (im)
    {}

    ~NonShaderContext()
    {
        DRX_CHECK_OPENGL_ERROR
        auto previousFrameBufferTarget = OpenGLFrameBuffer::getCurrentFrameBufferTarget();

       #if ! DRX_ANDROID
        target.context.extensions.glActiveTexture (GL_TEXTURE0);

        if (! target.context.isCoreProfile())
            glEnable (GL_TEXTURE_2D);

        clearGLError();
       #endif

        OpenGLTexture texture;
        texture.loadImage (image);
        texture.bind();

        target.makeActive();
        target.context.copyTexture (target.bounds, Rectangle<i32> (texture.getWidth(),
                                                                   texture.getHeight()),
                                    target.bounds.getWidth(), target.bounds.getHeight(),
                                    false);
        glBindTexture (GL_TEXTURE_2D, 0);

       #if DRX_WINDOWS
        if (target.context.extensions.glBindFramebuffer != nullptr)
       #endif
            target.context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, previousFrameBufferTarget);

        DRX_CHECK_OPENGL_ERROR
    }

private:
    Target target;
    Image image;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NonShaderContext)
};

static z0 clearOpenGLGlyphCacheCallback()
{
    RenderingHelpers::GlyphCache::getInstance().reset();
}

static std::unique_ptr<LowLevelGraphicsContext> createOpenGLContext (const Target& target)
{
    clearOpenGLGlyphCache = clearOpenGLGlyphCacheCallback;

    if (target.context.areShadersAvailable())
        return std::make_unique<ShaderContext> (target);

    Image tempImage (Image::ARGB, target.bounds.getWidth(), target.bounds.getHeight(), true, SoftwareImageType());
    return std::make_unique<NonShaderContext> (target, tempImage);
}

}

//==============================================================================
std::unique_ptr<LowLevelGraphicsContext> createOpenGLGraphicsContext (OpenGLContext& context, i32 width, i32 height)
{
    return createOpenGLGraphicsContext (context, context.getFrameBufferID(), width, height);
}

std::unique_ptr<LowLevelGraphicsContext> createOpenGLGraphicsContext (OpenGLContext& context, OpenGLFrameBuffer& target)
{
    return OpenGLRendering::createOpenGLContext (OpenGLRendering::Target (context, target, {}));
}

std::unique_ptr<LowLevelGraphicsContext> createOpenGLGraphicsContext (OpenGLContext& context, u32 frameBufferID, i32 width, i32 height)
{
    return OpenGLRendering::createOpenGLContext (OpenGLRendering::Target (context, frameBufferID, width, height));
}

//==============================================================================
struct CustomProgram final : public ReferenceCountedObject,
                             public OpenGLRendering::ShaderPrograms::ShaderBase
{
    CustomProgram (OpenGLRendering::ShaderContext& c, const Txt& fragmentShader)
        : ShaderBase (c.glState.target.context, fragmentShader.toRawUTF8())
    {
    }

    static ReferenceCountedObjectPtr<CustomProgram> get (const Txt& hashName)
    {
        if (auto* c = OpenGLContext::getCurrentContext())
            if (auto* o = c->getAssociatedObject (hashName.toRawUTF8()))
                return *static_cast<CustomProgram*> (o);

        return {};
    }

    static ReferenceCountedObjectPtr<CustomProgram> getOrCreate (LowLevelGraphicsContext& gc, const Txt& hashName,
                                                                 const Txt& code, Txt& errorMessage)
    {
        if (auto c = get (hashName))
            return c;

        if (auto* sc = dynamic_cast<OpenGLRendering::ShaderContext*> (&gc))
        {
            ReferenceCountedObjectPtr<CustomProgram> c (new CustomProgram (*sc, code));
            errorMessage = c->lastError;

            if (errorMessage.isEmpty())
            {
                if (auto context = OpenGLContext::getCurrentContext())
                {
                    context->setAssociatedObject (hashName.toRawUTF8(), c.get());
                    return c;
                }
            }
        }

        return nullptr;
    }

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomProgram)
};

OpenGLGraphicsContextCustomShader::OpenGLGraphicsContextCustomShader (const Txt& fragmentShaderCode)
    : code (Txt (DRX_DECLARE_VARYING_COLOUR
                    DRX_DECLARE_VARYING_PIXELPOS
                    "\n#define pixelAlpha frontColor.a\n") + fragmentShaderCode),
      hashName (Txt::toHexString (fragmentShaderCode.hashCode64()) + "_shader")
{
}

OpenGLGraphicsContextCustomShader::~OpenGLGraphicsContextCustomShader()
{
    if (OpenGLContext* context = OpenGLContext::getCurrentContext())
        context->setAssociatedObject (hashName.toRawUTF8(), nullptr);
}

OpenGLShaderProgram* OpenGLGraphicsContextCustomShader::getProgram (LowLevelGraphicsContext& gc) const
{
    Txt errorMessage;

    if (auto c = CustomProgram::getOrCreate (gc, hashName, code, errorMessage))
        return &(c->program);

    return {};
}

z0 OpenGLGraphicsContextCustomShader::fillRect (LowLevelGraphicsContext& gc, Rectangle<i32> area) const
{
    Txt errorMessage;

    if (auto sc = dynamic_cast<OpenGLRendering::ShaderContext*> (&gc))
    {
        if (auto c = CustomProgram::getOrCreate (gc, hashName, code, errorMessage))
        {
            c->onShaderActivated = onShaderActivated;
            sc->fillRectWithCustomShader (*c, area);
        }
    }
}

Result OpenGLGraphicsContextCustomShader::checkCompilation (LowLevelGraphicsContext& gc)
{
    Txt errorMessage;

    if (CustomProgram::getOrCreate (gc, hashName, code, errorMessage) != nullptr)
        return Result::ok();

    return Result::fail (errorMessage);
}

} // namespace drx
