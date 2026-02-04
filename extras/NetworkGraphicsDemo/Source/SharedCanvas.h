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


/**
    This scene description is broadcast to all the clients, and contains a list of all
    the clients involved, as well as the set of shapes to be drawn.

    Each client will draw the part of the path that lies within its own area. It can
    find its area by looking at the list of clients contained in this structure.

    All the path coordinates are roughly in units of inches, and devices will convert
    this to pixels based on their screen size and DPI
*/
struct SharedCanvasDescription
{
    SharedCanvasDescription() {}

    Color backgroundColor = Colors::black;

    struct ColoredPath
    {
        Path path;
        FillType fill;
    };

    Array<ColoredPath> paths;

    struct ClientArea
    {
        Txt name;
        Point<f32> centre; // in inches
        f32 scaleFactor; // extra scaling
    };

    Array<ClientArea> clients;

    //==============================================================================
    z0 reset()
    {
        paths.clearQuick();
        clients.clearQuick();
    }

    z0 swapWith (SharedCanvasDescription& other)
    {
        std::swap (backgroundColor, other.backgroundColor);
        paths.swapWith (other.paths);
        clients.swapWith (other.clients);
    }

    // This is a fixed size that represents the overall canvas limits that
    // content should lie within
    Rectangle<f32> getLimits() const
    {
        f32 inchesX = 60.0f;
        f32 inchesY = 30.0f;

        return { inchesX * -0.5f, inchesY * -0.5f, inchesX, inchesY };
    }

    //==============================================================================
    z0 draw (Graphics& g, Rectangle<f32> targetArea, Rectangle<f32> clientArea) const
    {
        draw (g, clientArea,
              AffineTransform::fromTargetPoints (clientArea.getX(),     clientArea.getY(),
                                                 targetArea.getX(),     targetArea.getY(),
                                                 clientArea.getRight(), clientArea.getY(),
                                                 targetArea.getRight(), targetArea.getY(),
                                                 clientArea.getRight(), clientArea.getBottom(),
                                                 targetArea.getRight(), targetArea.getBottom()));
    }

    z0 draw (Graphics& g, Rectangle<f32> clientArea, AffineTransform t) const
    {
        g.saveState();
        g.addTransform (t);

        for (const auto& p : paths)
        {
            if (p.path.getBounds().intersects (clientArea))
            {
                g.setFillType (p.fill);
                g.fillPath (p.path);
            }
        }

        g.restoreState();
    }

    const ClientArea* findClient (const Txt& clientName) const
    {
        for (const auto& c : clients)
            if (c.name == clientName)
                return &c;

        return nullptr;
    }

    //==============================================================================
    // Serialisation...

    z0 save (OutputStream& out) const
    {
        out.writeInt (magic);
        out.writeInt ((i32) backgroundColor.getARGB());

        out.writeInt (clients.size());

        for (const auto& c : clients)
        {
            out.writeString (c.name);
            writePoint (out, c.centre);
            out.writeFloat (c.scaleFactor);
        }

        out.writeInt (paths.size());

        for (const auto& p : paths)
        {
            writeFill (out, p.fill);
            p.path.writePathToStream (out);
        }
    }

    z0 load (InputStream& in)
    {
        if (in.readInt() != magic)
            return;

        backgroundColor = Color ((u32) in.readInt());

        {
            i32k numClients = in.readInt();
            clients.clearQuick();

            for (i32 i = 0; i < numClients; ++i)
            {
                ClientArea c;
                c.name = in.readString();
                c.centre = readPoint (in);
                c.scaleFactor = in.readFloat();
                clients.add (c);
            }
        }

        {
            i32k numPaths = in.readInt();
            paths.clearQuick();

            for (i32 i = 0; i < numPaths; ++i)
            {
                ColoredPath p;
                p.fill = readFill (in);
                p.path.loadPathFromStream (in);

                paths.add (std::move (p));
            }
        }
    }

    MemoryBlock toMemoryBlock() const
    {
        MemoryOutputStream o;
        save (o);
        return o.getMemoryBlock();
    }

private:
    //==============================================================================
    static z0 writePoint (OutputStream& out, Point<f32> p)
    {
        out.writeFloat (p.x);
        out.writeFloat (p.y);
    }

    static z0 writeRect (OutputStream& out, Rectangle<f32> r)
    {
        writePoint (out, r.getPosition());
        out.writeFloat (r.getWidth());
        out.writeFloat (r.getHeight());
    }

    static Point<f32> readPoint (InputStream& in)
    {
        Point<f32> p;
        p.x = in.readFloat();
        p.y = in.readFloat();
        return p;
    }

    static Rectangle<f32> readRect (InputStream& in)
    {
        Rectangle<f32> r;
        r.setPosition (readPoint (in));
        r.setWidth (in.readFloat());
        r.setHeight (in.readFloat());
        return r;
    }

