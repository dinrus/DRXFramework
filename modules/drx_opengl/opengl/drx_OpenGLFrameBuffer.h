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

//==============================================================================
/**
    Creates an openGL frame buffer.

    @tags{OpenGL}
*/
class DRX_API  OpenGLFrameBuffer
{
public:
    /** Creates an uninitialised buffer.
        To actually allocate the buffer, use initialise().
    */
    OpenGLFrameBuffer();

    /** Destructor. */
    ~OpenGLFrameBuffer();

    //==============================================================================
    /** Tries to allocates a buffer of the given size.
        Note that a valid openGL context must be selected when you call this method,
        or it will fail.
    */
    b8 initialise (OpenGLContext& context, i32 width, i32 height);

    /** Tries to allocates a buffer containing a copy of a given image.
        Note that a valid openGL context must be selected when you call this method,
        or it will fail.
    */
    b8 initialise (OpenGLContext& context, const Image& content);

    /** Tries to allocate a copy of another framebuffer.
    */
    b8 initialise (OpenGLFrameBuffer& other);

    /** Releases the buffer, if one has been allocated.
        Any saved state that was created with saveAndRelease() will also be freed by this call.
    */
    z0 release();

    /** If the framebuffer is active, this will save a stashed copy of its contents in main memory,
        and will release the GL buffer.
        After saving, the original state can be restored again by calling reloadSavedCopy().
    */
    z0 saveAndRelease();

    /** Restores the framebuffer content that was previously saved using saveAndRelease().
        After saving to main memory, the original state can be restored by calling restoreToGPUMemory().
    */
    b8 reloadSavedCopy (OpenGLContext& context);

    //==============================================================================
    /** Возвращает true, если a valid buffer has been allocated. */
    b8 isValid() const noexcept                       { return pimpl != nullptr; }

    /** Returns the width of the buffer. */
    i32 getWidth() const noexcept;

    /** Returns the height of the buffer. */
    i32 getHeight() const noexcept;

    /** Returns the texture ID number for using this buffer as a texture. */
    GLuint getTextureID() const noexcept;

    //==============================================================================
    /** Selects this buffer as the current OpenGL rendering target. */
    b8 makeCurrentRenderingTarget();

    /** Deselects this buffer as the current OpenGL rendering target. */
    z0 releaseAsRenderingTarget();

    /** Returns the ID of this framebuffer, or 0 if it isn't initialised. */
    GLuint getFrameBufferID() const noexcept;

    /** Returns the current frame buffer ID for the current context. */
    static GLuint getCurrentFrameBufferTarget() noexcept;

    /** Clears the framebuffer with the specified colour. */
    z0 clear (Color colour);

    /** Selects the framebuffer as the current target, and clears it to transparent. */
    z0 makeCurrentAndClear();

    /** Reads an area of pixels from the framebuffer into a 32-bit ARGB pixel array.
        The lineStride is measured as a number of pixels, not bytes - pass a stride
        of 0 to indicate a packed array.
    */
    b8 readPixels (PixelARGB* targetData, const Rectangle<i32>& sourceArea);

    /** Writes an area of pixels into the framebuffer from a specified pixel array.
        The lineStride is measured as a number of pixels, not bytes - pass a stride
        of 0 to indicate a packed array.
    */
    b8 writePixels (const PixelARGB* srcData, const Rectangle<i32>& targetArea);

private:
    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    class SavedState;
    std::unique_ptr<SavedState> savedState;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLFrameBuffer)
};

} // namespace drx
