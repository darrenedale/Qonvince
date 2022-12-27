#ifndef QONVINCE_PLUGINFACTORY_H
#define QONVINCE_PLUGINFACTORY_H

#include <memory>
#include <utility>
#include <vector>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <iostream>
#include <filesystem>

#include "sharedlibrary.h"
#include "plugininfo.h"

#if defined(Q_OS_UNIX)
#define QONVINCE_PLUGINFACTORY_DEFAULT_EXTENSION ".so"
#elif defined(Q_OS_WIN)
#define QONVINCE_PLUGINFACTORY_DEFAULT_EXTENSION ".dll"
#endif

using LibQonvince::SharedLibrary;
namespace fs = std::filesystem;

namespace Qonvince
{
    template<class PluginType>
    class PluginFactory final
    {
    public:
        using PathList = std::vector<std::string>;

        explicit PluginFactory(std::string extension = QONVINCE_PLUGINFACTORY_DEFAULT_EXTENSION)
                : m_extension(std::move(extension))
        {
        }

        explicit PluginFactory(const PathList & searchPaths, const std::string & extension = QONVINCE_PLUGINFACTORY_DEFAULT_EXTENSION)
                : PluginFactory(extension)
        {
            setSearchPaths(searchPaths);
        }

        PluginFactory(const PluginFactory & other) = delete;
        PluginFactory(PluginFactory && other) = delete;
        void operator=(const PluginFactory & other) = delete;
		  void operator=(PluginFactory && other) = delete;

        ~PluginFactory()
        {
            // plugin destructors *must* be called before libraries that contain them are closed. default destructor
            // would be fine as long as m_openLibs and m_loadedPlugins are delcared in the correct order. in case the
            // order is changed, we implement a custom destructor to enforce the order in which plugin and lib
            // destructors are called
            m_loadedPlugins.clear();
            m_openLibs.clear();
        }

        bool addSearchPath(const PathList::value_type & path)
        {
            auto isMatchingPath = [&path](const auto & existingPathEntry){
                return 0 == path.compare(existingPathEntry.path);
            };

            if (m_searchPaths.cend() != std::find_if(std::cbegin(m_searchPaths), std::cend(m_searchPaths), isMatchingPath)) {
                return false;
            }

            m_searchPaths.emplace_back(path, false);
            return true;
        }

        bool addSearchPaths(const PathList & paths)
        {
            bool ret = true;

            for (const auto & path: paths) {
                ret = addSearchPath(path) && ret;
            }

            return ret;
        }

        void setSearchPaths(PathList paths)
        {
            auto eraseFrom = std::unique(paths.begin(), paths.end());
            paths.erase(eraseFrom, paths.end());

            for (const auto & path: paths) {
                m_searchPaths.emplace_back(path, false);
            }

            m_loadedPlugins.clear();
        }

        std::vector<PluginType *> loadedPlugins() const
        {
            std::vector<PluginType *> ret;

            std::transform(m_loadedPlugins.cbegin(), m_loadedPlugins.cend(), std::back_inserter(ret),
                           [](const typename decltype(m_loadedPlugins)::value_type & plugin){
                               return plugin.second.get();
                           });

            return ret;
        }

        PluginType * pluginByName(const std::string & name)
        {
            loadAllPlugins();
            auto plugin = m_loadedPlugins.find(name);

            if (plugin == m_loadedPlugins.cend()) {
                return nullptr;
            }

            return plugin->second.get();
        }

        // this will load all plugins from paths that have yet to be searched
        void loadAllPlugins()
        {
            for (auto & searchPath: m_searchPaths) {
                if (!searchPath.searched) {
                    if (!std::filesystem::is_directory(searchPath.path)) {
                        continue;
                    }

                    loadPluginsFromPath(searchPath.path);
                    searchPath.searched = true;
                }
            }
        }

        bool loadPlugin(const std::string & path)
        {
            using InstanceFunction = PluginType * (*)();
            auto lib = SharedLibrary(path);

            if (!lib.isOpen()) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): failed to open plugin file \"" << path << "\" (\""
                          << lib.lastError() << "\")\n";
                return false;
            }

