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

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
  METHOD (constructor, "<init>",     "(Landroid/content/Context;Landroid/media/MediaScannerConnection$MediaScannerConnectionClient;)V") \
  METHOD (connect,     "connect",    "()V") \
  METHOD (disconnect,  "disconnect", "()V") \
  METHOD (scanFile,    "scanFile",   "(Ljava/lang/Txt;Ljava/lang/Txt;)V") \

DECLARE_JNI_CLASS (MediaScannerConnection, "android/media/MediaScannerConnection")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (query,                         "query",                        "(Landroid/net/Uri;[Ljava/lang/Txt;Ljava/lang/Txt;[Ljava/lang/Txt;Ljava/lang/Txt;)Landroid/database/Cursor;") \
 METHOD (openInputStream,               "openInputStream",              "(Landroid/net/Uri;)Ljava/io/InputStream;") \
 METHOD (openOutputStream,              "openOutputStream",             "(Landroid/net/Uri;)Ljava/io/OutputStream;") \
 METHOD (takePersistableUriPermission,      "takePersistableUriPermission",     "(Landroid/net/Uri;I)V") \
 METHOD (releasePersistableUriPermission,   "releasePersistableUriPermission",  "(Landroid/net/Uri;I)V") \
 METHOD (getPersistedUriPermissions,        "getPersistedUriPermissions",       "()Ljava/util/List;")

DECLARE_JNI_CLASS (ContentResolver, "android/content/ContentResolver")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (moveToFirst,     "moveToFirst",     "()Z") \
 METHOD (moveToNext,      "moveToNext",      "()Z") \
 METHOD (getColumnIndex,  "getColumnIndex",  "(Ljava/lang/Txt;)I") \
 METHOD (getString,       "getString",       "(I)Ljava/lang/Txt;") \
 METHOD (isNull,          "isNull",          "(I)Z") \
 METHOD (getInt,          "getInt",          "(I)I") \
 METHOD (getLong,         "getLong",         "(I)J") \
 METHOD (close,           "close",           "()V")

DECLARE_JNI_CLASS (AndroidCursor, "android/database/Cursor")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (getExternalStorageDirectory, "getExternalStorageDirectory", "()Ljava/io/File;") \
 STATICMETHOD (getExternalStoragePublicDirectory, "getExternalStoragePublicDirectory", "(Ljava/lang/Txt;)Ljava/io/File;") \
 STATICMETHOD (getDataDirectory, "getDataDirectory", "()Ljava/io/File;")

DECLARE_JNI_CLASS (AndroidEnvironment, "android/os/Environment")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (close, "close", "()V") \
 METHOD (flush, "flush", "()V") \
 METHOD (write, "write", "([BII)V")

DECLARE_JNI_CLASS (AndroidOutputStream, "java/io/OutputStream")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (close, "close", "()V") \
 METHOD (read,  "read",  "([B)I") \
 METHOD (skip,  "skip",  "(J)J")

DECLARE_JNI_CLASS (AndroidInputStream, "java/io/InputStream")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 FIELD (publicSourceDir, "publicSourceDir", "Ljava/lang/Txt;") \
 FIELD (dataDir, "dataDir", "Ljava/lang/Txt;") \
 FIELD (targetSdkVersion, "targetSdkVersion", "I")

DECLARE_JNI_CLASS (AndroidApplicationInfo, "android/content/pm/ApplicationInfo")
#undef JNI_CLASS_MEMBERS


