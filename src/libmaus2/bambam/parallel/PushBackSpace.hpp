/*
    libmaus2
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
#if ! defined(LIBMAUS2_BAMBAM_PARALLEL_PUSHBACKSPACE_HPP)
#define LIBMAUS2_BAMBAM_PARALLEL_PUSHBACKSPACE_HPP

#include <libmaus2/bambam/BamAlignment.hpp>
#include <libmaus2/util/GrowingFreeList.hpp>
#include <stack>

namespace libmaus2
{
	namespace bambam
	{
		namespace parallel
		{
			struct PushBackSpace
			{
				libmaus2::util::GrowingFreeList<libmaus2::bambam::BamAlignment> algnFreeList;
				std::stack<libmaus2::bambam::BamAlignment *> putbackStack;
				
				size_t byteSize()
				{
					return
						algnFreeList.byteSize() +
						putbackStack.size() * sizeof(libmaus2::bambam::BamAlignment *);
				}
	
				PushBackSpace()
				: algnFreeList(), putbackStack()
				{
				
				}
				
				void push(uint8_t const * D, uint64_t const bs)
				{
					libmaus2::bambam::BamAlignment * algn = algnFreeList.get();
					algn->copyFrom(D,bs);
					putbackStack.push(algn);
				}
				
				bool empty() const
				{
					return putbackStack.empty();
				}
				
				libmaus2::bambam::BamAlignment * top()
				{
					assert ( ! empty() );
					return putbackStack.top();
				}
				
				void pop()
				{
					putbackStack.pop();
				}
			};
		}
	}
}
#endif
