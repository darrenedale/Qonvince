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

	// convenience algorithms to work with STL and QT containers transparently
	template<class ContainerT, typename ValueT = typename ContainerT::value_type>
	bool contains(const ContainerT & container, const ValueT & value) {
		const auto & end = container.cend();
		return end != std::find(container.cbegin(), end, value);
	}

	template<class ContainerT, typename ValueT = typename ContainerT::value_type>
	int removeAll(ContainerT & container, const ValueT & value) {
		const auto originalEnd = container.end();
		const auto newEnd = std::remove(container.begin(), originalEnd, value);

		if(newEnd == originalEnd) {
			return 0;
		}

		auto ret = static_cast<int>(std::distance(newEnd, originalEnd));
		container.erase(newEnd, originalEnd);
		return ret;
	}

}  // namespace Qonvince

#endif  // QONVINCE_ALGORITHMS_H
