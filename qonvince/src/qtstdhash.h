#ifndef QTSTDHASH_H
#define QTSTDHASH_H

#include <functional>
#include <QString>

namespace Qonvince {
	namespace Detail {
		template<class QtClass>
		struct QtHash {
			typedef std::size_t result_type;
			typedef QtClass argument_type;

			result_type operator()(const argument_type & arg) {
				return static_cast<std::size_t>(qHash(arg));
			}
		};
	}
}

namespace std {
	template<> struct hash<QString> : public Qonvince::Detail::QtHash<QString> {};
}

#endif // QTSTDHASH_H
