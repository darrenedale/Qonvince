#ifndef LIBQONVINCE_PLUGININFO_H
#define LIBQONVINCE_PLUGININFO_H

#include <QString>

namespace LibQonvince {

	extern "C" {
	struct PluginInfo {
		const int apiVersion;
		const QString pluginType;
		const QString fileName;
		const QString className;
		const QString pluginName;
		const QString displayName;
		const QString description;
		const QString authorName;
		const QString versionString;
	};
	}

}  // namespace LibQonvince

#endif  // LIBQONVINCE_PLUGININFO_H