            if (!lib.hasSymbol("pluginInfo")) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): plugin \"" << path
                          << "\" does not provide a PluginInfo data structure\n";
                return false;
            }

            SharedLibrary::Symbol symbol = nullptr;

            if (!lib.symbol("pluginInfo", &symbol)) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): the plugin information could not be fetched from the plugin \""
                          << path << "\"\n";
                return false;
            }

            if (!symbol) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): the plugin information fetched from the plugin \"" << path
                          << "\" was null\n";
                return false;
            }

            auto * info = reinterpret_cast<LibQonvince::PluginInfo *>(symbol);
#if !defined(NDEBUG)
            std::cout << "\nPlugin type: " << info->pluginType << "\n";
            std::cout << "API version: " << info->apiVersion << "\n";
            std::cout << "Plugin name: " << info->pluginName << "\n";
            std::cout << "Class name: " << info->className << "\n";
            std::cout << "Display name: " << info->displayName << "\n";
            std::cout << "Plugin version string: " << info->versionString << "\n";
            std::cout << "Author: " << info->authorName << "\n";
            std::cout << "Description: " << info->description << "\n\n";
#endif
            if (0 != info->pluginType.compare(PluginType::PluginTypeName)) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): the plugin \"" << path << "\" is not a(n) "
                          << PluginType::PluginTypeName << "\n";
                return false;
            }

            if (PluginType::ApiVersion != info->apiVersion) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): the plugin \"" << path
                          << "\" has an incorrect API version (it has v" << info->apiVersion << ", expecting v" << PluginType::ApiVersion << ")\n";
                return false;
            }

            if (!lib.hasSymbol("createInstance")) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): plugin \"" << path
                          << "\" does not provide an createInstance() function\n";
                return false;
            }

            if (!lib.symbol("createInstance", &symbol)) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__
                          << "]): the createInstance() function could not be fetched from the plugin \"" << path << "\"\n";
                return false;
            }

            if (!symbol) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): the createInstance() function in the plugin \"" << path
                          << "\" is null\n";
                return false;
            }

            auto instanceFn = reinterpret_cast<InstanceFunction>(symbol);
            std::unique_ptr<PluginType> plugin(instanceFn());

            if (!plugin) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): the createInstance() function in the plugin \"" << path
                          << "\" did not provide a plugin instance\n";
                return false;
            }

            if (m_loadedPlugins.cend() != m_loadedPlugins.find(plugin->name())) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): the plugin loaded from the file \"" << path
                          << "\" has a name (\"" << plugin->name()
                          << "\") that is identical to another plugin that is already loaded and therefore cannot be used.\n";
                return false;
            }

            std::cout << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]) : successfully loaded plugin \"" << plugin->name() << "\" from \""
                      << path << "\"\n";
            m_openLibs.push_back(std::move(lib));
            m_loadedPlugins.insert(std::make_pair(plugin->name(), std::move(plugin)));
            return true;
        }

    private:
        void loadPluginsFromPath(const std::string & path)
        {
				const auto fsPath = fs::canonical(path);

				if (!fs::exists(fsPath)) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): path \"" << path << "\" does not exist or is not readable.\n";
                return;
            }

            if (!fs::is_directory(fsPath)) {
                std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): path \"" << path << "\" is not a directory.\n";
                return;
            }

            for (const auto & entry: fs::directory_iterator(fsPath)) {
                if (!entry.is_regular_file() || entry.is_symlink()) {
                    continue;
                }

                auto pluginPath = entry.path();

                if (!m_extension.empty() && pluginPath.extension() != m_extension) {
                    std::cerr << __PRETTY_FUNCTION__ << " (" << __FILE__ << " [" << __LINE__ << "]): file \"" << pluginPath << "\" does not have extension \""
                              << m_extension << "\"\n";
                    continue;
                }

                loadPlugin(pluginPath);
            }
        }

        struct SearchPathsEntry
        {
            SearchPathsEntry(std::string newPath, bool newSearched)
                    : path(std::move(newPath)),
                      searched(newSearched)
            {
            }

            PathList::value_type path;
            bool searched;
        };

        std::vector<SearchPathsEntry> m_searchPaths;
        const std::string m_extension;
        std::vector<SharedLibrary> m_openLibs;
        std::unordered_map<std::string, std::unique_ptr<PluginType>> m_loadedPlugins;
    };
}  // namespace Qonvince

#undef QONVINCE_PLUGINFACTORY_DEFAULT_EXTENSION

#endif  // QONVINCE_PLUGINFACTORY_H