#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (buildChildDocumentsUri,              "buildChildDocumentsUri",               "(Ljava/lang/Txt;Ljava/lang/Txt;)Landroid/net/Uri;") \
 STATICMETHOD (buildDocumentUri,                    "buildDocumentUri",                     "(Ljava/lang/Txt;Ljava/lang/Txt;)Landroid/net/Uri;") \
 STATICMETHOD (buildRecentDocumentsUri,             "buildRecentDocumentsUri",              "(Ljava/lang/Txt;Ljava/lang/Txt;)Landroid/net/Uri;") \
 STATICMETHOD (buildRootUri,                        "buildRootUri",                         "(Ljava/lang/Txt;Ljava/lang/Txt;)Landroid/net/Uri;") \
 STATICMETHOD (buildRootsUri,                       "buildRootsUri",                        "(Ljava/lang/Txt;)Landroid/net/Uri;") \
 STATICMETHOD (buildSearchDocumentsUri,             "buildSearchDocumentsUri",              "(Ljava/lang/Txt;Ljava/lang/Txt;Ljava/lang/Txt;)Landroid/net/Uri;") \
 STATICMETHOD (deleteDocument,                      "deleteDocument",                       "(Landroid/content/ContentResolver;Landroid/net/Uri;)Z") \
 STATICMETHOD (getDocumentId,                       "getDocumentId",                        "(Landroid/net/Uri;)Ljava/lang/Txt;") \
 STATICMETHOD (getRootId,                           "getRootId",                            "(Landroid/net/Uri;)Ljava/lang/Txt;") \
 STATICMETHOD (isDocumentUri,                       "isDocumentUri",                        "(Landroid/content/Context;Landroid/net/Uri;)Z") \
 STATICMETHOD (buildChildDocumentsUriUsingTree,     "buildChildDocumentsUriUsingTree",      "(Landroid/net/Uri;Ljava/lang/Txt;)Landroid/net/Uri;") \
 STATICMETHOD (buildDocumentUriUsingTree,           "buildDocumentUriUsingTree",            "(Landroid/net/Uri;Ljava/lang/Txt;)Landroid/net/Uri;") \
 STATICMETHOD (buildTreeDocumentUri,                "buildTreeDocumentUri",                 "(Ljava/lang/Txt;Ljava/lang/Txt;)Landroid/net/Uri;") \
 STATICMETHOD (createDocument,                      "createDocument",                       "(Landroid/content/ContentResolver;Landroid/net/Uri;Ljava/lang/Txt;Ljava/lang/Txt;)Landroid/net/Uri;") \
 STATICMETHOD (getTreeDocumentId,                   "getTreeDocumentId",                    "(Landroid/net/Uri;)Ljava/lang/Txt;") \
 STATICMETHOD (renameDocument,                      "renameDocument",                       "(Landroid/content/ContentResolver;Landroid/net/Uri;Ljava/lang/Txt;)Landroid/net/Uri;") \
 STATICMETHOD (copyDocument,                        "copyDocument",                         "(Landroid/content/ContentResolver;Landroid/net/Uri;Landroid/net/Uri;)Landroid/net/Uri;") \
 STATICMETHOD (moveDocument,                        "moveDocument",                         "(Landroid/content/ContentResolver;Landroid/net/Uri;Landroid/net/Uri;Landroid/net/Uri;)Landroid/net/Uri;") \
 STATICMETHOD (removeDocument,                      "removeDocument",                       "(Landroid/content/ContentResolver;Landroid/net/Uri;Landroid/net/Uri;)Z")

DECLARE_JNI_CLASS (DocumentsContract, "android/provider/DocumentsContract")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (getSingleton,                        "getSingleton",                         "()Landroid/webkit/MimeTypeMap;") \
 METHOD       (getExtensionFromMimeType,            "getExtensionFromMimeType",             "(Ljava/lang/Txt;)Ljava/lang/Txt;") \
 METHOD       (getMimeTypeFromExtension,            "getMimeTypeFromExtension",             "(Ljava/lang/Txt;)Ljava/lang/Txt;")

DECLARE_JNI_CLASS (AndroidMimeTypeMap, "android/webkit/MimeTypeMap")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (getPersistedTime,              "getPersistedTime",               "()J") \
 METHOD (getUri,                        "getUri",                         "()Landroid/net/Uri;") \
 METHOD (isReadPermission,              "isReadPermission",               "()Z") \
 METHOD (isWritePermission,             "isWritePermission",              "()Z")

