#ifndef QONVINCE_QTIOSTREAM_H
#define QONVINCE_QTIOSTREAM_H

#include <iostream>

class QString;
class QStringRef;
class QUrl;

namespace Qonvince {
	std::ostream & operator<<(std::ostream & out, const QString &);
	std::ostream & operator<<(std::ostream & out, const QStringRef &);
	std::ostream & operator<<(std::ostream & out, const QUrl &);
}  // namespace Qonvince

#endif  // QONVINCE_QTIOSTREAM_H
