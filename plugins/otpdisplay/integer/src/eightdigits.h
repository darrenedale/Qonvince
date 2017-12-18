#ifndef QONVINCE_OTPDISPLAYPLUGIN_EIGHTDIGITS_H
#define QONVINCE_OTPDISPLAYPLUGIN_EIGHTDIGITS_H

#include "otpdisplayplugin.h"

#include <QString>
#include <QByteArray>

class EightDigitsPlugin
:	public LibQonvince::OtpDisplayPlugin {
	LIBQONVINCE_ANALYSIS_OTPDISPLAYPLUGIN

	virtual QString codeDisplayString(const QByteArray & hmac) const override;
};

#endif // QONVINCE_OTPDISPLAYPLUGIN_EIGHTDIGITS_H
