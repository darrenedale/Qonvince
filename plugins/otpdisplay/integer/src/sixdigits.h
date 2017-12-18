#ifndef QONVINCE_OTPDISPLAYPLUGIN_SIXDIGITS_H
#define QONVINCE_OTPDISPLAYPLUGIN_SIXDIGITS_H

#include "otpdisplayplugin.h"

class SixDigitsPlugin
:	public LibQonvince::OtpDisplayPlugin {
	LIBQONVINCE_ANALYSIS_OTPDISPLAYPLUGIN

	virtual QString codeDisplayString(const QByteArray & hmac) const override;
};

#endif // QONVINCE_OTPDISPLAYPLUGIN_SIXDIGITS_H
