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

// This type isn't in the headers until v2.36
#if ! WEBKIT_CHECK_VERSION (2, 36, 0)
struct WebKitURISchemeResponse;
#endif

namespace drx
{

//==============================================================================
class WebKitSymbols final : public DeletedAtShutdown
{
public:
    //==============================================================================
    b8 isWebKitAvailable() const noexcept  { return webKitIsAvailable; }

    //==============================================================================
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_new, drx_webkit_settings_new,
                                         (), WebKitSettings*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_set_hardware_acceleration_policy, drx_webkit_settings_set_hardware_acceleration_policy,
                                         (WebKitSettings*, i32), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_set_user_agent, drx_webkit_settings_set_user_agent,
                                         (WebKitSettings*, const gchar*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_new_with_settings, drx_webkit_web_view_new_with_settings,
                                         (WebKitSettings*), GtkWidget*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_load_uri, drx_webkit_web_view_load_uri,
                                         (WebKitWebView*, const gchar*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_policy_decision_use, drx_webkit_policy_decision_use,
                                         (WebKitPolicyDecision*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_policy_decision_ignore, drx_webkit_policy_decision_ignore,
                                         (WebKitPolicyDecision*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_go_back, drx_webkit_web_view_go_back,
                                         (WebKitWebView*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_go_forward, drx_webkit_web_view_go_forward,
                                         (WebKitWebView*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_reload, drx_webkit_web_view_reload,
                                         (WebKitWebView*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_stop_loading, drx_webkit_web_view_stop_loading,
                                         (WebKitWebView*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_request_get_uri, drx_webkit_uri_request_get_uri,
                                         (WebKitURIRequest*), const gchar*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_navigation_action_get_request, drx_webkit_navigation_action_get_request,
                                         (WebKitNavigationAction*), WebKitURIRequest*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_navigation_policy_decision_get_frame_name, drx_webkit_navigation_policy_decision_get_frame_name,
                                         (WebKitNavigationPolicyDecision*), const gchar*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_navigation_policy_decision_get_navigation_action, drx_webkit_navigation_policy_decision_get_navigation_action,
                                         (WebKitNavigationPolicyDecision*), WebKitNavigationAction*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_get_uri, drx_webkit_web_view_get_uri,
                                         (WebKitWebView*), const gchar*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_run_javascript, drx_webkit_web_view_run_javascript,
                                         (WebKitWebView*, const gchar*, GCancellable*, GAsyncReadyCallback, gpointer), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_javascript_result_unref, drx_webkit_javascript_result_unref,
                                         (WebKitJavascriptResult*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_run_javascript_finish, drx_webkit_web_view_run_javascript_finish,
                                         (WebKitWebView*, GAsyncResult*, GError**), WebKitJavascriptResult*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_javascript_result_get_js_value, drx_webkit_javascript_result_get_js_value,
                                         (WebKitJavascriptResult*), JSCValue*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (jsc_value_to_string, drx_jsc_value_to_string,
                                         (JSCValue*), tuk)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_get_user_content_manager,
                                         drx_webkit_web_view_get_user_content_manager,
                                         (WebKitWebView*), WebKitUserContentManager*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_set_javascript_can_access_clipboard,
                                         drx_webkit_settings_set_javascript_can_access_clipboard,
                                         (WebKitSettings*, gboolean), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_set_enable_write_console_messages_to_stdout,
                                         drx_webkit_settings_set_enable_write_console_messages_to_stdout,
                                         (WebKitSettings*, gboolean), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_settings_set_enable_developer_extras,
                                         drx_webkit_settings_set_enable_developer_extras,
                                         (WebKitSettings*, gboolean), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_user_content_manager_register_script_message_handler,
                                         drx_webkit_user_content_manager_register_script_message_handler,
                                         (WebKitUserContentManager*, const gchar*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_user_script_new,
                                         drx_webkit_user_script_new,
                                         (const gchar*,
                                          WebKitUserContentInjectedFrames,
                                          WebKitUserScriptInjectionTime,
                                          const gchar* const*,
                                          const gchar* const*),
                                         WebKitUserScript*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_user_content_manager_add_script,
                                         drx_webkit_user_content_manager_add_script,
                                         (WebKitUserContentManager*, WebKitUserScript*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_context_register_uri_scheme,
                                         drx_webkit_web_context_register_uri_scheme,
                                         (WebKitWebContext*,
                                          const gchar*,
                                          WebKitURISchemeRequestCallback,
                                          gpointer,
                                          GDestroyNotify),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_web_view_get_context,
                                         drx_webkit_web_view_get_context,
                                         (WebKitWebView*),
                                         WebKitWebContext*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_scheme_request_get_path,
                                         drx_webkit_uri_scheme_request_get_path,
                                         (WebKitURISchemeRequest*),
                                         const gchar*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_scheme_response_new,
                                         drx_webkit_uri_scheme_response_new,
                                         (GInputStream*, gint64),
                                         WebKitURISchemeResponse*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_scheme_response_set_http_headers,
                                         drx_webkit_uri_scheme_response_set_http_headers,
                                         (WebKitURISchemeResponse*, SoupMessageHeaders*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_scheme_response_set_status,
                                         drx_webkit_uri_scheme_response_set_status,
                                         (WebKitURISchemeResponse*, guint, const gchar*),
                                         z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (webkit_uri_scheme_request_finish_with_response,
                                         drx_webkit_uri_scheme_request_finish_with_response,
                                         (WebKitURISchemeRequest*, WebKitURISchemeResponse*),
                                         z0)

    //==============================================================================
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (gtk_init, drx_gtk_init,
                                         (i32*, tuk**), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (gtk_plug_new, drx_gtk_plug_new,
                                         (::Window), GtkWidget*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (gtk_scrolled_window_new, drx_gtk_scrolled_window_new,
                                         (GtkAdjustment*, GtkAdjustment*), GtkWidget*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (gtk_container_add, drx_gtk_container_add,
                                         (GtkContainer*, GtkWidget*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (gtk_widget_show_all, drx_gtk_widget_show_all,
                                         (GtkWidget*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (gtk_plug_get_id, drx_gtk_plug_get_id,
                                         (GtkPlug*), ::Window)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (gtk_main, drx_gtk_main,
                                         (), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (gtk_main_quit, drx_gtk_main_quit,
                                         (), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (g_unix_fd_add, drx_g_unix_fd_add,
                                         (gint, GIOCondition, GUnixFDSourceFunc, gpointer), guint)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (g_object_ref, drx_g_object_ref,
                                         (gpointer), gpointer)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (g_object_unref, drx_g_object_unref,
                                         (gpointer), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (g_memory_input_stream_new, drx_g_memory_input_stream_new,
                                         (), GInputStream*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (g_memory_input_stream_new_from_bytes, drx_g_memory_input_stream_new_from_bytes,
                                         (GBytes*), GInputStream*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (g_bytes_new, drx_g_bytes_new,
                                         (gconstpointer, gsize), GBytes*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (g_bytes_unref, drx_g_bytes_unref,
                                         (GBytes*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (g_error_free, drx_g_error_free,
                                         (GError*), z0)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (g_signal_connect_data, drx_g_signal_connect_data,
                                         (gpointer, const gchar*, GCallback, gpointer, GClosureNotify, GConnectFlags), gulong)

    //==============================================================================
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (gdk_set_allowed_backends, drx_gdk_set_allowed_backends,
                                         (tukk), z0)

    //==============================================================================
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (jsc_value_to_json, drx_jsc_value_to_json,
                                         (JSCValue*, guint), tuk)

    //==============================================================================
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (soup_message_headers_new, drx_soup_message_headers_new,
                                         (SoupMessageHeadersType), SoupMessageHeaders*)

    DRX_GENERATE_FUNCTION_WITH_DEFAULT (soup_message_headers_append, drx_soup_message_headers_append,
                                         (SoupMessageHeaders*, tukk, tukk), z0)

    //==============================================================================
    DRX_GENERATE_FUNCTION_WITH_DEFAULT (g_free, drx_g_free,
                                         (gpointer), z0)

    //==============================================================================
    DRX_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE (WebKitSymbols)

private:
    WebKitSymbols() = default;

    ~WebKitSymbols()
    {
        clearSingletonInstance();
    }

    template <typename FuncPtr>
    struct SymbolBinding
    {
        FuncPtr& func;
        tukk name;
    };

    template <typename FuncPtr>
    SymbolBinding<FuncPtr> makeSymbolBinding (FuncPtr& func, tukk name)
    {
        return { func, name };
    }

    template <typename FuncPtr>
    b8 loadSymbols (DynamicLibrary& lib, SymbolBinding<FuncPtr> binding)
    {
        if (auto* func = lib.getFunction (binding.name))
        {
            binding.func = reinterpret_cast<FuncPtr> (func);
            return true;
        }

        return false;
    }

    template <typename FuncPtr, typename... Args>
    b8 loadSymbols (DynamicLibrary& lib, SymbolBinding<FuncPtr> binding, Args... args)
    {
        return loadSymbols (lib, binding) && loadSymbols (lib, args...);
    }

    //==============================================================================
    b8 loadWebkitSymbols()
    {
        return loadSymbols (webkitLib,
                            makeSymbolBinding (drx_webkit_settings_new,                                         "webkit_settings_new"),
                            makeSymbolBinding (drx_webkit_settings_set_hardware_acceleration_policy,            "webkit_settings_set_hardware_acceleration_policy"),
                            makeSymbolBinding (drx_webkit_settings_set_user_agent,                              "webkit_settings_set_user_agent"),
                            makeSymbolBinding (drx_webkit_web_view_new_with_settings,                           "webkit_web_view_new_with_settings"),
                            makeSymbolBinding (drx_webkit_policy_decision_use,                                  "webkit_policy_decision_use"),
                            makeSymbolBinding (drx_webkit_policy_decision_ignore,                               "webkit_policy_decision_ignore"),
                            makeSymbolBinding (drx_webkit_web_view_go_back,                                     "webkit_web_view_go_back"),
                            makeSymbolBinding (drx_webkit_web_view_go_forward,                                  "webkit_web_view_go_forward"),
                            makeSymbolBinding (drx_webkit_web_view_reload,                                      "webkit_web_view_reload"),
                            makeSymbolBinding (drx_webkit_web_view_stop_loading,                                "webkit_web_view_stop_loading"),
                            makeSymbolBinding (drx_webkit_uri_request_get_uri,                                  "webkit_uri_request_get_uri"),
                            makeSymbolBinding (drx_webkit_web_view_load_uri,                                    "webkit_web_view_load_uri"),
                            makeSymbolBinding (drx_webkit_navigation_action_get_request,                        "webkit_navigation_action_get_request"),
                            makeSymbolBinding (drx_webkit_navigation_policy_decision_get_frame_name,            "webkit_navigation_policy_decision_get_frame_name"),
                            makeSymbolBinding (drx_webkit_navigation_policy_decision_get_navigation_action,     "webkit_navigation_policy_decision_get_navigation_action"),
                            makeSymbolBinding (drx_webkit_web_view_get_uri,                                     "webkit_web_view_get_uri"),
                            makeSymbolBinding (drx_webkit_web_view_run_javascript,                              "webkit_web_view_run_javascript"),
                            makeSymbolBinding (drx_webkit_javascript_result_unref,                              "webkit_javascript_result_unref"),
                            makeSymbolBinding (drx_webkit_web_view_get_user_content_manager,                    "webkit_web_view_get_user_content_manager"),
                            makeSymbolBinding (drx_webkit_settings_set_javascript_can_access_clipboard,         "webkit_settings_set_javascript_can_access_clipboard"),
                            makeSymbolBinding (drx_webkit_settings_set_enable_write_console_messages_to_stdout, "webkit_settings_set_enable_write_console_messages_to_stdout"),
                            makeSymbolBinding (drx_webkit_settings_set_enable_developer_extras,                 "webkit_settings_set_enable_developer_extras"),
                            makeSymbolBinding (drx_webkit_user_content_manager_register_script_message_handler, "webkit_user_content_manager_register_script_message_handler"),
                            makeSymbolBinding (drx_webkit_user_script_new,                                      "webkit_user_script_new"),
                            makeSymbolBinding (drx_webkit_user_content_manager_add_script,                      "webkit_user_content_manager_add_script"),
                            makeSymbolBinding (drx_webkit_javascript_result_get_js_value,                       "webkit_javascript_result_get_js_value"),
                            makeSymbolBinding (drx_jsc_value_to_string,                                         "jsc_value_to_string"),
                            makeSymbolBinding (drx_webkit_web_view_run_javascript_finish,                       "webkit_web_view_run_javascript_finish"),
                            makeSymbolBinding (drx_webkit_web_context_register_uri_scheme,                      "webkit_web_context_register_uri_scheme"),
                            makeSymbolBinding (drx_webkit_web_view_get_context,                                 "webkit_web_view_get_context"),
                            makeSymbolBinding (drx_webkit_uri_scheme_request_get_path,                          "webkit_uri_scheme_request_get_path"),
                            makeSymbolBinding (drx_webkit_uri_scheme_response_new,                              "webkit_uri_scheme_response_new"),
                            makeSymbolBinding (drx_webkit_uri_scheme_response_set_http_headers,                 "webkit_uri_scheme_response_set_http_headers"),
                            makeSymbolBinding (drx_webkit_uri_scheme_response_set_status,                       "webkit_uri_scheme_response_set_status"),
                            makeSymbolBinding (drx_webkit_uri_scheme_request_finish_with_response,              "webkit_uri_scheme_request_finish_with_response"));
    }

    b8 loadGtkSymbols()
    {
        return loadSymbols (gtkLib,
                            makeSymbolBinding (drx_gtk_init,                             "gtk_init"),
                            makeSymbolBinding (drx_gtk_plug_new,                         "gtk_plug_new"),
                            makeSymbolBinding (drx_gtk_scrolled_window_new,              "gtk_scrolled_window_new"),
                            makeSymbolBinding (drx_gtk_container_add,                    "gtk_container_add"),
                            makeSymbolBinding (drx_gtk_widget_show_all,                  "gtk_widget_show_all"),
                            makeSymbolBinding (drx_gtk_plug_get_id,                      "gtk_plug_get_id"),
                            makeSymbolBinding (drx_gtk_main,                             "gtk_main"),
                            makeSymbolBinding (drx_gtk_main_quit,                        "gtk_main_quit"),
                            makeSymbolBinding (drx_g_unix_fd_add,                        "g_unix_fd_add"),
                            makeSymbolBinding (drx_g_object_ref,                         "g_object_ref"),
                            makeSymbolBinding (drx_g_object_unref,                       "g_object_unref"),
                            makeSymbolBinding (drx_g_bytes_new,                          "g_bytes_new"),
                            makeSymbolBinding (drx_g_bytes_unref,                        "g_bytes_unref"),
                            makeSymbolBinding (drx_g_signal_connect_data,                "g_signal_connect_data"),
                            makeSymbolBinding (drx_gdk_set_allowed_backends,             "gdk_set_allowed_backends"),
                            makeSymbolBinding (drx_g_memory_input_stream_new,            "g_memory_input_stream_new"),
                            makeSymbolBinding (drx_g_memory_input_stream_new_from_bytes, "g_memory_input_stream_new_from_bytes"));
    }

    b8 loadJsLibSymbols()
    {
        return loadSymbols (jsLib,
                            makeSymbolBinding (drx_jsc_value_to_json, "jsc_value_to_json"));
    }

    b8 loadSoupLibSymbols()
    {
        return loadSymbols (soupLib,
                            makeSymbolBinding (drx_soup_message_headers_new, "soup_message_headers_new"),
                            makeSymbolBinding (drx_soup_message_headers_append, "soup_message_headers_append"));
    }

    b8 loadGlibSymbols()
    {
        return loadSymbols (glib,
                            makeSymbolBinding (drx_g_free, "g_free"));
    }

    struct WebKitAndDependencyLibraryNames
    {
        tukk webkitLib;
        tukk jsLib;
        tukk soupLib;
    };

    b8 openWebKitAndDependencyLibraries (const WebKitAndDependencyLibraryNames& names)
    {
        if (webkitLib.open (names.webkitLib) && jsLib.open (names.jsLib) && soupLib.open (names.soupLib))
            return true;

        for (auto* l : { &webkitLib, &jsLib, &soupLib })
            l->close();

        return false;
    }

    //==============================================================================
    DynamicLibrary webkitLib, jsLib, soupLib;

    DynamicLibrary gtkLib    { "libgtk-3.so" },
                   glib      { "libglib-2.0.so" };

    const b8 webKitIsAvailable =    (   openWebKitAndDependencyLibraries ({ "libwebkit2gtk-4.1.so",
                                                                              "libjavascriptcoregtk-4.1.so",
                                                                              "libsoup-3.0.so" })
                                       || openWebKitAndDependencyLibraries ({ "libwebkit2gtk-4.0.so",
                                                                              "libjavascriptcoregtk-4.0.so",
                                                                              "libsoup-2.4.so" }))
                                   && loadWebkitSymbols()
                                   && loadGtkSymbols()
                                   && loadJsLibSymbols()
                                   && loadSoupLibSymbols()
                                   && loadGlibSymbols();

    DRX_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebKitSymbols)
};

//==============================================================================
extern "C" i32 drx_gtkWebkitMain (i32 argc, tukk const* argv);

class CommandReceiver
{
public:
    struct Responder
    {
        virtual ~Responder() {}

        virtual z0 handleCommand (const Txt& cmd, const var& param) = 0;
        virtual z0 receiverHadError() = 0;
    };

    enum class ReturnAfterMessageReceived
    {
        no,
        yes
    };

    CommandReceiver (Responder* responderToUse, i32 inputChannelToUse)
        : responder (responderToUse), inChannel (inputChannelToUse)
    {
        setBlocking (inChannel, false);
    }

    static z0 setBlocking (i32 fd, b8 shouldBlock)
    {
        auto flags = fcntl (fd, F_GETFL);
        fcntl (fd, F_SETFL, (shouldBlock ? (flags & ~O_NONBLOCK)
                                         : (flags | O_NONBLOCK)));
    }

    i32 getFd() const     { return inChannel; }

    z0 tryNextRead (ReturnAfterMessageReceived ret = ReturnAfterMessageReceived::no)
    {
        for (;;)
        {
            auto len = (receivingLength ? sizeof (size_t) : bufferLength.len);

            if (! receivingLength)
                buffer.realloc (len);

            auto* dst = (receivingLength ? bufferLength.data : buffer.getData());

            auto actual = read (inChannel, &dst[pos], static_cast<size_t> (len - pos));

            if (actual < 0)
            {
                if (errno == EINTR)
                    continue;

                break;
            }

            pos += static_cast<size_t> (actual);

            if (pos == len)
            {
                pos = 0;

                if (! std::exchange (receivingLength, ! receivingLength))
                {
                    parseJSON (Txt (buffer.getData(), bufferLength.len));

                    if (ret == ReturnAfterMessageReceived::yes)
                        return;
                }
            }
        }

        if (errno != EAGAIN && errno != EWOULDBLOCK && responder != nullptr)
            responder->receiverHadError();
    }

    static z0 sendCommand (i32 outChannel, const Txt& cmd, const var& params)
    {
        DynamicObject::Ptr obj = new DynamicObject;

        obj->setProperty (getCmdIdentifier(), cmd);

        if (! params.isVoid())
            obj->setProperty (getParamIdentifier(), params);

        auto json = JSON::toString (var (obj.get()));

        auto jsonLength = static_cast<size_t> (json.length());
        auto len        = sizeof (size_t) + jsonLength;

        HeapBlock<t8> buffer (len);
        auto* dst = buffer.getData();

        memcpy (dst, &jsonLength, sizeof (size_t));
        dst += sizeof (size_t);

        memcpy (dst, json.toRawUTF8(), jsonLength);

        ssize_t ret;

        for (;;)
        {
            ret = write (outChannel, buffer.getData(), len);

            if (ret != -1 || errno != EINTR)
                break;
        }
    }

private:
    z0 parseJSON (const Txt& json)
    {
        auto object = JSON::fromString (json);

        if (! object.isVoid())
        {
            auto cmd    = object.getProperty (getCmdIdentifier(),   {}).toString();
            auto params = object.getProperty (getParamIdentifier(), {});

            if (responder != nullptr)
                responder->handleCommand (cmd, params);
        }
    }

    static Identifier getCmdIdentifier()    { static Identifier Id ("cmd");    return Id; }
    static Identifier getParamIdentifier()  { static Identifier Id ("params"); return Id; }

    Responder* responder = nullptr;
    i32 inChannel = 0;
    size_t pos = 0;
    b8 receivingLength = true;
    union { t8 data [sizeof (size_t)]; size_t len; } bufferLength;
    HeapBlock<t8> buffer;
};

#define drx_g_signal_connect(instance, detailed_signal, c_handler, data) \
    WebKitSymbols::getInstance()->drx_g_signal_connect_data (instance, detailed_signal, c_handler, data, nullptr, (GConnectFlags) 0)

static constexpr tukk platformSpecificIntegrationScript = R"(
window.__DRX__ = {
  postMessage: function (object) {
    window.webkit.messageHandlers.__DRX__.postMessage(object);
  },
};
)";

struct InitialisationData
{
    b8 nativeIntegrationsEnabled;
    Txt userAgent;
    Txt userScript;
    Txt allowedOrigin;

    static constexpr std::optional<i32> marshallingVersion = std::nullopt;

    template <typename Archive, typename Item>
    static z0 serialise (Archive& archive, Item& item)
    {
        archive (named ("nativeIntegrationsEnabled", item.nativeIntegrationsEnabled),
                 named ("userAgent", item.userAgent),
                 named ("userScript", item.userScript),
                 named ("allowedOrigin", item.allowedOrigin));
    }
};

struct EvaluateJavascriptParams
{
    Txt script;
    b8 requireCallback;

    static constexpr std::optional<i32> marshallingVersion = std::nullopt;

    template <typename Archive, typename Item>
    static z0 serialise (Archive& archive, Item& item)
    {
        archive (named ("script", item.script),
                 named ("requireCallback", item.requireCallback));
    }
};

struct EvaluateJavascriptCallbackParams
{
    b8 success;

    // This is necessary because a DynamicObject with a property value of var::undefined()
    // cannot be unserialised. So we need to signal this case with an extra variable.
    b8 hasPayload;

    var payload;
    Txt error;

    static constexpr std::optional<i32> marshallingVersion = std::nullopt;

    template <typename Archive, typename Item>
    static z0 serialise (Archive& archive, Item& item)
    {
        archive(named ("success", item.success),
                named ("hasPayload", item.hasPayload),
                named ("payload", item.payload),
                named ("error", item.error));
    }

    static inline Txt key { "evaluateJavascriptCallbackParams" };
};

struct ResourceRequest
{
    z64 requestId;
    Txt path;

    static constexpr std::optional<i32> marshallingVersion = std::nullopt;

    template <typename Archive, typename Item>
    static z0 serialise (Archive& archive, Item& item)
    {
        archive (named ("requestId", item.requestId),
                 named ("path", item.path));
    }

    static inline const Txt key { "resourceRequest" };
};

template <>
struct SerialisationTraits<WebBrowserComponent::Resource>
{
    static constexpr auto marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& item)
    {
        archive (named ("data", item.data),
                 named ("mimeType", item.mimeType));
    }
};

struct ResourceRequestResponse
{
    z64 requestId;
    std::optional<WebBrowserComponent::Resource> resource;

    static constexpr std::optional<i32> marshallingVersion = std::nullopt;

    template <typename Archive, typename T>
    static z0 serialise (Archive& archive, T& item)
    {
        archive (named ("requestId", item.requestId),
                 named ("resource", item.resource));
    }

    static inline const Txt key { "resourceRequestResponse" };
};

//==============================================================================
class GtkChildProcess final : private CommandReceiver::Responder
{
public:
    //==============================================================================
    GtkChildProcess (i32 inChannel, i32 outChannelToUse, const Txt& userAgentToUse)
        : outChannel (outChannelToUse),
          receiver (this, inChannel),
          userAgent (userAgentToUse)
    {}

    i32 entry()
    {
        CommandReceiver::setBlocking (outChannel, true);

        {
            const ScopeGuard scope { [this] { CommandReceiver::setBlocking (receiver.getFd(), false); } };
            CommandReceiver::setBlocking (receiver.getFd(), true);
            receiver.tryNextRead (CommandReceiver::ReturnAfterMessageReceived::yes);

            if (! initialisationData.has_value())
            {
                std::cerr << "The first message received by GtkChildProcess should have been "
                             "the initialisationData, but it wasn't." << std::endl;

                return 1;
            }
        }

        auto& wk = *WebKitSymbols::getInstance();

        // webkit2gtk crashes when using the wayland backend embedded into an x11 window
        wk.drx_gdk_set_allowed_backends ("x11");

        wk.drx_gtk_init (nullptr, nullptr);

        auto* settings = wk.drx_webkit_settings_new();

        static constexpr i32 webkitHadwareAccelerationPolicyNeverFlag = 2;

        WebKitSymbols::getInstance()->drx_webkit_settings_set_hardware_acceleration_policy (settings,
                                                                                             webkitHadwareAccelerationPolicyNeverFlag);
        if (initialisationData->userAgent.isNotEmpty())
            WebKitSymbols::getInstance()->drx_webkit_settings_set_user_agent (settings,
                                                                               initialisationData->userAgent.toRawUTF8());

        auto* plug      = WebKitSymbols::getInstance()->drx_gtk_plug_new (0);
        auto* container = WebKitSymbols::getInstance()->drx_gtk_scrolled_window_new (nullptr, nullptr);

       #if DRX_DEBUG
        wk.drx_webkit_settings_set_enable_write_console_messages_to_stdout (settings, true);
        wk.drx_webkit_settings_set_enable_developer_extras (settings, true);
       #endif

        auto* webviewWidget = WebKitSymbols::getInstance()->drx_webkit_web_view_new_with_settings (settings);
        webview = (WebKitWebView*) webviewWidget;

        if (initialisationData->nativeIntegrationsEnabled)
        {
            manager = wk.drx_webkit_web_view_get_user_content_manager (webview);

            // It's probably fine to not disconnect these signals, given that upon closing the
            // WebBrowserComponent the entire subprocess is cleaned up with the manager and
            // everything.
            drx_g_signal_connect (manager,
                                   "script-message-received::__DRX__",
                                   G_CALLBACK (+[] (WebKitUserContentManager*, WebKitJavascriptResult* r, gpointer arg)
                                   {
                                       static_cast<GtkChildProcess*> (arg)->invokeCallback (r);
                                   }),
                                   this);

            wk.drx_webkit_user_content_manager_register_script_message_handler (manager, "__DRX__");

            auto* context = wk.drx_webkit_web_view_get_context (webview);
            wk.drx_webkit_web_context_register_uri_scheme (context, "drx", resourceRequestedCallback, this, nullptr);

            const StringArray userScripts { platformSpecificIntegrationScript,
                                            initialisationData->userScript };

            wk.drx_webkit_user_content_manager_add_script (manager, wk.drx_webkit_user_script_new (userScripts.joinIntoString ("\n").toRawUTF8(),
                                                                                                           WEBKIT_USER_CONTENT_INJECT_TOP_FRAME,
                                                                                                           WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
                                                                                                           nullptr, nullptr));
        }

        WebKitSymbols::getInstance()->drx_gtk_container_add ((GtkContainer*) container, webviewWidget);
        WebKitSymbols::getInstance()->drx_gtk_container_add ((GtkContainer*) plug,      container);

        WebKitSymbols::getInstance()->drx_webkit_web_view_load_uri (webview, "about:blank");

        drx_g_signal_connect (webview, "decide-policy",
                               (GCallback) decidePolicyCallback, this);

        drx_g_signal_connect (webview, "load-changed",
                               (GCallback) loadChangedCallback, this);

        drx_g_signal_connect (webview, "load-failed",
                               (GCallback) loadFailedCallback, this);

        WebKitSymbols::getInstance()->drx_gtk_widget_show_all (plug);
        auto wID = (u64) WebKitSymbols::getInstance()->drx_gtk_plug_get_id ((GtkPlug*) plug);

        ssize_t ret;

        for (;;)
        {
            ret = write (outChannel, &wID, sizeof (wID));

            if (ret != -1 || errno != EINTR)
                break;
        }

        WebKitSymbols::getInstance()->drx_g_unix_fd_add (receiver.getFd(), G_IO_IN, pipeReadyStatic, this);
        receiver.tryNextRead();

        WebKitSymbols::getInstance()->drx_gtk_main();

        WebKitSymbols::getInstance()->deleteInstance();
        return 0;
    }

    z0 invokeCallback (WebKitJavascriptResult* r)
    {
        auto& wk = *WebKitSymbols::getInstance();

        auto s = wk.drx_jsc_value_to_string (wk.drx_webkit_javascript_result_get_js_value (r));
        CommandReceiver::sendCommand (outChannel, "invokeCallback", var (s));
        wk.drx_g_free (s);
    }

    z0 goToURL (const var& params)
    {
        static Identifier urlIdentifier ("url");
        auto url = params.getProperty (urlIdentifier, var()).toString();

        WebKitSymbols::getInstance()->drx_webkit_web_view_load_uri (webview, url.toRawUTF8());
    }

    z0 handleDecisionResponse (const var& params)
    {
        auto* decision = (WebKitPolicyDecision*) ((z64) params.getProperty ("decision_id", var (0)));
        b8 allow = params.getProperty ("allow", var (false));

        if (decision != nullptr && decisions.contains (decision))
        {
            if (allow)
                WebKitSymbols::getInstance()->drx_webkit_policy_decision_use (decision);
            else
                WebKitSymbols::getInstance()->drx_webkit_policy_decision_ignore (decision);

            decisions.removeAllInstancesOf (decision);
            WebKitSymbols::getInstance()->drx_g_object_unref (decision);
        }
    }

    z0 evaluateJavascript (const var& params)
    {
        const auto jsParams = FromVar::convert<EvaluateJavascriptParams> (params);

        if (! jsParams.has_value())
        {
            std::cerr << "Wrong params received by evaluateJavascript()" << std::endl;
            return;
        }

        WebKitSymbols::getInstance()->drx_webkit_web_view_run_javascript (webview,
                                                                           jsParams->script.toRawUTF8(),
                                                                           nullptr,
                                                                           javascriptFinishedCallback,
                                                                           this);
    }

    z0 handleResourceRequesteResponse (const var& params)
    {
        auto& wk = *WebKitSymbols::getInstance();

        const auto response = FromVar::convert<ResourceRequestResponse> (params);

        if (! response.has_value())
        {
            std::cerr << "Bad request response received" << std::endl;
            return;
        }

        auto* request = requestIds.remove (response->requestId);

        // The WebKitURISchemeResponse object will take ownership of the headers
        auto* headers = wk.drx_soup_message_headers_new (SoupMessageHeadersType::SOUP_MESSAGE_HEADERS_RESPONSE);

        if (initialisationData->allowedOrigin.isNotEmpty())
            wk.drx_soup_message_headers_append (headers, "Access-Control-Allow-Origin", initialisationData->allowedOrigin.toRawUTF8());

        if (response->resource.has_value())
        {
            auto* streamBytes = wk.drx_g_bytes_new (response->resource->data.data(),
                                                        static_cast<gsize> (response->resource->data.size()));
            ScopeGuard bytesScope { [&] { wk.drx_g_bytes_unref (streamBytes); } };

            auto* stream = wk.drx_g_memory_input_stream_new_from_bytes (streamBytes);
            ScopeGuard streamScope { [&] { wk.drx_g_object_unref (stream); } };

            auto* webkitResponse = wk.drx_webkit_uri_scheme_response_new (stream,
                                                                              static_cast<gint64> (response->resource->data.size()));
            ScopeGuard webkitResponseScope { [&] { wk.drx_g_object_unref (webkitResponse); } };

            wk.drx_soup_message_headers_append (headers, "Content-Type", response->resource->mimeType.toRawUTF8());

            wk.drx_webkit_uri_scheme_response_set_http_headers (webkitResponse, headers);
            wk.drx_webkit_uri_scheme_response_set_status (webkitResponse, 200, nullptr);
            wk.drx_webkit_uri_scheme_request_finish_with_response (request, webkitResponse);

            return;
        }

        auto* stream = wk.drx_g_memory_input_stream_new();
        ScopeGuard streamScope { [&] { wk.drx_g_object_unref (stream); } };

        auto* webkitResponse = wk.drx_webkit_uri_scheme_response_new (stream, 0);
        ScopeGuard webkitResponseScope { [&] { wk.drx_g_object_unref (webkitResponse); } };

        wk.drx_webkit_uri_scheme_response_set_http_headers (webkitResponse, headers);
        wk.drx_webkit_uri_scheme_response_set_status (webkitResponse, 404, nullptr);
        wk.drx_webkit_uri_scheme_request_finish_with_response (request, webkitResponse);
    }

    //==============================================================================
    z0 handleCommand (const Txt& cmd, const var& params) override
    {
        auto& wk = *WebKitSymbols::getInstance();

        if      (cmd == "quit")                       quit();
        else if (cmd == "goToURL")                    goToURL (params);
        else if (cmd == "goBack")                     wk.drx_webkit_web_view_go_back      (webview);
        else if (cmd == "goForward")                  wk.drx_webkit_web_view_go_forward   (webview);
        else if (cmd == "refresh")                    wk.drx_webkit_web_view_reload       (webview);
        else if (cmd == "stop")                       wk.drx_webkit_web_view_stop_loading (webview);
        else if (cmd == "decision")                   handleDecisionResponse (params);
        else if (cmd == "init")                       initialisationData = FromVar::convert<InitialisationData> (params);
        else if (cmd == "evaluateJavascript")         evaluateJavascript (params);
        else if (cmd == ResourceRequestResponse::key) handleResourceRequesteResponse (params);
    }

    z0 receiverHadError() override
    {
        exit (-1);
    }

    //==============================================================================
    b8 pipeReady (gint fd, GIOCondition)
    {
        if (fd == receiver.getFd())
        {
            receiver.tryNextRead();
            return true;
        }

        return false;
    }

    z0 quit()
    {
        WebKitSymbols::getInstance()->drx_gtk_main_quit();
    }

    Txt getURIStringForAction (WebKitNavigationAction* action)
    {
        auto* request = WebKitSymbols::getInstance()->drx_webkit_navigation_action_get_request (action);
        return WebKitSymbols::getInstance()->drx_webkit_uri_request_get_uri (request);
    }

    b8 onNavigation (Txt frameName,
                       WebKitNavigationAction* action,
                       WebKitPolicyDecision* decision)
    {
        if (decision != nullptr && frameName.isEmpty())
        {
            WebKitSymbols::getInstance()->drx_g_object_ref (decision);
            decisions.add (decision);

            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", getURIStringForAction (action));
            params->setProperty ("decision_id", (z64) decision);
            CommandReceiver::sendCommand (outChannel, "pageAboutToLoad", var (params.get()));

            return true;
        }

        return false;
    }

    b8 onNewWindow (Txt /*frameName*/,
                      WebKitNavigationAction* action,
                      WebKitPolicyDecision* decision)
    {
        if (decision != nullptr)
        {
            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", getURIStringForAction (action));
            CommandReceiver::sendCommand (outChannel, "newWindowAttemptingToLoad", var (params.get()));

            // never allow new windows
            WebKitSymbols::getInstance()->drx_webkit_policy_decision_ignore (decision);

            return true;
        }

        return false;
    }

    z0 onLoadChanged (WebKitLoadEvent loadEvent)
    {
        if (loadEvent == WEBKIT_LOAD_FINISHED)
        {
            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("url", Txt (WebKitSymbols::getInstance()->drx_webkit_web_view_get_uri (webview)));
            CommandReceiver::sendCommand (outChannel, "pageFinishedLoading", var (params.get()));
        }
    }

    b8 onDecidePolicy (WebKitPolicyDecision*    decision,
                         WebKitPolicyDecisionType decisionType)
    {
        switch (decisionType)
        {
        case WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION:
            {
                auto* navigationDecision = (WebKitNavigationPolicyDecision*) decision;
                auto* frameName = WebKitSymbols::getInstance()->drx_webkit_navigation_policy_decision_get_frame_name (navigationDecision);

                return onNavigation (Txt (frameName != nullptr ? frameName : ""),
                                     WebKitSymbols::getInstance()->drx_webkit_navigation_policy_decision_get_navigation_action (navigationDecision),
                                     decision);
            }
            break;
        case WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION:
            {
                auto* navigationDecision = (WebKitNavigationPolicyDecision*) decision;
                auto* frameName = WebKitSymbols::getInstance()->drx_webkit_navigation_policy_decision_get_frame_name (navigationDecision);

                return onNewWindow  (Txt (frameName != nullptr ? frameName : ""),
                                     WebKitSymbols::getInstance()->drx_webkit_navigation_policy_decision_get_navigation_action (navigationDecision),
                                     decision);
            }
            break;
        case WEBKIT_POLICY_DECISION_TYPE_RESPONSE:
            {
                [[maybe_unused]] auto* response = (WebKitNavigationPolicyDecision*) decision;

                // for now just always allow response requests
                WebKitSymbols::getInstance()->drx_webkit_policy_decision_use (decision);
                return true;
            }
            break;
        default:
            break;
        }

        return false;
    }

    z0 onLoadFailed (GError* error)
    {
        DynamicObject::Ptr params = new DynamicObject;

        params->setProperty ("error", Txt (error != nullptr ? error->message : "unknown error"));
        CommandReceiver::sendCommand (outChannel, "pageLoadHadNetworkError", var (params.get()));
    }

private:
    z0 handleEvaluationCallback (const std::optional<var>& value, const Txt& error)
    {
        const auto success = value.has_value();
        const auto hasPayload = success && ! value->isUndefined();

        CommandReceiver::sendCommand (outChannel,
                                      EvaluateJavascriptCallbackParams::key,
                                      *ToVar::convert (EvaluateJavascriptCallbackParams { success,
                                                                                          hasPayload,
                                                                                          hasPayload ? *value : var{},
                                                                                          error }));
    }

    z0 handleResourceRequestedCallback (WebKitURISchemeRequest* request, const Txt& path)
    {
        const auto requestId = requestIds.insert (request);
        CommandReceiver::sendCommand (outChannel,
                                      ResourceRequest::key,
                                      *ToVar::convert (ResourceRequest { requestId, path }));
    }

    static gboolean pipeReadyStatic (gint fd, GIOCondition condition, gpointer user)
    {
        return (reinterpret_cast<GtkChildProcess*> (user)->pipeReady (fd, condition) ? TRUE : FALSE);
    }

    static gboolean decidePolicyCallback (WebKitWebView*,
                                          WebKitPolicyDecision*    decision,
                                          WebKitPolicyDecisionType decisionType,
                                          gpointer user)
    {
        auto& owner = *reinterpret_cast<GtkChildProcess*> (user);
        return (owner.onDecidePolicy (decision, decisionType) ? TRUE : FALSE);
    }

    static z0 loadChangedCallback (WebKitWebView*,
                                     WebKitLoadEvent loadEvent,
                                     gpointer        user)
    {
        auto& owner = *reinterpret_cast<GtkChildProcess*> (user);
        owner.onLoadChanged (loadEvent);
    }

    static z0 loadFailedCallback (WebKitWebView*,
                                    WebKitLoadEvent /*loadEvent*/,
                                    gchar*          /*failing_uri*/,
                                    GError*         error,
                                    gpointer        user)
    {
        auto& owner = *reinterpret_cast<GtkChildProcess*> (user);
        owner.onLoadFailed (error);
    }

    static var fromJSCValue (JSCValue* value)
    {
        auto* json = WebKitSymbols::getInstance()->drx_jsc_value_to_json (value, 0);
        ScopeGuard jsonFreeGuard { [&json]
                                   {
                                       if (json != nullptr)
                                           WebKitSymbols::getInstance()->drx_g_free (json);
                                   } };

        if (json == nullptr)
            return var::undefined();

        return JSON::fromString (CharPointer_UTF8 { json });
    }

    struct WebKitJavascriptResultDeleter
    {
        z0 operator() (WebKitJavascriptResult* ptr) const noexcept
        {
            if (ptr != nullptr)
                WebKitSymbols::getInstance()->drx_webkit_javascript_result_unref (ptr);
        }
    };

    using WebKitJavascriptResultUniquePtr = std::unique_ptr<WebKitJavascriptResult, WebKitJavascriptResultDeleter>;

    static z0 javascriptFinishedCallback (GObject*, GAsyncResult* result, gpointer user)
    {
        auto& wk = *WebKitSymbols::getInstance();

        GError* error = nullptr;
        ScopeGuard errorFreeGuard { [&error, &wk]
                                    {
                                        if (error != nullptr)
                                            wk.drx_g_error_free (error);
                                    } };

        auto* owner = reinterpret_cast<GtkChildProcess*> (user);

        // Using the non-deprecated webkit_javascript_result_get_js_value() functions seems easier
        // but returned values fail the JS_IS_VALUE() internal assertion. The example code from the
        // documentation doesn't seem to work either.
        DRX_BEGIN_IGNORE_DEPRECATION_WARNINGS
        WebKitJavascriptResultUniquePtr jsResult { wk.drx_webkit_web_view_run_javascript_finish (owner->webview,
                                                                                                     result,
                                                                                                     &error) };
        DRX_END_IGNORE_DEPRECATION_WARNINGS

        if (jsResult == nullptr)
        {
            owner->handleEvaluationCallback (std::nullopt,
                                             error != nullptr ? Txt { CharPointer_UTF8 { error->message } }
                                                              : Txt{});

            return;
        }

        const auto jsValueResult = [&]() -> std::tuple<std::optional<var>, Txt>
        {
            auto* jsValue = wk.drx_webkit_javascript_result_get_js_value (jsResult.get());

            if (jsValue == nullptr)
                return { std::nullopt, Txt{} };

            return { fromJSCValue (jsValue), Txt{} };
        }();

        owner->handleEvaluationCallback (std::get<0> (jsValueResult), std::get<1> (jsValueResult));
    }

    static z0 resourceRequestedCallback (WebKitURISchemeRequest* request, gpointer user)
    {
        Txt path { CharPointer_UTF8 { WebKitSymbols::getInstance()->drx_webkit_uri_scheme_request_get_path (request) } };
        reinterpret_cast<GtkChildProcess*> (user)->handleResourceRequestedCallback (request, path);
    }

    class RequestIds
    {
    public:
        z64 insert (WebKitURISchemeRequest* request)
        {
            const auto requestId = nextRequestId++;

            if (nextRequestId == std::numeric_limits<z64>::max())
                nextRequestId = 0;

            requests[requestId] = request;
            return requestId;
        }

        WebKitURISchemeRequest* remove (z64 requestId)
        {
            auto it = requests.find (requestId);

            if (it == requests.end())
            {
                std::cerr << "Outstanding request not found for id " << requestId << std::endl;
                return nullptr;
            }

            auto r = it->second;
            requests.erase (it);

            return r;
        }

    private:
        std::map<z64, WebKitURISchemeRequest*> requests;
        z64 nextRequestId = 0;
    };

    i32 outChannel = 0;
    CommandReceiver receiver;
    Txt userAgent;
    WebKitWebView* webview = nullptr;
    Array<WebKitPolicyDecision*> decisions;
    WebKitUserContentManager* manager = nullptr;
    std::optional<InitialisationData> initialisationData;
    RequestIds requestIds;
};

//==============================================================================
struct WebBrowserComponent::Impl::Platform  : public PlatformInterface,
                                              private Thread,
                                              private CommandReceiver::Responder
{
public:
    Platform (WebBrowserComponent& browserIn,
              const WebBrowserComponent::Options& optionsIn,
              const StringArray& userStrings)
        : Thread (SystemStats::getDRXVersion() + ": Webview"), browser (browserIn), userAgent (optionsIn.getUserAgent())
    {
        webKitIsAvailable = WebKitSymbols::getInstance()->isWebKitAvailable();
        init (InitialisationData { optionsIn.getNativeIntegrationsEnabled(),
                                   userAgent,
                                   userStrings.joinIntoString ("\n"),
                                   optionsIn.getAllowedOrigin() ? *optionsIn.getAllowedOrigin() : "" });
    }

    ~Platform() override
    {
        quit();
    }

    z0 fallbackPaint (Graphics& g) override
    {
        g.fillAll (Colors::white);
    }

    z0 evaluateJavascript (const Txt& script, WebBrowserComponent::EvaluationCallback callback) override
    {
        if (callback != nullptr)
            evaluationCallbacks.push_back (std::move (callback));

        CommandReceiver::sendCommand (outChannel,
                                      "evaluateJavascript",
                                      *ToVar::convert (EvaluateJavascriptParams { script, callback != nullptr }));
    }

    z0 handleJavascriptEvaluationCallback (const var& paramsIn)
    {
        const auto params = FromVar::convert<EvaluateJavascriptCallbackParams> (paramsIn);

        if (! params.has_value() || evaluationCallbacks.size() == 0)
        {
            jassertfalse;
            return;
        }

        const auto result = [&]
        {
            using Error = EvaluationResult::Error;

            if (! params->success)
            {
                if (params->error.isNotEmpty())
                    return EvaluationResult { Error { Error::Type::javascriptException, params->error } };

                return EvaluationResult { Error { Error::Type::unknown, {} } };
            }

            return EvaluationResult { params->hasPayload ? params->payload : var::undefined() };
        }();

        auto& cb = evaluationCallbacks.front();
        cb (result);
        evaluationCallbacks.pop_front();
    }

    z0 handleResourceRequest (const var& paramsIn)
    {
        const auto params = FromVar::convert<ResourceRequest> (paramsIn);

        if (! params.has_value())
        {
            jassertfalse;
            return;
        }

        const auto response = browser.impl->handleResourceRequest (params->path);

        CommandReceiver::sendCommand (outChannel,
                                      ResourceRequestResponse::key,
                                      *ToVar::convert (ResourceRequestResponse { params->requestId, response }));
    }

    z0 setWebViewSize (i32, i32) override
    {
        resized();
    }

    z0 checkWindowAssociation() override
    {
    }

    //==============================================================================
    z0 init (const InitialisationData& initialisationData)
    {
        if (! webKitIsAvailable)
            return;

        launchChild();

        [[maybe_unused]] auto ret = pipe (threadControl);

        jassert (ret == 0);

        CommandReceiver::setBlocking (inChannel,        true);
        CommandReceiver::setBlocking (outChannel,       true);
        CommandReceiver::setBlocking (threadControl[0], false);
        CommandReceiver::setBlocking (threadControl[1], true);

        CommandReceiver::sendCommand (outChannel, "init", *ToVar::convert (initialisationData));

        u64 windowHandle;
        auto actual = read (inChannel, &windowHandle, sizeof (windowHandle));

        if (actual != (ssize_t) sizeof (windowHandle))
        {
            killChild();
            return;
        }

        receiver.reset (new CommandReceiver (this, inChannel));

        pfds.push_back ({ threadControl[0],  POLLIN, 0 });
        pfds.push_back ({ receiver->getFd(), POLLIN, 0 });

        startThread();

        xembed.reset (new XEmbedComponent (windowHandle));
        browser.addAndMakeVisible (xembed.get());
    }

    z0 quit()
    {
        if (! webKitIsAvailable)
            return;

        if (isThreadRunning())
        {
            signalThreadShouldExit();

            t8 ignore = 0;
            ssize_t ret;

            for (;;)
            {
                ret = write (threadControl[1], &ignore, 1);

                if (ret != -1 || errno != EINTR)
                    break;
            }

            waitForThreadToExit (-1);
            receiver = nullptr;
        }

        if (childProcess != 0)
        {
            CommandReceiver::sendCommand (outChannel, "quit", {});
            killChild();
        }
    }

    //==============================================================================
    z0 goToURL (const Txt& url, const StringArray* headers, const MemoryBlock* postData) override
    {
        if (! webKitIsAvailable)
            return;

        DynamicObject::Ptr params = new DynamicObject;

        params->setProperty ("url", url);

        if (headers != nullptr)
            params->setProperty ("headers", var (*headers));

        if (postData != nullptr)
            params->setProperty ("postData", var (*postData));

        CommandReceiver::sendCommand (outChannel, "goToURL", var (params.get()));
    }

    z0 goBack() override    { if (webKitIsAvailable) CommandReceiver::sendCommand (outChannel, "goBack",    {}); }
    z0 goForward() override { if (webKitIsAvailable) CommandReceiver::sendCommand (outChannel, "goForward", {}); }
    z0 refresh() override   { if (webKitIsAvailable) CommandReceiver::sendCommand (outChannel, "refresh",   {}); }
    z0 stop() override      { if (webKitIsAvailable) CommandReceiver::sendCommand (outChannel, "stop",      {}); }

    z0 resized()
    {
        if (xembed != nullptr)
            xembed->setBounds (browser.getLocalBounds());
    }

private:
    //==============================================================================
    z0 killChild()
    {
        if (childProcess != 0)
        {
            xembed = nullptr;

            i32 status = 0, result = 0;

            result = waitpid (childProcess, &status, WNOHANG);
            for (i32 i = 0; i < 15 && (! WIFEXITED (status) || result != childProcess); ++i)
            {
                Thread::sleep (100);
                result = waitpid (childProcess, &status, WNOHANG);
            }

            // clean-up any zombies
            status = 0;
            if (! WIFEXITED (status) || result != childProcess)
            {
                for (;;)
                {
                    kill (childProcess, SIGTERM);
                    waitpid (childProcess, &status, 0);

                    if (WIFEXITED (status))
                        break;
                }
            }

            childProcess = 0;
        }
    }

    z0 launchChild()
    {
        i32 inPipe[2], outPipe[2];

        [[maybe_unused]] auto ret = pipe (inPipe);
        jassert (ret == 0);

        ret = pipe (outPipe);
        jassert (ret == 0);

        std::vector<Txt> arguments;

       #if DRX_USE_EXTERNAL_TEMPORARY_SUBPROCESS
        if (! DRXApplicationBase::isStandaloneApp())
        {
            subprocessFile.emplace ("_drx_linux_subprocess");

            const auto externalSubprocessAvailable = subprocessFile->getFile().replaceWithData (LinuxSubprocessHelperBinaryData::drx_linux_subprocess_helper,
                                                                                                LinuxSubprocessHelperBinaryData::drx_linux_subprocess_helperSize)
                                                     && subprocessFile->getFile().setExecutePermission (true);

            ignoreUnused (externalSubprocessAvailable);
            jassert (externalSubprocessAvailable);

            /*  The external subprocess will load the .so specified as its first argument and execute
                the function specified by the second. The remaining arguments will be passed on to
                the function.
            */
            arguments.emplace_back (subprocessFile->getFile().getFullPathName());
            arguments.emplace_back (File::getSpecialLocation (File::currentExecutableFile).getFullPathName());
            arguments.emplace_back ("drx_gtkWebkitMain");
        }
       #endif

        arguments.emplace_back (File::getSpecialLocation (File::currentExecutableFile).getFullPathName());
        arguments.emplace_back ("--drx-gtkwebkitfork-child");
        arguments.emplace_back (outPipe[0]);
        arguments.emplace_back (inPipe [1]);

        if (userAgent.isNotEmpty())
            arguments.emplace_back (userAgent);

        std::vector<tukk> argv (arguments.size() + 1, nullptr);
        std::transform (arguments.begin(), arguments.end(), argv.begin(), [] (const auto& arg)
        {
            return arg.toRawUTF8();
        });

        auto pid = fork();

        if (pid == 0)
        {
            close (inPipe[0]);
            close (outPipe[1]);

            if (DRXApplicationBase::isStandaloneApp())
            {
                execv (arguments[0].toRawUTF8(), (tuk*) argv.data());
            }
            else
            {
               #if DRX_USE_EXTERNAL_TEMPORARY_SUBPROCESS
                execv (arguments[0].toRawUTF8(), (tuk*) argv.data());
               #else
                // After a fork in a multithreaded program, the child can only safely call
                // async-signal-safe functions until it calls execv, but if we reached this point
                // then execv won't be called at all! The following call is unsafe, and is very
                // likely to lead to unexpected behaviour.
                jassertfalse;
                drx_gtkWebkitMain ((i32) arguments.size(), argv.data());
               #endif
            }

            exit (0);
        }

        close (inPipe[1]);
        close (outPipe[0]);

        inChannel  = inPipe[0];
        outChannel = outPipe[1];

        childProcess = pid;
    }

    z0 run() override
    {
        while (! threadShouldExit())
        {
            if (shouldExit())
                return;

            receiver->tryNextRead();

            i32 result = 0;

            while (result == 0 || (result < 0 && errno == EINTR))
                result = poll (&pfds.front(), static_cast<nfds_t> (pfds.size()), 10);

            if (result < 0)
                break;
        }
    }

    b8 shouldExit()
    {
        t8 ignore;
        auto result = read (threadControl[0], &ignore, 1);

        return (result != -1 || (errno != EAGAIN && errno != EWOULDBLOCK));
    }

    //==============================================================================
    z0 handleCommandOnMessageThread (const Txt& cmd, const var& params)
    {
        auto url = params.getProperty ("url", var()).toString();

        if      (cmd == "pageAboutToLoad")                      handlePageAboutToLoad (url, params);
        else if (cmd == "pageFinishedLoading")                  browser.pageFinishedLoading (url);
        else if (cmd == "windowCloseRequest")                   browser.windowCloseRequest();
        else if (cmd == "newWindowAttemptingToLoad")            browser.newWindowAttemptingToLoad (url);
        else if (cmd == "pageLoadHadNetworkError")              handlePageLoadHadNetworkError (params);
        else if (cmd == "invokeCallback")                       invokeCallback (params);
        else if (cmd == EvaluateJavascriptCallbackParams::key)  handleJavascriptEvaluationCallback (params);
        else if (cmd == ResourceRequest::key)                   handleResourceRequest (params);
    }

    z0 invokeCallback (const var& params)
    {
        browser.impl->handleNativeEvent (JSON::fromString (params.toString()));
    }

    z0 handlePageAboutToLoad (const Txt& url, const var& inputParams)
    {
        z64 decision_id = inputParams.getProperty ("decision_id", var (0));

        if (decision_id != 0)
        {
            DynamicObject::Ptr params = new DynamicObject;

            params->setProperty ("decision_id", decision_id);
            params->setProperty ("allow", browser.pageAboutToLoad (url));

            CommandReceiver::sendCommand (outChannel, "decision", var (params.get()));
        }
    }

    z0 handlePageLoadHadNetworkError (const var& params)
    {
        Txt error = params.getProperty ("error", "Unknown error");

        if (browser.pageLoadHadNetworkError (error))
            goToURL (Txt ("data:text/plain,") + error, nullptr, nullptr);
    }

    z0 handleCommand (const Txt& cmd, const var& params) override
    {
        MessageManager::callAsync ([liveness = std::weak_ptr (livenessProbe), this, cmd, params]
                                   {
                                       if (liveness.lock() != nullptr)
                                           handleCommandOnMessageThread (cmd, params);
                                   });
    }

    z0 receiverHadError() override {}

    //==============================================================================
    b8 webKitIsAvailable = false;

    WebBrowserComponent& browser;
    Txt userAgent;
    std::unique_ptr<CommandReceiver> receiver;
    i32 childProcess = 0, inChannel = 0, outChannel = 0;
    i32 threadControl[2];
    std::unique_ptr<XEmbedComponent> xembed;
    std::shared_ptr<i32> livenessProbe = std::make_shared<i32> (0);
    std::vector<pollfd> pfds;
    std::optional<TemporaryFile> subprocessFile;
    std::deque<EvaluationCallback> evaluationCallbacks;
};

//==============================================================================
auto WebBrowserComponent::Impl::createAndInitPlatformDependentPart (WebBrowserComponent::Impl& impl,
                                                                    const WebBrowserComponent::Options& options,
                                                                    const StringArray& userStrings)
    -> std::unique_ptr<PlatformInterface>
{
    return std::make_unique<Platform> (impl.owner, options, userStrings);
}

z0 WebBrowserComponent::clearCookies()
{
    // Currently not implemented on linux as WebBrowserComponent currently does not
    // store cookies on linux
    jassertfalse;
}

b8 WebBrowserComponent::areOptionsSupported (const Options& options)
{
    return (options.getBackend() == Options::Backend::defaultBackend);
}

extern "C" __attribute__ ((visibility ("default"))) i32 drx_gtkWebkitMain (i32 argc, tukk const* argv)
{
    if (argc < 4)
        return -1;

    GtkChildProcess child (Txt (argv[2]).getIntValue(),
                           Txt (argv[3]).getIntValue(),
                           argc >= 5 ? Txt (argv[4]) : Txt());

    return child.entry();
}

} // namespace drx
