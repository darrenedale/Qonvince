#include "qtiostream.h"

#include <QString>


namespace Qonvince {


	std::ostream & operator<<(std::ostream & out, const QString & str ) {
		out << qPrintable(str);
		return out;
	}


}
