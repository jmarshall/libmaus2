/*
    libmaus2
    Copyright (C) 2015 German Tischler

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <libmaus2/LibMausConfig.hpp>

#if defined(LIBMAUS2_HAVE_GLOBAL_ALIGNMENT_Y256_16)
#include <libmaus2/lcs/SimdX86GlobalAlignmentY256_16.hpp>
#include <libmaus2/lcs/SimdX86GlobalAlignmentConstants256.hpp>
#include <libmaus2/lcs/SimdX86GlobalAlignmentY256_16_def.hpp>
#include <libmaus2/lcs/SimdX86GlobalAlignmentTemplate.cpp> 
#include <libmaus2/lcs/SimdX86GlobalAlignmentY256_16_undef.hpp>
#endif
