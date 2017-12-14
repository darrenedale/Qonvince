#ifndef QONVINCE_ALGORITHMS_H
#define QONVINCE_ALGORITHMS_H

#include <string>
#include <QHash>

namespace Qonvince {

	template<class QtType>
	struct QtHash final {
		std::size_t operator()(const QtType & value) const {
			return static_cast<std::size_t>(qHash(value));
		}
	};

}  // namespace Qonvince

#endif  // QONVINCE_ALGORITHMS_H
