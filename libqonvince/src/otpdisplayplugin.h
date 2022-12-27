#ifndef QONVINCE_OTPDISPLAYPLUGIN_H
#define QONVINCE_OTPDISPLAYPLUGIN_H

#include <string>

#include "plugininfo.h"
#include "securestring.h"

#define LIBQONVINCE_OTPDISPLAYPLUGIN_PLUGIN_API_VERSION 2
#define LIBQONVINCE_OTPDISPLAYPLUGIN_PLUGIN_TYPE "OtpDisplayPlugin"

namespace LibQonvince
{
    class OtpDisplayPlugin
    {
    public:
        static const std::string PluginTypeName;
        static constexpr const int ApiVersion = LIBQONVINCE_OTPDISPLAYPLUGIN_PLUGIN_API_VERSION;

        OtpDisplayPlugin() = default;
        OtpDisplayPlugin(const OtpDisplayPlugin &) = delete;
        OtpDisplayPlugin(OtpDisplayPlugin &&) = delete;
        OtpDisplayPlugin & operator=(const OtpDisplayPlugin &) = delete;
        OtpDisplayPlugin & operator=(OtpDisplayPlugin &&) = delete;

        virtual ~OtpDisplayPlugin();

        // implemented by the DECLARE_LIBQONVINCE_OTPDISPLAYPLUGIN macro
        [[nodiscard]] virtual const std::string & name() const = 0;
        [[nodiscard]] virtual const std::string & displayName() const = 0;
        [[nodiscard]] virtual const std::string & description() const = 0;
        [[nodiscard]] virtual const std::string & author() const = 0;
        [[nodiscard]] virtual const std::string & versionString() const = 0;

        // plugin classes must implement this
        [[nodiscard]] virtual SecureString codeDisplayString(const SecureString & hmac) const = 0;
    };

    extern "C"
    {
        using OtpDisplayPluginInstanceFunction = OtpDisplayPlugin * (*) ();
    }

#define LIBQONVINCE_OTPDISPLAYPLUGIN                  \
public:                                               \
    const std::string & name() const override;        \
    const std::string & displayName() const override; \
    const std::string & description() const override; \
    const std::string & author() const override;      \
    const std::string & versionString() const override;

    // use the macros for the api version and plugin type name because we want the
    // content set at compile time not resolved at runtime (otherwise the checks in
    // PluginFactory would be circumventable)
#define DECLARE_LIBQONVINCE_OTPDISPLAYPLUGIN(className_, displayName_, description_, author_, version_) \
    extern LibQonvince::PluginInfo pluginInfo;                                                          \
    extern "C"                                                                                          \
    {                                                                                                   \
        LibQonvince::OtpDisplayPlugin * createInstance()                                                \
        {                                                                                               \
            return new className_();                                                                    \
        }                                                                                               \
    }                                                                                                   \
                                                                                                        \
    LibQonvince::PluginInfo pluginInfo = {                                                              \
      LIBQONVINCE_OTPDISPLAYPLUGIN_PLUGIN_API_VERSION, /* apiVersion */                                 \
      LIBQONVINCE_OTPDISPLAYPLUGIN_PLUGIN_TYPE,        /* pluginType (i.e. base class name)*/           \
      __FILE__,                                        /* fileName */                                   \
      #className_,                                     /* className */                                  \
      #className_,                                     /* pluginName */                                 \
      displayName_,                                    /* displayName */                                \
      description_,                                    /* description */                                \
      author_,                                         /* authorName */                                 \
      version_,                                        /* versionString */                              \
    };                                                                                                  \
                                                                                                        \
    const std::string & className_::name() const                                                        \
    {                                                                                                   \
        static const std::string s_name = #className_;                                                  \
        return s_name;                                                                                  \
    }                                                                                                   \
                                                                                                        \
    const std::string & className_::displayName() const                                                 \
    {                                                                                                   \
        static const std::string s_displayName = displayName_;                                          \
        return s_displayName;                                                                           \
    }                                                                                                   \
                                                                                                        \
    const std::string & className_::description() const                                                 \
    {                                                                                                   \
        static const std::string s_description = description_;                                          \
        return s_description;                                                                           \
    }                                                                                                   \
                                                                                                        \
    const std::string & className_::author() const                                                      \
    {                                                                                                   \
        static const std::string s_author = author_;                                                    \
        return s_author;                                                                                \
    }                                                                                                   \
                                                                                                        \
    const std::string & className_::versionString() const                                               \
    {                                                                                                   \
        static const std::string s_version = version_;                                                  \
        return s_version;                                                                               \
    }
}  // namespace LibQonvince

#endif  // QONVINCE_OTPDISPLAYPLUGIN_H