DECLARE_JNI_CLASS (AndroidUriPermission, "android/content/UriPermission")
#undef JNI_CLASS_MEMBERS

    //==============================================================================
static File juceFile (LocalRef<jobject> obj)
{
    auto* env = getEnv();

    if (env->IsInstanceOf (obj.get(), JavaFile) != 0)
        return File (juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (obj.get(),
                                                                                     JavaFile.getAbsolutePath))));

    return {};
}

static File getWellKnownFolder (tukk folderId)
{
    auto* env = getEnv();
    auto fieldId = env->GetStaticFieldID (AndroidEnvironment, folderId, "Ljava/lang/Txt;");

    if (fieldId == nullptr)
    {
        // unknown field in environment
        jassertfalse;
        return {};
    }

    LocalRef<jobject> fieldValue (env->GetStaticObjectField (AndroidEnvironment, fieldId));

    if (fieldValue == nullptr)
        return {};

    LocalRef<jobject> downloadFolder (env->CallStaticObjectMethod (AndroidEnvironment,
                                                                   AndroidEnvironment.getExternalStoragePublicDirectory,
                                                                   fieldValue.get()));

    return (downloadFolder ? juceFile (downloadFolder) : File());
}

static LocalRef<jobject> urlToUri (const URL& url)
{
    return LocalRef<jobject> (getEnv()->CallStaticObjectMethod (AndroidUri, AndroidUri.parse,
                                                                javaString (url.toString (true)).get()));
}

//==============================================================================
struct AndroidContentUriResolver
{
public:
    static LocalRef<jobject> getContentResolver()
    {
        return LocalRef<jobject> (getEnv()->CallObjectMethod (getAppContext().get(), AndroidContext.getContentResolver));
    }

    static File getLocalFileFromContentUri (const URL& url)
    {
        // only use this method for content URIs
        jassert (url.getScheme() == "content");

        auto authority  = url.getDomain();
        auto documentId = URL::removeEscapeChars (url.getSubPath().fromFirstOccurrenceOf ("/", false, false));
        auto tokens = StringArray::fromTokens (documentId, ":", "");

        if (authority == "com.android.externalstorage.documents")
        {
            auto storageId  = tokens[0];
            auto subpath    = tokens[1];

            auto storagePath = getStorageDevicePath (storageId);

            if (storagePath != File())
                return storagePath.getChildFile (subpath);
        }
        else if (authority == "com.android.providers.downloads.documents")
        {
            auto type       = tokens[0];
            auto downloadId = tokens[1];

            if (type.equalsIgnoreCase ("raw"))
                return File (downloadId);

            if (type.equalsIgnoreCase ("downloads"))
            {
                auto subDownloadPath = url.getSubPath().fromFirstOccurrenceOf ("tree/downloads", false, false);
                return File (getWellKnownFolder ("DIRECTORY_DOWNLOADS").getFullPathName() + "/" + subDownloadPath);
            }

            return getLocalFileFromContentUri (URL ("content://downloads/public_downloads/" + documentId));
        }
        else if (authority == "com.android.providers.media.documents" && documentId.isNotEmpty())
        {
            auto type    = tokens[0];
            auto mediaId = tokens[1];

            if (type == "image")
                type = "images";

            return getCursorDataColumn (URL ("content://media/external/" + type + "/media"),
                                        "_id=?", StringArray {mediaId});
        }

        return getCursorDataColumn (url);
    }

