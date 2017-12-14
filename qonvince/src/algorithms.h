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

	template<typename T>
	void toUpper(std::basic_string<T> & str) {
		for(auto & c : str) {
			if('a' <= c && 'z' >= c) {
				c -= 32;
			}
		}
	}

	template<typename T>
	void toLower(std::basic_string<T> & str) {
		for(auto & c : str) {
			if('A' <= c && 'Z' >= c) {
				c += 32;
			}
		}
	}

}  // namespace Qonvince

#endif  // QONVINCE_ALGORITHMS_H
