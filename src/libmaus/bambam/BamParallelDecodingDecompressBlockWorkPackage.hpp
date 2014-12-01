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
#if ! defined(LIBMAUS_BAMBAM_BAMPARALLELDECODINGDECOMPRESSBLOCKWORKPACKAGE_HPP)
#define LIBMAUS_BAMBAM_BAMPARALLELDECODINGDECOMPRESSBLOCKWORKPACKAGE_HPP

#include <libmaus/bambam/BamParallelDecodingControlInputInfo.hpp>
#include <libmaus/bambam/BamParallelDecodingDecompressedBlock.hpp>
#include <libmaus/parallel/SimpleThreadWorkPackage.hpp>

namespace libmaus
{
	namespace bambam
	{
		struct BamParallelDecodingDecompressBlockWorkPackage : public libmaus::parallel::SimpleThreadWorkPackage
		{
			typedef BamParallelDecodingDecompressBlockWorkPackage this_type;
			typedef libmaus::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef libmaus::util::shared_ptr<this_type>::type shared_ptr_type;
		
			BamParallelDecodingControlInputInfo::input_block_type * inputblock;
			BamParallelDecodingDecompressedBlock * outputblock;
			libmaus::lz::BgzfInflateZStreamBase * decoder;

			BamParallelDecodingDecompressBlockWorkPackage()
			: 
				libmaus::parallel::SimpleThreadWorkPackage(), 
				inputblock(0),
				outputblock(0),
				decoder(0)
			{
			}
			
			BamParallelDecodingDecompressBlockWorkPackage(
				uint64_t const rpriority, 
				BamParallelDecodingControlInputInfo::input_block_type * rinputblock,
				BamParallelDecodingDecompressedBlock * routputblock,
				libmaus::lz::BgzfInflateZStreamBase * rdecoder,
				uint64_t const rdecompressDispatcherId
			)
			: libmaus::parallel::SimpleThreadWorkPackage(rpriority,rdecompressDispatcherId), inputblock(rinputblock), outputblock(routputblock), decoder(rdecoder)
			{
			}
		
			char const * getPackageName() const
			{
				return "BamParallelDecodingDecompressBlockWorkPackage";
			}
		};
	}
}
#endif
