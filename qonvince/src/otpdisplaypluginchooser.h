#ifndef OTPDISPLAYPLUGINCHOOSER_H
#define OTPDISPLAYPLUGINCHOOSER_H

#include <QObject>
#include <QComboBox>
#include <QString>

namespace LibQonvince {
	class OtpDisplayPlugin;
}

namespace Qonvince {

	class OtpDisplayPluginChooser
	: public QComboBox {
			Q_OBJECT
		public:
			OtpDisplayPluginChooser(QWidget * parent = nullptr);

			void addItem() = delete;
			void addItems() = delete;
			void setItemData() = delete;
			void setItemIcon() = delete;
			void setItemText() = delete;

			inline QString currentPluginName() const {
				return currentData().toString();
			}

			LibQonvince::OtpDisplayPlugin * currentPlugin() const;

		public Q_SLOTS:
			void setCurrentPluginName(const QString & pluginName);

		Q_SIGNALS:
			void currentPluginChanged(const QString & pluginName);

		private Q_SLOTS:
			void refreshPlugins();
	};

} // namespace Qonvince

#endif // OTPDISPLAYPLUGINCHOOSER_H
