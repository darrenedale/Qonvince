#ifndef LIBQONVINCE_PLUGININFO_H
#define LIBQONVINCE_PLUGININFO_H

namespace LibQonvince
{
    extern "C"
    {
        struct PluginInfo
        {
            const int apiVersion;
            const std::string pluginType;
            const std::string fileName;
            const std::string className;
            const std::string pluginName;
            const std::string displayName;
            const std::string description;
            const std::string authorName;
            const std::string versionString;
        };
    }
}  // namespace LibQonvince

#endif  // LIBQONVINCE_PLUGININFO_H
