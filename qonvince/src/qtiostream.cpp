#include "qtiostream.h"

#include <QString>


namespace Qonvince {


	std::ostream & operator<<(std::ostream & out, const QString & str) {
		out << qPrintable(str);
		return out;
	}


	std::ostream & operator<<(std::ostream & out, const QStringRef & str) {
		out << str.toString();
		return out;
	}


}  // namespace Qonvince
