#include "qtiostream.h"

#include <QString>
#include <QUrl>


namespace Qonvince {


	std::ostream & operator<<(std::ostream & out, const QString & str) {
		out << qPrintable(str);
		return out;
	}


	std::ostream & operator<<(std::ostream & out, const QStringRef & str) {
		out << str.toString();
		return out;
	}


	std::ostream & operator<<(std::ostream & out, const QUrl & url) {
		out << url.toString();
		return out;
	}


}  // namespace Qonvince