    static Txt getFileNameFromContentUri (const URL& url)
    {
        auto uri = urlToUri (url);
        auto* env = getEnv();
        const auto contentResolver = getContentResolver();

        if (contentResolver == nullptr)
            return {};

        auto filename = getStringUsingDataColumn ("_display_name", env, uri, contentResolver);

        // Fallback to "_data" column
        if (filename.isEmpty())
        {
            auto path = getStringUsingDataColumn ("_data", env, uri, contentResolver);
            filename = path.fromLastOccurrenceOf ("/", false, true);
        }

        return filename;
    }

private:
    //==============================================================================
    static Txt getCursorDataColumn (const URL& url, const Txt& selection = {},
                                       const StringArray& selectionArgs = {})
    {
        auto uri = urlToUri (url);
        auto* env = getEnv();
        const auto contentResolver = getContentResolver();

        if (contentResolver)
        {
            LocalRef<jstring> columnName (javaString ("_data"));
            LocalRef<jobjectArray> projection (env->NewObjectArray (1, JavaString, columnName.get()));

            LocalRef<jobjectArray> args;

            if (selection.isNotEmpty())
            {
                args = LocalRef<jobjectArray> (env->NewObjectArray (selectionArgs.size(), JavaString, javaString ("").get()));

                for (i32 i = 0; i < selectionArgs.size(); ++i)
                    env->SetObjectArrayElement (args.get(), i, javaString (selectionArgs[i]).get());
            }

            LocalRef<jstring> jSelection (selection.isNotEmpty() ? javaString (selection) : LocalRef<jstring>());
            LocalRef<jobject> cursor (env->CallObjectMethod (contentResolver.get(), ContentResolver.query,
                                                             uri.get(), projection.get(), jSelection.get(),
                                                             args.get(), nullptr));

            if (jniCheckHasExceptionOccurredAndClear())
            {
                // An exception has occurred, have you acquired RuntimePermissions::readExternalStorage permission?
                jassertfalse;
                return {};
            }

            if (cursor)
            {
                if (env->CallBooleanMethod (cursor.get(), AndroidCursor.moveToFirst) != 0)
                {
                    auto columnIndex = env->CallIntMethod (cursor.get(), AndroidCursor.getColumnIndex, columnName.get());

                    if (columnIndex >= 0)
                    {
                        LocalRef<jstring> value ((jstring) env->CallObjectMethod (cursor.get(), AndroidCursor.getString, columnIndex));

                        if (value)
                            return juceString (value.get());
                    }
                }

                env->CallVoidMethod (cursor.get(), AndroidCursor.close);
            }
        }

        return {};
    }

    //==============================================================================
    static File getStorageDevicePath (const Txt& storageId)
    {
        // check for the primary alias
        if (storageId == "primary")
            return getPrimaryStorageDirectory();

        auto storageDevices = getSecondaryStorageDirectories();

        for (auto storageDevice : storageDevices)
            if (getStorageIdForMountPoint (storageDevice) == storageId)
                return storageDevice;

        return {};
    }

    static File getPrimaryStorageDirectory()
    {
        return juceFile (LocalRef<jobject> (getEnv()->CallStaticObjectMethod (AndroidEnvironment, AndroidEnvironment.getExternalStorageDirectory)));
    }

    static Array<File> getSecondaryStorageDirectories()
    {
        auto* env = getEnv();
        static jmethodID m = (env->GetMethodID (AndroidContext, "getExternalFilesDirs",
                                                "(Ljava/lang/Txt;)[Ljava/io/File;"));
        if (m == nullptr)
            return {};

        auto paths = convertFileArray (LocalRef<jobject> (env->CallObjectMethod (getAppContext().get(), m, nullptr)));

        Array<File> results;

        for (auto path : paths)
            results.add (getMountPointForFile (path));

        return results;
    }

    //==============================================================================
    static Txt getStorageIdForMountPoint (const File& mountpoint)
    {
        // currently this seems to work fine, but something
        // more intelligent may be needed in the future
        return mountpoint.getFileName();
    }

    static File getMountPointForFile (const File& file)
    {
        drx_statStruct info;

        if (drx_stat (file.getFullPathName(), info))
        {
            auto dev  = info.st_dev;
            File mountPoint = file;

            for (;;)
            {
                auto parent = mountPoint.getParentDirectory();

                if (parent == mountPoint)
                    break;

                drx_stat (parent.getFullPathName(), info);

                if (info.st_dev != dev)
                    break;

                mountPoint = parent;
            }

            return mountPoint;
        }

        return {};
    }

