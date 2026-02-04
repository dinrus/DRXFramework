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

namespace DragAndDropHelpers
{
    //==============================================================================
    struct DrxDropSource final : public ComBaseClassHelper<IDropSource>
    {
        DrxDropSource() = default;

        DRX_COMRESULT QueryContinueDrag (BOOL escapePressed, DWORD keys) override
        {
            if (escapePressed)
                return DRAGDROP_S_CANCEL;

            if ((keys & (MK_LBUTTON | MK_RBUTTON)) == 0)
                return DRAGDROP_S_DROP;

            return S_OK;
        }

        DRX_COMRESULT GiveFeedback (DWORD) override
        {
            return DRAGDROP_S_USEDEFAULTCURSORS;
        }
    };

    //==============================================================================
    struct DrxEnumFormatEtc final : public ComBaseClassHelper<IEnumFORMATETC>
    {
        DrxEnumFormatEtc (const FORMATETC* f)  : format (f) {}

        DRX_COMRESULT Clone (IEnumFORMATETC** result) override
        {
            if (result == nullptr)
                return E_POINTER;

            auto newOne = new DrxEnumFormatEtc (format);
            newOne->index = index;
            *result = newOne;
            return S_OK;
        }

        DRX_COMRESULT Next (ULONG celt, LPFORMATETC lpFormatEtc, ULONG* pceltFetched) override
        {
            if (pceltFetched != nullptr)
                *pceltFetched = 0;
            else if (celt != 1)
                return S_FALSE;

            if (index == 0 && celt > 0 && lpFormatEtc != nullptr)
            {
                copyFormatEtc (lpFormatEtc [0], *format);
                ++index;

                if (pceltFetched != nullptr)
                    *pceltFetched = 1;

                return S_OK;
            }

            return S_FALSE;
        }

        DRX_COMRESULT Skip (ULONG celt) override
        {
            if (index + (i32) celt >= 1)
                return S_FALSE;

            index += (i32) celt;
            return S_OK;
        }

        DRX_COMRESULT Reset() override
        {
            index = 0;
            return S_OK;
        }

    private:
        const FORMATETC* const format;
        i32 index = 0;

        static z0 copyFormatEtc (FORMATETC& dest, const FORMATETC& source)
        {
            dest = source;

            if (source.ptd != nullptr)
            {
                dest.ptd = (DVTARGETDEVICE*) CoTaskMemAlloc (sizeof (DVTARGETDEVICE));

                if (dest.ptd != nullptr)
                    *(dest.ptd) = *(source.ptd);
            }
        }

        DRX_DECLARE_NON_COPYABLE (DrxEnumFormatEtc)
    };

    //==============================================================================
    class DrxDataObject final : public ComBaseClassHelper<IDataObject>
    {
    public:
        DrxDataObject (const FORMATETC* f, const STGMEDIUM* m)
            : format (f), medium (m)
        {
        }

        ~DrxDataObject() override
        {
            jassert (refCount == 0);
        }

        DRX_COMRESULT GetData (FORMATETC* pFormatEtc, STGMEDIUM* pMedium) override
        {
            if ((pFormatEtc->tymed & format->tymed) != 0
                 && pFormatEtc->cfFormat == format->cfFormat
                 && pFormatEtc->dwAspect == format->dwAspect)
            {
                pMedium->tymed = format->tymed;
                pMedium->pUnkForRelease = nullptr;

                if (format->tymed == TYMED_HGLOBAL)
                {
                    auto len = GlobalSize (medium->hGlobal);
                    uk const src = GlobalLock (medium->hGlobal);
                    uk const dst = GlobalAlloc (GMEM_FIXED, len);

                    if (src != nullptr && dst != nullptr)
                        memcpy (dst, src, len);

                    GlobalUnlock (medium->hGlobal);

                    pMedium->hGlobal = dst;
                    return S_OK;
                }
            }

            return DV_E_FORMATETC;
        }

        DRX_COMRESULT QueryGetData (FORMATETC* f) override
        {
            if (f == nullptr)
                return E_INVALIDARG;

            if (f->tymed == format->tymed
                  && f->cfFormat == format->cfFormat
                  && f->dwAspect == format->dwAspect)
                return S_OK;

            return DV_E_FORMATETC;
        }

        DRX_COMRESULT GetCanonicalFormatEtc (FORMATETC*, FORMATETC* pFormatEtcOut) override
        {
            pFormatEtcOut->ptd = nullptr;
            return E_NOTIMPL;
        }

        DRX_COMRESULT EnumFormatEtc (DWORD direction, IEnumFORMATETC** result) override
        {
            if (result == nullptr)
                return E_POINTER;

            if (direction == DATADIR_GET)
            {
                *result = new DrxEnumFormatEtc (format);
                return S_OK;
            }

            *result = nullptr;
            return E_NOTIMPL;
        }

        DRX_COMRESULT GetDataHere (FORMATETC*, STGMEDIUM*)                  override { return DATA_E_FORMATETC; }
        DRX_COMRESULT SetData (FORMATETC*, STGMEDIUM*, BOOL)                override { return E_NOTIMPL; }
        DRX_COMRESULT DAdvise (FORMATETC*, DWORD, IAdviseSink*, DWORD*)     override { return OLE_E_ADVISENOTSUPPORTED; }
        DRX_COMRESULT DUnadvise (DWORD)                                     override { return E_NOTIMPL; }
        DRX_COMRESULT EnumDAdvise (IEnumSTATDATA**)                         override { return OLE_E_ADVISENOTSUPPORTED; }

