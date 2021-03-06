/*
    libmaus2
    Copyright (C) 2009-2013 German Tischler
    Copyright (C) 2011-2013 Genome Research Limited

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

#include <libmaus2/util/PositiveDifferenceArray64.hpp>
			
void libmaus2::util::PositiveDifferenceArray64::serialise(std::ostream & out) const
{
	A->serialise(out);
}

libmaus2::util::PositiveDifferenceArray64::PositiveDifferenceArray64(::std::istream & in)
: A ( new ::libmaus2::util::Array864(in) )
{

}

libmaus2::util::PositiveDifferenceArray64::PositiveDifferenceArray64(::libmaus2::util::Array864::unique_ptr_type & rA)
: A(UNIQUE_PTR_MOVE(rA))
{

}