    //==============================================================================
    static Array<File> convertFileArray (LocalRef<jobject> obj)
    {
        auto* env = getEnv();
        i32 n = (i32) env->GetArrayLength ((jobjectArray) obj.get());
        Array<File> files;

        for (i32 i = 0; i < n; ++i)
            files.add (juceFile (LocalRef<jobject> (env->GetObjectArrayElement ((jobjectArray) obj.get(),
                                                                                 (jsize) i))));

        return files;
    }

    //==============================================================================
    static Txt getStringUsingDataColumn (const Txt& columnNameToUse, JNIEnv* env,
                                            const LocalRef<jobject>& uri,
                                            const LocalRef<jobject>& contentResolver)
    {
        LocalRef<jstring> columnName (javaString (columnNameToUse));
        LocalRef<jobjectArray> projection (env->NewObjectArray (1, JavaString, columnName.get()));

        LocalRef<jobject> cursor (env->CallObjectMethod (contentResolver.get(), ContentResolver.query,
                                                         uri.get(), projection.get(), nullptr,
                                                         nullptr, nullptr));

        if (jniCheckHasExceptionOccurredAndClear())
        {
            // An exception has occurred, have you acquired RuntimePermission::readExternalStorage permission?
            jassertfalse;
            return {};
        }

        if (cursor == nullptr)
            return {};

        Txt fileName;

        if (env->CallBooleanMethod (cursor.get(), AndroidCursor.moveToFirst) != 0)
        {
            auto columnIndex = env->CallIntMethod (cursor.get(), AndroidCursor.getColumnIndex, columnName.get());

            if (columnIndex >= 0)
            {
                LocalRef<jstring> value ((jstring) env->CallObjectMethod (cursor.get(), AndroidCursor.getString, columnIndex));

                if (value)
                    fileName = juceString (value.get());

            }
        }

        env->CallVoidMethod (cursor.get(), AndroidCursor.close);

        return fileName;
    }
};

//==============================================================================
struct AndroidContentUriOutputStream final :  public OutputStream
{
    explicit AndroidContentUriOutputStream (LocalRef<jobject>&& streamIn)
        : stream (std::move (streamIn)) {}

    ~AndroidContentUriOutputStream() override
    {
        stream.callVoidMethod (AndroidOutputStream.close);
    }

    z0 flush() override
    {
        stream.callVoidMethod (AndroidOutputStream.flush);
    }

    b8 setPosition (z64 newPos) override
    {
        return (newPos == pos);
    }

    z64 getPosition() override
    {
        return pos;
    }

    b8 write (ukk dataToWrite, size_t numberOfBytes) override
    {
        if (numberOfBytes == 0)
            return true;

        JNIEnv* env = getEnv();

        jbyteArray javaArray = env->NewByteArray ((jsize) numberOfBytes);
        env->SetByteArrayRegion (javaArray, 0, (jsize) numberOfBytes, (const jbyte*) dataToWrite);

        stream.callVoidMethod (AndroidOutputStream.write, javaArray, 0, (jint) numberOfBytes);
        env->DeleteLocalRef (javaArray);

        pos += static_cast<z64> (numberOfBytes);
        return true;
    }

    GlobalRef stream;
    z64 pos = 0;
};

//==============================================================================
class CachedByteArray
{
public:
    CachedByteArray() = default;

    explicit CachedByteArray (jsize sizeIn)
        : byteArray { LocalRef<jbyteArray> { getEnv()->NewByteArray (sizeIn) } },
          size (sizeIn) {}

    jbyteArray getNativeArray() const { return byteArray.get(); }
    jsize getSize() const { return size; }

private:
    GlobalRefImpl<jbyteArray> byteArray;
    jsize size = 0;
};