    static z0 writeFill (OutputStream& out, const FillType& f)
    {
        if (f.isColor())
        {
            out.writeByte (0);
            out.writeInt ((i32) f.colour.getARGB());
        }
        else if (f.isGradient())
        {
            const ColorGradient& cg = *f.gradient;
            jassert (cg.getNumColors() >= 2);

            out.writeByte (cg.isRadial ? 2 : 1);

            writePoint (out, cg.point1);
            writePoint (out, cg.point2);

            out.writeCompressedInt (cg.getNumColors());

            for (i32 i = 0; i < cg.getNumColors(); ++i)
            {
                out.writeDouble (cg.getColorPosition (i));
                out.writeInt ((i32) cg.getColor (i).getARGB());
            }
        }
        else
        {
            jassertfalse;
        }
    }

    static FillType readFill (InputStream& in)
    {
        i32 type = in.readByte();

        if (type == 0)
            return FillType (Color ((u32) in.readInt()));

        if (type > 2)
        {
            jassertfalse;
            return FillType();
        }

        ColorGradient cg;
        cg.point1 = readPoint (in);
        cg.point2 = readPoint (in);

        cg.clearColors();

        i32 numColors = in.readCompressedInt();

        for (i32 i = 0; i < numColors; ++i)
        {
            const f64 pos = in.readDouble();
            cg.addColor (pos, Color ((u32) in.readInt()));
        }

        jassert (cg.getNumColors() >= 2);

        return FillType (cg);
    }

    i32k magic = 0x2381239a;

    DRX_DECLARE_NON_COPYABLE (SharedCanvasDescription)
};

//==============================================================================
class CanvasGeneratingContext final : public LowLevelGraphicsContext
{
public:
    CanvasGeneratingContext (SharedCanvasDescription& c)  : canvas (c)
    {
        stateStack.add (new SavedState());
    }

    //==============================================================================
    b8 isVectorDevice() const override               { return true; }
    f32 getPhysicalPixelScaleFactor() const override { return 1.0f; }
    zu64 getFrameId() const override               { return 0; }
    z0 setOrigin (Point<i32> o) override             { addTransform (AffineTransform::translation ((f32) o.x, (f32) o.y)); }

    z0 addTransform (const AffineTransform& t) override
    {
        getState().transform = t.followedBy (getState().transform);
    }

    b8 clipToRectangle (const Rectangle<i32>&) override                   { return true; }
    b8 clipToRectangleList (const RectangleList<i32>&) override           { return true; }
    z0 excludeClipRectangle (const Rectangle<i32>&) override              {}
    z0 clipToPath (const Path&, const AffineTransform&) override          {}
    z0 clipToImageAlpha (const Image&, const AffineTransform&) override   {}

    z0 saveState() override
    {
        stateStack.add (new SavedState (getState()));
    }

    z0 restoreState() override
    {
        jassert (stateStack.size() > 0);

        if (stateStack.size() > 0)
            stateStack.removeLast();
    }

    z0 beginTransparencyLayer (f32 alpha) override
    {
        saveState();
        getState().transparencyLayer = new SharedCanvasHolder();
        getState().transparencyOpacity = alpha;
    }

    z0 endTransparencyLayer() override
    {
        const ReferenceCountedObjectPtr<SharedCanvasHolder> finishedTransparencyLayer (getState().transparencyLayer);
        f32 alpha = getState().transparencyOpacity;
        restoreState();

        if (SharedCanvasHolder* c = finishedTransparencyLayer)
        {
            for (auto& path : c->canvas.paths)
            {
                path.fill.setOpacity (path.fill.getOpacity() * alpha);
                getTargetCanvas().paths.add (path);
            }
        }
    }

    Rectangle<i32> getClipBounds() const override
    {
        return canvas.getLimits().getSmallestIntegerContainer()
                .transformedBy (getState().transform.inverted());
    }

    b8 clipRegionIntersects (const Rectangle<i32>&) override      { return true; }
    b8 isClipEmpty() const override                               { return false; }

    //==============================================================================
    z0 setFill (const FillType& fillType) override                { getState().fillType = fillType; }
    z0 setOpacity (f32 op) override                             { getState().fillType.setOpacity (op); }
    z0 setInterpolationQuality (Graphics::ResamplingQuality) override {}

    //==============================================================================
    z0 fillRect (const Rectangle<i32>& r, b8) override          { fillRect (r.toFloat()); }
    z0 fillRectList (const RectangleList<f32>& list) override   { fillPath (list.toPath(), AffineTransform()); }

    z0 fillRect (const Rectangle<f32>& r) override
    {
        Path p;
        p.addRectangle (r.toFloat());
        fillPath (p, AffineTransform());
    }

    z0 fillPath (const Path& p, const AffineTransform& t) override
    {
        Path p2 (p);
        p2.applyTransform (t.followedBy (getState().transform));

        getTargetCanvas().paths.add ({ std::move (p2), getState().fillType });
    }

    z0 drawImage (const Image&, const AffineTransform&) override {}

