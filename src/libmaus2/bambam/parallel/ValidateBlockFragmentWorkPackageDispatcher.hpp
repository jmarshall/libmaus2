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
#if ! defined(LIBMAUS2_BAMBAM_PARALLEL_VALIDATEBLOCKWORKPACKAGEDISPATCHER_HPP)
#define LIBMAUS2_BAMBAM_PARALLEL_VALIDATEBLOCKWORKPACKAGEDISPATCHER_HPP

#include <libmaus2/bambam/parallel/ValidateBlockFragmentPackageReturnInterface.hpp>
#include <libmaus2/bambam/parallel/ValidateBlockFragmentAddPendingInterface.hpp>
#include <libmaus2/bambam/parallel/ValidateBlockFragmentWorkPackage.hpp>
#include <libmaus2/bambam/parallel/ChecksumsInterfaceGetInterface.hpp>
#include <libmaus2/bambam/parallel/ChecksumsInterfacePutInterface.hpp>

namespace libmaus2
{
	namespace bambam
	{
		namespace parallel
		{
			// dispatcher for block validation
			struct ValidateBlockFragmentWorkPackageDispatcher : public libmaus2::parallel::SimpleThreadWorkPackageDispatcher
			{
				ValidateBlockFragmentPackageReturnInterface   & packageReturnInterface;
				ValidateBlockFragmentAddPendingInterface & addValidatedPendingInterface;
				ChecksumsInterfaceGetInterface & getChecksumsInterface;
				ChecksumsInterfacePutInterface & putChecksumsInterface;
	
				ValidateBlockFragmentWorkPackageDispatcher(
					ValidateBlockFragmentPackageReturnInterface & rpackageReturnInterface,
					ValidateBlockFragmentAddPendingInterface    & raddValidatedPendingInterface,
					ChecksumsInterfaceGetInterface & rgetChecksumsInterface,
					ChecksumsInterfacePutInterface & rputChecksumsInterface
				) : packageReturnInterface(rpackageReturnInterface), addValidatedPendingInterface(raddValidatedPendingInterface),
				    getChecksumsInterface(rgetChecksumsInterface), putChecksumsInterface(rputChecksumsInterface)
				{
				}
			
				virtual void dispatch(
					libmaus2::parallel::SimpleThreadWorkPackage * P, 
					libmaus2::parallel::SimpleThreadPoolInterfaceEnqueTermInterface & /* tpi */
				)
				{
					ValidateBlockFragmentWorkPackage * BP = dynamic_cast<ValidateBlockFragmentWorkPackage *>(P);
					assert ( BP );
	
					bool const ok = BP->dispatch();
					
					if ( ok )
					{
						ChecksumsInterface::shared_ptr_type Schksums = getChecksumsInterface.getSeqChecksumsObject();
						if ( Schksums )
						{
							BP->updateChecksums(*Schksums);
							putChecksumsInterface.returnSeqChecksumsObject(Schksums);
						}
					}
									
					addValidatedPendingInterface.validateBlockFragmentFinished(BP->fragment,ok);
									
					// return the work package				
					packageReturnInterface.putReturnValidateBlockFragmentPackage(BP);				
				}		
			};
		}
	}
}
#endif