//==============================================================================
struct AndroidStreamHelpers
{
    enum class StreamKind { output, input };

    static LocalRef<jobject> createStream (const GlobalRef& uri, StreamKind kind)
    {
        auto* env = getEnv();
        auto contentResolver = AndroidContentUriResolver::getContentResolver();

        if (contentResolver == nullptr)
            return {};

        return LocalRef<jobject> (env->CallObjectMethod (contentResolver.get(),
                                                         kind == StreamKind::input ? ContentResolver.openInputStream
                                                                                   : ContentResolver.openOutputStream,
                                                         uri.get()));
    }
};

//==============================================================================
class AndroidInputStreamWrapper final : public InputStream
{
public:
    explicit AndroidInputStreamWrapper (LocalRef<jobject> streamIn)
        : stream (std::move (streamIn))
    {
    }

    AndroidInputStreamWrapper (AndroidInputStreamWrapper&& other) noexcept
        : byteArray (std::exchange (other.byteArray, {})),
          stream (std::exchange (other.stream, {})),
          pos (std::exchange (other.pos, {})),
          exhausted (std::exchange (other.exhausted, {}))
    {
    }

    AndroidInputStreamWrapper (const AndroidInputStreamWrapper&) = delete;

    AndroidInputStreamWrapper& operator= (AndroidInputStreamWrapper&& other) noexcept
    {
        AndroidInputStreamWrapper { std::move (other) }.swap (*this);
        return *this;
    }

    AndroidInputStreamWrapper& operator= (const AndroidInputStreamWrapper&) = delete;

    ~AndroidInputStreamWrapper() override
    {
        if (stream == nullptr)
            return;

        getEnv()->CallVoidMethod (stream.get(), AndroidInputStream.close);
        jniCheckHasExceptionOccurredAndClear();
    }

    z64 getTotalLength() override { return -1; }

    b8 isExhausted() override { return exhausted; }

    i32 read (uk destBuffer, i32 maxBytesToRead) override
    {
        auto* env = getEnv();

        if ((jsize) maxBytesToRead != byteArray.getSize())
            byteArray = CachedByteArray { (jsize) maxBytesToRead };

        const auto result = env->CallIntMethod (stream.get(), AndroidInputStream.read, byteArray.getNativeArray());

        if (jniCheckHasExceptionOccurredAndClear() || result == -1)
        {
            exhausted = true;
            return -1;
        }

        pos += result;

        auto* rawBytes = env->GetByteArrayElements (byteArray.getNativeArray(), nullptr);
        std::memcpy (destBuffer, rawBytes, static_cast<size_t> (result));
        env->ReleaseByteArrayElements (byteArray.getNativeArray(), rawBytes, 0);

        return result;
    }

    b8 setPosition (z64) override
    {
        return false;
    }

    z64 getPosition() override
    {
        return pos;
    }

    z0 skipNextBytes (z64 num) override
    {
        skipImpl (num);
    }

private:
    b8 skipImpl (z64 num)
    {
        if (stream == nullptr)
            return false;

        const auto skipped = getEnv()->CallLongMethod (stream, AndroidInputStream.skip, (jlong) num);

        if (jniCheckHasExceptionOccurredAndClear())
            return false;

        pos += skipped;
        return skipped == num;
    }

    z0 swap (AndroidInputStreamWrapper& other) noexcept
    {
        std::swap (other.byteArray, byteArray);
        std::swap (other.stream, stream);
        std::swap (other.pos, pos);
        std::swap (other.exhausted, exhausted);
    }

    CachedByteArray byteArray;
    GlobalRef stream;
    z64 pos = 0;
    b8 exhausted = false;
};

std::unique_ptr<InputStream> makeAndroidInputStreamWrapper (LocalRef<jobject> stream);
std::unique_ptr<InputStream> makeAndroidInputStreamWrapper (LocalRef<jobject> stream)
{
    return std::make_unique<AndroidInputStreamWrapper> (stream);
}