    z0 drawLine (const Line<f32>& line) override
    {
        Path p;
        p.addLineSegment (line, 1.0f);
        fillPath (p, AffineTransform());
    }

    //==============================================================================
    const Font& getFont() override                  { return getState().font; }
    z0 setFont (const Font& newFont) override     { getState().font = newFont; }

    z0 drawGlyphs (Span<u16k_t> indices,
                     Span<const Point<f32>> positions,
                     const AffineTransform& transform) override
    {
        std::unordered_map<u16_t, Path> cache;

        const auto& font = getState().font;

        for (size_t i = 0; i < indices.size(); i++)
        {
            const auto glyphNumber = indices[i];
            const auto pos         = positions[i];
            auto& path             = cache[glyphNumber];

            if (path.isEmpty())
                font.getTypefacePtr()->getOutlineForGlyph (TypefaceMetricsKind::legacy, glyphNumber, path);

            auto t = AffineTransform::scale (font.getHeight() * font.getHorizontalScale(), font.getHeight())
                                     .followedBy (AffineTransform::translation (pos))
                                     .followedBy (transform);
            fillPath (path, t);
        }
    }

private:
    //==============================================================================
    struct SharedCanvasHolder final : public ReferenceCountedObject
    {
        SharedCanvasDescription canvas;
    };

    struct SavedState
    {
        FillType fillType;
        AffineTransform transform;
        Font font { FontOptions{} };
        ReferenceCountedObjectPtr<SharedCanvasHolder> transparencyLayer;
        f32 transparencyOpacity = 1.0f;
    };

    SharedCanvasDescription& getTargetCanvas() const
    {
        if (SharedCanvasHolder* c = getState().transparencyLayer)
            return c->canvas;

        return canvas;
    }

    SavedState& getState() const noexcept
    {
        jassert (stateStack.size() > 0);
        return *stateStack.getLast();
    }

    SharedCanvasDescription& canvas;
    OwnedArray<SavedState> stateStack;

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CanvasGeneratingContext)
};

//==============================================================================
/** Helper for breaking and reassembling a memory block into smaller checksummed
    blocks that will fit inside UDP packets
*/
struct BlockPacketiser
{
    z0 createBlocksFromData (const MemoryBlock& data, size_t maxBlockSize)
    {
        jassert (blocks.size() == 0);

        i32 offset = 0;
        size_t remaining = data.getSize();

        while (remaining > 0)
        {
            auto num = (size_t) jmin (maxBlockSize, remaining);
            blocks.add (MemoryBlock (addBytesToPointer (data.getData(), offset), num));
            offset += (i32) num;
            remaining -= num;
        }

        MemoryOutputStream checksumBlock;
        checksumBlock << getLastPacketPrefix() << MD5 (data).toHexString() << (t8) 0 << (t8) 0;
        blocks.add (checksumBlock.getMemoryBlock());

        for (i32 i = 0; i < blocks.size(); ++i)
        {
            auto index = (u32) ByteOrder::swapIfBigEndian (i);
            blocks.getReference (i).append (&index, sizeof (index));
        }
    }

    // returns true if this is an end-of-sequence block
    b8 appendIncomingBlock (MemoryBlock data)
    {
        if (data.getSize() > 4)
            blocks.addSorted (*this, data);

        return Txt (CharPointer_ASCII ((tukk) data.getData())).startsWith (getLastPacketPrefix());
    }

    b8 reassemble (MemoryBlock& result)
    {
        result.reset();

        if (blocks.size() > 1)
        {
            for (i32 i = 0; i < blocks.size() - 1; ++i)
                result.append (blocks.getReference (i).getData(), blocks.getReference (i).getSize() - 4);

            Txt storedMD5 (Txt (CharPointer_ASCII ((tukk) blocks.getLast().getData()))
                                .fromFirstOccurrenceOf (getLastPacketPrefix(), false, false));

            blocks.clearQuick();

            if (MD5 (result).toHexString().trim().equalsIgnoreCase (storedMD5.trim()))
                return true;
        }

        result.reset();
        return false;
    }

    static i32 compareElements (const MemoryBlock& b1, const MemoryBlock& b2)
    {
        auto i1 = ByteOrder::littleEndianInt (addBytesToPointer (b1.getData(), b1.getSize() - 4));
        auto i2 = ByteOrder::littleEndianInt (addBytesToPointer (b2.getData(), b2.getSize() - 4));
        return (i32) (i1 - i2);
    }

    static tukk getLastPacketPrefix()   { return "**END_OF_PACKET_LIST** "; }

    Array<MemoryBlock> blocks;
};


//==============================================================================
struct AnimatedContent
{
    virtual ~AnimatedContent() {}

    virtual Txt getName() const = 0;
    virtual z0 reset() = 0;
    virtual z0 generateCanvas (Graphics&, SharedCanvasDescription& canvas, Rectangle<f32> activeArea) = 0;
    virtual z0 handleTouch (Point<f32> position) = 0;
};
