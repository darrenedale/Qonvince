#ifndef QONVINCE_ALGORITHMS_H
#define QONVINCE_ALGORITHMS_H

#include <cstddef>

namespace Qonvince {
	/* a class to use qHash() to hash Qt types for use in STL maps */
	template<typename QtType>
	class QtHash {
		public:
			std::size_t operator()( const QtType & obj ) const {
				return static_cast<std::size_t>(qHash(obj));
			}
	};
}

#endif // QONVINCE_ALGORITHMS_H
