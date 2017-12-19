#include "qtiostream.h"

#include <QString>
#include <QUrl>
#include <QRect>
#include <QPoint>


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


	std::ostream & operator<<(std::ostream & out, const QRect & rect) {
		out << rect.topLeft() << '-' << rect.bottomRight();
		return out;
	}


	std::ostream & operator<<(std::ostream & out, const QPoint & point) {
		out << '(' << point.x() << ", " << point.y() << ')';
		return out;
	}


}  // namespace Qonvince
