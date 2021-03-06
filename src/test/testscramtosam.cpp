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
#include <libmaus2/bambam/ScramInputContainer.hpp>
#include <libmaus2/LibMausConfig.hpp>
#include <iostream>
#include <cstdlib>

#if ! defined(LIBMAUS2_HAVE_IO_LIB)
int main()
{
	std::cerr << "io_lib support is not present." << std::endl;
	return EXIT_FAILURE;
}
#else
#include <libmaus2/util/ArgInfo.hpp>
#include <libmaus2/bambam/ScramDecoder.hpp>
#include <libmaus2/bambam/ScramEncoder.hpp>

#if defined(LIBMAUS2_HAVE_IRODS)	
#include <libmaus2/irods/IRodsInputStreamFactory.hpp>
#endif

int main(int argc, char * argv[])
{
	try
	{
		::libmaus2::util::ArgInfo const arginfo(argc,argv);

		#if defined(LIBMAUS2_HAVE_IRODS)	
		libmaus2::irods::IRodsInputStreamFactory::registerHandler();
		#endif

		for ( size_t i = 0; i < arginfo.restargs.size(); ++i )
		{
			libmaus2::bambam::ScramDecoder deccb(arginfo.restargs[i],
				libmaus2::bambam::ScramInputContainer::allocate,
				libmaus2::bambam::ScramInputContainer::deallocate,8192,std::string(),false /* put rank*/);
			while ( deccb.readAlignment() )
				std::cout << deccb.getAlignment().formatAlignment(deccb.getHeader()) << std::endl;
		}

		::libmaus2::bambam::ScramDecoder dec("-","rc","");
		::libmaus2::bambam::ScramEncoder enc(dec.getHeader(),"-","w", "/nfs/srpipe_references/references/PhiX/Illumina/all/fasta/phix-illumina.fa");

		while ( dec.readAlignment() )
			enc.encode(dec.getAlignment());			
	}
	catch(std::exception const & ex)
	{
		std::cerr << ex.what() << std::endl;
		return EXIT_FAILURE;
	}
}
#endif
