/*
 * Copyright 2018 Darren Edale
 *
 * This file is part of Qonvince.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef QONVINCE_QTENDIANEXTRA_H
#define QONVINCE_QTENDIANEXTRA_H

#include <QtEndian>

// fill in the missing template specialisations for standard int types -
// on some platforms uint64_t and quint64 are not identical types

#if(defined(Q_CC_GNU) && Q_CC_GNU >= 403) || QT_HAS_BUILTIN(__builtin_bswap32)

template<>
inline Q_DECL_CONSTEXPR uint64_t qbswap<uint64_t>(uint64_t source) {
	return __builtin_bswap64(source);
}

#else

template<>
inline Q_DECL_CONSTEXPR uint64_t qbswap<uint64_t>(uint64_t source) {
	return 0 | ((source & Q_UINT64_C(0x00000000000000ff)) << 56) | ((source & Q_UINT64_C(0x000000000000ff00)) << 40) | ((source & Q_UINT64_C(0x0000000000ff0000)) << 24) | ((source & Q_UINT64_C(0x00000000ff000000)) << 8) | ((source & Q_UINT64_C(0x000000ff00000000)) >> 8) | ((source & Q_UINT64_C(0x0000ff0000000000)) >> 24) | ((source & Q_UINT64_C(0x00ff000000000000)) >> 40) | ((source & Q_UINT64_C(0xff00000000000000)) >> 56);
}

#endif

#endif  // QONVINCE_QTENDIANEXTRA_H