    private:
        const FORMATETC* const format;
        const STGMEDIUM* const medium;

        DRX_DECLARE_NON_COPYABLE (DrxDataObject)
    };

    //==============================================================================
    static HDROP createHDrop (const StringArray& fileNames)
    {
        size_t totalBytes = 0;
        for (i32 i = fileNames.size(); --i >= 0;)
            totalBytes += CharPointer_UTF16::getBytesRequiredFor (fileNames[i].getCharPointer()) + sizeof (WCHAR);

        struct Deleter
        {
            z0 operator() (uk ptr) const noexcept { GlobalFree (ptr); }
        };

        auto hDrop = std::unique_ptr<z0, Deleter> ((HDROP) GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, sizeof (DROPFILES) + totalBytes + 4));

        if (hDrop != nullptr)
        {
            auto pDropFiles = (LPDROPFILES) GlobalLock (hDrop.get());

            if (pDropFiles == nullptr)
                return nullptr;

            pDropFiles->pFiles = sizeof (DROPFILES);
            pDropFiles->fWide = true;

            auto* fname = reinterpret_cast<WCHAR*> (addBytesToPointer (pDropFiles, sizeof (DROPFILES)));

            for (i32 i = 0; i < fileNames.size(); ++i)
            {
                auto bytesWritten = fileNames[i].copyToUTF16 (fname, 2048);
                fname = reinterpret_cast<WCHAR*> (addBytesToPointer (fname, bytesWritten));
            }

            *fname = 0;

            GlobalUnlock (hDrop.get());
        }

        return static_cast<HDROP> (hDrop.release());
    }

    struct DragAndDropJob final : public ThreadPoolJob
    {
        DragAndDropJob (FORMATETC f, STGMEDIUM m, DWORD d, std::function<z0()>&& cb)
            : ThreadPoolJob ("DragAndDrop"),
              format (f), medium (m), whatToDo (d),
              completionCallback (std::move (cb))
        {
        }

        JobStatus runJob() override
        {
            [[maybe_unused]] const auto result = OleInitialize (nullptr);

            auto* source = new DrxDropSource();
            auto* data = new DrxDataObject (&format, &medium);

            DWORD effect;
            DoDragDrop (data, source, whatToDo, &effect);

            data->Release();
            source->Release();

            OleUninitialize();

            if (completionCallback != nullptr)
                MessageManager::callAsync (std::move (completionCallback));

            return jobHasFinished;
        }

        FORMATETC format;
        STGMEDIUM medium;
        DWORD whatToDo;

        std::function<z0()> completionCallback;
    };

    class ThreadPoolHolder final : private DeletedAtShutdown
    {
    public:
        ThreadPoolHolder() = default;

        ~ThreadPoolHolder()
        {
            // Wait forever if there's a job running. The user needs to cancel the transfer
            // in the GUI.
            pool.removeAllJobs (true, -1);

            clearSingletonInstance();
        }

        DRX_DECLARE_SINGLETON_SINGLETHREADED_INLINE (ThreadPoolHolder, false)

        // We need to make sure we don't do simultaneous text and file drag and drops,
        // so use a pool that can only run a single job.
        ThreadPool pool { ThreadPoolOptions{}.withNumberOfThreads (1) };
    };
}

//==============================================================================
b8 DragAndDropContainer::performExternalDragDropOfFiles (const StringArray& files, const b8 canMove,
                                                           Component*, std::function<z0()> callback)
{
    if (files.isEmpty())
        return false;

    FORMATETC format = { CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium = { TYMED_HGLOBAL, { nullptr }, nullptr };

    medium.hGlobal = DragAndDropHelpers::createHDrop (files);

    auto& pool = DragAndDropHelpers::ThreadPoolHolder::getInstance()->pool;
    pool.addJob (new DragAndDropHelpers::DragAndDropJob (format, medium,
                                                         canMove ? (DROPEFFECT_COPY | DROPEFFECT_MOVE) : DROPEFFECT_COPY,
                                                         std::move (callback)),
                true);

    return true;
}

b8 DragAndDropContainer::performExternalDragDropOfText (const Txt& text, Component*, std::function<z0()> callback)
{
    if (text.isEmpty())
        return false;

    FORMATETC format = { CF_TEXT, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    STGMEDIUM medium = { TYMED_HGLOBAL, { nullptr }, nullptr };

    auto numBytes = CharPointer_UTF16::getBytesRequiredFor (text.getCharPointer());

    medium.hGlobal = GlobalAlloc (GMEM_MOVEABLE | GMEM_ZEROINIT, numBytes + 2);

    if (medium.hGlobal == nullptr)
        return false;

    auto* data = static_cast<WCHAR*> (GlobalLock (medium.hGlobal));

    text.copyToUTF16 (data, numBytes + 2);
    format.cfFormat = CF_UNICODETEXT;

    GlobalUnlock (medium.hGlobal);

    auto& pool = DragAndDropHelpers::ThreadPoolHolder::getInstance()->pool;
    pool.addJob (new DragAndDropHelpers::DragAndDropJob (format,
                                                        medium,
                                                        DROPEFFECT_COPY | DROPEFFECT_MOVE,
                                                        std::move (callback)),
                 true);

    return true;
}

} // namespace drx
