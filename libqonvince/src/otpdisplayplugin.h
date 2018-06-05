#ifndef QONVINCE_OTPDISPLAYPLUGIN_H
#define QONVINCE_OTPDISPLAYPLUGIN_H

#include <QString>

#include "plugininfo.h"

class QByteArray;

#define LIBQONVINCE_OTPDISPLAYPLUGIN_PLUGIN_API_VERSION 1
#define LIBQONVINCE_OTPDISPLAYPLUGIN_PLUGIN_TYPE "OtpDisplayPlugin"

namespace LibQonvince {

	class OtpDisplayPlugin {
	public:
		static const QString PlugnTypeName;
		static constexpr const int ApiVersion = LIBQONVINCE_OTPDISPLAYPLUGIN_PLUGIN_API_VERSION;

		OtpDisplayPlugin() = default;
		OtpDisplayPlugin(const OtpDisplayPlugin &) = delete;
		OtpDisplayPlugin(OtpDisplayPlugin &&) = delete;
		OtpDisplayPlugin & operator=(const OtpDisplayPlugin &) = delete;
		OtpDisplayPlugin & operator=(OtpDisplayPlugin &&) = delete;

		virtual ~OtpDisplayPlugin();

		// implemented by the LIBQONVINCE_ANALYSIS_PLUGIN macro
		virtual const QString & name() const = 0;
		virtual const QString & displayName() const = 0;
		virtual const QString & description() const = 0;
		virtual const QString & author() const = 0;
		virtual const QString & versionString() const = 0;

		// plugin classes must implement these
		virtual QString codeDisplayString(const QByteArray & hmac) const = 0;
	};

	extern "C" {
	using OtpDisplayPluginInstanceFunction = OtpDisplayPlugin * (*) ();
	}

#define LIBQONVINCE_OTPDISPLAYPLUGIN                     \
public:                                                  \
	virtual const QString & name() const override;        \
	virtual const QString & displayName() const override; \
	virtual const QString & description() const override; \
	virtual const QString & author() const override;      \
	virtual const QString & versionString() const override;

	// use the macros for the api version and plugin type name because we want the
	// content set at compile time not resolved at runtime (otherwise the checks in
	// PluginFactory would be circumventable)
#define DECLARE_LIBQONVINCE_OTPDISPLAYPLUGIN(className_, displayName_, description_, author_, version_) \
	extern LibQonvince::PluginInfo pluginInfo;                                                           \
	extern "C" {                                                                                         \
	LibQonvince::OtpDisplayPlugin * createInstance() {                                                   \
		return new className_();                                                                          \
	}                                                                                                    \
	}                                                                                                    \
																										\
	LibQonvince::PluginInfo pluginInfo = {                                                               \
	  LIBQONVINCE_OTPDISPLAYPLUGIN_PLUGIN_API_VERSION,				/* apiVersion */                         \
	  QStringLiteral(LIBQONVINCE_OTPDISPLAYPLUGIN_PLUGIN_TYPE), /* pluginType (i.e. base class name)*/   \
	  QStringLiteral(__FILE__),											/* fileName */                           \
	  QStringLiteral(#className_),										/* className */                          \
	  QStringLiteral(#className_),										/* pluginName */                         \
	  QStringLiteral(displayName_),										/* displayName */                        \
	  QStringLiteral(description_),										/* description */                        \
	  QStringLiteral(author_),												/* authorName */                         \
	  QStringLiteral(version_),											/* versionString */                      \
	};                                                                                                   \
																										\
	const QString & className_::name() const {                                                           \
		static const QString s_name = QStringLiteral(#className_);                                        \
		return s_name;                                                                                    \
	}                                                                                                    \
																										\
	const QString & className_::displayName() const {                                                    \
		static const QString s_displayName = QStringLiteral(displayName_);                                \
		return s_displayName;                                                                             \
	}                                                                                                    \
																										\
	const QString & className_::description() const {                                                    \
		static const QString s_description = QStringLiteral(description_);                                \
		return s_description;                                                                             \
	}                                                                                                    \
																										\
	const QString & className_::author() const {                                                         \
		static const QString s_author = QStringLiteral(author_);                                          \
		return s_author;                                                                                  \
	}                                                                                                    \
																										\
	const QString & className_::versionString() const {                                                  \
		static const QString s_version = QStringLiteral(version_);                                        \
		return s_version;                                                                                 \
	}

}  // namespace LibQonvince

#endif  // QONVINCE_OTPDISPLAYPLUGIN_H