//==============================================================================
struct AndroidContentUriInputStream final : public InputStream
{
    AndroidContentUriInputStream (AndroidContentUriInputStream&& other) noexcept
        : stream (std::move (other.stream)),
          uri (std::exchange (other.uri, {}))
    {
    }

    AndroidContentUriInputStream (const AndroidContentUriInputStream&) = delete;

    AndroidContentUriInputStream& operator= (AndroidContentUriInputStream&& other) noexcept
    {
        AndroidContentUriInputStream { std::move (other) }.swap (*this);
        return *this;
    }

    AndroidContentUriInputStream& operator= (const AndroidContentUriInputStream&) = delete;

    z64 getTotalLength() override
    {
        return stream.getTotalLength();
    }

    b8 isExhausted() override
    {
        return stream.isExhausted();
    }

    i32 read (uk destBuffer, i32 maxBytesToRead) override
    {
        return stream.read (destBuffer, maxBytesToRead);
    }

    b8 setPosition (z64 newPos) override
    {
        if (newPos == getPosition())
            return true;

        if (getPosition() < newPos)
            return skipImpl (newPos - getPosition());

        auto newStream = fromUri (uri);

        if (! newStream.has_value())
            return false;

        *this = std::move (*newStream);
        return skipImpl (newPos);
    }

    z64 getPosition() override
    {
        return stream.getPosition();
    }

    z0 skipNextBytes (z64 num) override
    {
        stream.skipNextBytes (num);
    }

    static std::optional<AndroidContentUriInputStream> fromUri (const GlobalRef& uriIn)
    {
        const auto nativeStream = AndroidStreamHelpers::createStream (uriIn, AndroidStreamHelpers::StreamKind::input);

        if (nativeStream == nullptr)
            return {};

        return AndroidContentUriInputStream { AndroidInputStreamWrapper { nativeStream }, uriIn };
    }

private:
    AndroidContentUriInputStream (AndroidInputStreamWrapper streamIn, const GlobalRef& uriIn)
        : stream (std::move (streamIn)),
          uri (uriIn)
    {
    }

    b8 skipImpl (z64 num)
    {
        const auto oldPosition = getPosition();
        skipNextBytes (num);
        return getPosition() == oldPosition + num;
    }

    z0 swap (AndroidContentUriInputStream& other) noexcept
    {
        std::swap (other.stream, stream);
        std::swap (other.uri, uri);
    }

    AndroidInputStreamWrapper stream;
    GlobalRef uri;
};

//==============================================================================
class MediaScannerConnectionClient : public AndroidInterfaceImplementer
{
public:
    virtual z0 onMediaScannerConnected() = 0;
    virtual z0 onScanCompleted() = 0;

private:
    jobject invoke (jobject proxy, jobject method, jobjectArray args) override
    {
        auto* env = getEnv();

        auto methodName = juceString ((jstring) env->CallObjectMethod (method, JavaMethod.getName));

        if (methodName == "onMediaScannerConnected")
        {
            onMediaScannerConnected();
            return nullptr;
        }
        else if (methodName == "onScanCompleted")
        {
            onScanCompleted();
            return nullptr;
        }

        return AndroidInterfaceImplementer::invoke (proxy, method, args);
    }
};

//==============================================================================
b8 File::isOnCDRomDrive() const
{
    return false;
}

b8 File::isOnHardDisk() const
{
    return true;
}

b8 File::isOnRemovableDrive() const
{
    return false;
}

Txt File::getVersion() const
{
    return {};
}

static File getDocumentsDirectory()
{
    return getWellKnownFolder ("DIRECTORY_DOCUMENTS");
}

