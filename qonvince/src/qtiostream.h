#ifndef QONVINCE_QTIOSTREAM_H
#define QONVINCE_QTIOSTREAM_H

#include <iostream>

class QString;

namespace Qonvince {
	std::ostream & operator<<(std::ostream & out, const QString &);
}

#endif // QONVINCE_QTIOSTREAM_H
