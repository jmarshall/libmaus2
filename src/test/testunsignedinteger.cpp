/*
    libmaus
    Copyright (C) 2009-2014 German Tischler
    Copyright (C) 2011-2014 Genome Research Limited

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
#include <libmaus/math/UnsignedInteger.hpp>
#include <iostream>

int main(int argc, char * argv[])
{
	libmaus::math::UnsignedInteger<2> N(5);
	libmaus::math::UnsignedInteger<2> M(7);
	libmaus::math::UnsignedInteger<2> R = N*M;
	libmaus::math::UnsignedInteger<2> T(10);
	
	std::cout << std::hex << R << std::dec << std::endl;
	
	std::cerr << T[0] << std::endl;
	std::cerr << R[0] << std::endl;
	std::cerr << (R/T)[0] << std::endl;
	std::cerr << (R%T)[0] << std::endl;
	
	std::cerr << R << std::endl;
}