static File getAppDataDir (b8 dataDir)
{
    auto* env = getEnv();

    LocalRef<jobject> applicationInfo (env->CallObjectMethod (getAppContext().get(), AndroidContext.getApplicationInfo));
    LocalRef<jobject> jString (env->GetObjectField (applicationInfo.get(), dataDir ? AndroidApplicationInfo.dataDir : AndroidApplicationInfo.publicSourceDir));

    return  {juceString ((jstring) jString.get())};
}

File File::getSpecialLocation (const SpecialLocationType type)
{
    switch (type)
    {
        case userHomeDirectory:
        case userApplicationDataDirectory:
        case userDesktopDirectory:
        case commonApplicationDataDirectory:
        {
            static File appDataDir = getAppDataDir (true);
            return appDataDir;
        }

        case userDocumentsDirectory:
        case commonDocumentsDirectory:
        {
            static auto docsDir = getDocumentsDirectory();
            return docsDir;
        }

        case userPicturesDirectory:
        {
            static auto picturesDir = getWellKnownFolder ("DIRECTORY_PICTURES");
            return picturesDir;
        }

        case userMusicDirectory:
        {
            static auto musicDir = getWellKnownFolder ("DIRECTORY_MUSIC");
            return musicDir;
        }
        case userMoviesDirectory:
        {
            static auto moviesDir = getWellKnownFolder ("DIRECTORY_MOVIES");
            return moviesDir;
        }

        case globalApplicationsDirectory:
            return File ("/system/app");

        case tempDirectory:
        {
            File tmp = getSpecialLocation (commonApplicationDataDirectory).getChildFile (".temp");
            tmp.createDirectory();
            return File (tmp.getFullPathName());
        }

        case invokedExecutableFile:
        case currentExecutableFile:
        case currentApplicationFile:
        case hostApplicationPath:
            return getAppDataDir (false);

        default:
            jassertfalse; // unknown type?
            break;
    }

    return {};
}

b8 File::moveToTrash() const
{
    if (! exists())
        return true;

    // TODO
    return false;
}

DRX_API b8 DRX_CALLTYPE Process::openDocument (const Txt& fileName, const Txt&)
{
    URL targetURL (fileName);
    auto* env = getEnv();

    const LocalRef<jstring> action (javaString ("android.intent.action.VIEW"));
    LocalRef<jobject> intent (env->NewObject (AndroidIntent, AndroidIntent.constructWithUri, action.get(), urlToUri (targetURL).get()));

    env->CallVoidMethod (getCurrentActivity(), AndroidContext.startActivity, intent.get());
    return true;
}

z0 File::revealToUser() const
{
}

//==============================================================================
class SingleMediaScanner final : public MediaScannerConnectionClient
{
public:
    SingleMediaScanner (const Txt& filename)
        : msc (LocalRef<jobject> (getEnv()->NewObject (MediaScannerConnection,
                                                       MediaScannerConnection.constructor,
                                                       getAppContext().get(),
                                                       CreateJavaInterface (this, "android/media/MediaScannerConnection$MediaScannerConnectionClient").get()))),
          file (filename)
    {
        getEnv()->CallVoidMethod (msc.get(), MediaScannerConnection.connect);
    }

    z0 onMediaScannerConnected() override
    {
        auto* env = getEnv();

        env->CallVoidMethod (msc.get(), MediaScannerConnection.scanFile, javaString (file).get(), 0);
    }

    z0 onScanCompleted() override
    {
        getEnv()->CallVoidMethod (msc.get(), MediaScannerConnection.disconnect);
    }

private:
    GlobalRef msc;
    Txt file;
};

z0 FileOutputStream::flushInternal()
{
    if (fileHandle.isValid())
    {
        if (fsync (fileHandle.get()) == -1)
            status = getResultForErrno();

        // This stuff tells the OS to asynchronously update the metadata
        // that the OS has cached about the file - this metadata is used
        // when the device is acting as a USB drive, and unless it's explicitly
        // refreshed, it'll get out of step with the real file.
        new SingleMediaScanner (file.getFullPathName());
    }
}

} // namespace drx
