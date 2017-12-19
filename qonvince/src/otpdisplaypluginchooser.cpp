#include "otpdisplaypluginchooser.h"

#include "application.h"


namespace Qonvince {


	OtpDisplayPluginChooser::OtpDisplayPluginChooser(QWidget * parent)
	: QComboBox(parent) {
		Q_ASSERT_X(qonvinceApp, __PRETTY_FUNCTION__, "can't create an OtpDisplayPluginChooser without a Qonvince::Application object");

		refreshPlugins();

		connect(this, qOverload<int>(&QComboBox::currentIndexChanged), [this](int) {
			Q_EMIT currentPluginChanged(currentPluginName());
		});
	}


	LibQonvince::OtpDisplayPlugin * OtpDisplayPluginChooser::currentPlugin() const {
		return qonvinceApp->otpDisplayPluginByName(currentPluginName());
	}


	void OtpDisplayPluginChooser::setCurrentPluginName(const QString & pluginName) {
		setCurrentIndex(findData(pluginName));
	}

	void OtpDisplayPluginChooser::refreshPlugins() {
		auto reblock = blockSignals(true);
		QString current = currentPluginName();
		clear();

		for(auto * plugin : qonvinceApp->otpDisplayPlugins()) {
			QComboBox::addItem(plugin->displayName(), plugin->name());
		}

		if(!current.isEmpty()) {
			setCurrentPluginName(current);
		}

		blockSignals(reblock);

		if(current != currentPluginName()) {
			Q_EMIT currentPluginChanged(currentPluginName());
		}
	}

}  // namespace Qonvince
