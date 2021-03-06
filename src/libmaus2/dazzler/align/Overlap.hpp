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
#if ! defined(LIBMAUS2_DAZZLER_ALIGN_OVERLAP_HPP)
#define LIBMAUS2_DAZZLER_ALIGN_OVERLAP_HPP

#include <libmaus2/dazzler/align/Path.hpp>
#include <libmaus2/lcs/Aligner.hpp>
		
namespace libmaus2
{
	namespace dazzler
	{
		namespace align
		{
			struct Overlap : public libmaus2::dazzler::db::InputBase, public libmaus2::dazzler::db::OutputBase
			{
				Path path;
				uint32_t flags;
				int32_t aread;
				int32_t bread;
				
				// 40
				
				uint64_t deserialise(std::istream & in)
				{
					uint64_t s = 0;
					
					s += path.deserialise(in);
					
					uint64_t offset = 0;
					flags = getUnsignedLittleEndianInteger4(in,offset);
					aread = getLittleEndianInteger4(in,offset);
					bread = getLittleEndianInteger4(in,offset);

					getLittleEndianInteger4(in,offset); // padding
					
					s += offset;
					
					// std::cerr << "flags=" << flags << " aread=" << aread << " bread=" << bread << std::endl;
					
					return s;
				}
				
				uint64_t serialise(std::ostream & out)
				{
					uint64_t s = 0;
					s += path.serialise(out);
					uint64_t offset = 0;
					putUnsignedLittleEndianInteger4(out,flags,offset);
					putLittleEndianInteger4(out,aread,offset);
					putLittleEndianInteger4(out,bread,offset);
					putLittleEndianInteger4(out,0,offset); // padding
					s += offset;
					return s;
				}
				
				uint64_t serialiseWithPath(std::ostream & out, bool const small) const
				{
					uint64_t s = 0;
					s += path.serialise(out);
					uint64_t offset = 0;
					putUnsignedLittleEndianInteger4(out,flags,offset);
					putLittleEndianInteger4(out,aread,offset);
					putLittleEndianInteger4(out,bread,offset);
					putLittleEndianInteger4(out,0,offset); // padding
					s += offset;
					s += path.serialisePath(out,small);
					return s;	
				}
				
				bool operator==(Overlap const & O) const
				{
					return
						compareMeta(O) && path == O.path;
				}
				
				bool compareMeta(Overlap const & O) const
				{
					return 
						path.comparePathMeta(O.path) &&
						flags == O.flags &&
						aread == O.aread &&
						bread == O.bread;
				}

				bool compareMetaLower(Overlap const & O) const
				{
					return 
						path.comparePathMetaLower(O.path) &&
						flags == O.flags &&
						aread == O.aread &&
						bread == O.bread;
				}
				
				Overlap()
				{
				
				}
				
				Overlap(std::istream & in)
				{
					deserialise(in);
				}

				Overlap(std::istream & in, uint64_t & s)
				{
					s += deserialise(in);
				}
				
				bool isInverse() const
				{
					return (flags & 1) != 0;
				}
				
				uint64_t getNumErrors() const
				{
					return path.getNumErrors();
				}
				
				static Overlap computeOverlap(
					int64_t const flags,
					int64_t const aread,
					int64_t const bread,
					int64_t const abpos,
					int64_t const aepos,
					int64_t const bbpos,
					int64_t const bepos,
					int64_t const tspace,
					libmaus2::lcs::AlignmentTraceContainer const & ATC
				)
				{
					Overlap OVL;
					OVL.flags = flags;
					OVL.aread = aread;
					OVL.bread = bread;
					OVL.path = computePath(abpos,aepos,bbpos,bepos,tspace,ATC);
					return OVL;
				}

				static Path computePath(
					int64_t const abpos,
					int64_t const aepos,
					int64_t const bbpos,
					int64_t const bepos,
					int64_t const tspace,
					libmaus2::lcs::AlignmentTraceContainer const & ATC
				)
				{
					Path path;
					
					path.abpos = abpos;
					path.bbpos = bbpos;
					path.aepos = aepos;
					path.bepos = bepos;
					path.diffs = 0;

					// current point on A
					int64_t a_i = ( path.abpos / tspace ) * tspace;
					
					libmaus2::lcs::AlignmentTraceContainer::step_type const * tc = ATC.ta;

					while ( a_i < aepos )
					{
						int64_t a_c = std::max(abpos,a_i);
						// block end point on A
						int64_t const a_i_1 = std::min ( static_cast<int64_t>(a_i + tspace), static_cast<int64_t>(path.aepos) );
						
						int64_t bforw = 0;
						int64_t err = 0;
						while ( a_c < a_i_1 )
						{
							assert ( tc < ATC.te );
							switch ( *(tc++) )
							{
								case libmaus2::lcs::AlignmentTraceContainer::STEP_MATCH:
									++a_c;
									++bforw;
									break;
								case libmaus2::lcs::AlignmentTraceContainer::STEP_MISMATCH:
									++a_c;
									++bforw;
									++err;
									break;
								case libmaus2::lcs::AlignmentTraceContainer::STEP_DEL:
									++a_c;
									++err;
									break;
								case libmaus2::lcs::AlignmentTraceContainer::STEP_INS:
									++bforw;
									++err;
									break;
							}
						}
						
						path.diffs += err;
						path.path.push_back(Path::tracepoint(err,bforw));
						
						a_i = a_i_1;
					}
					
					path.tlen = path.path.size() << 1;

					return path;
				}
				
				/**
				 * compute alignment trace
				 *
				 * @param aptr base sequence of aread
				 * @param bptr base sequence of bread if isInverse() returns false, reverse complement of bread if isInverse() returns true
				 * @param tspace trace point spacing
				 * @param ATC trace container for storing trace
				 * @param aligner aligner
				 **/
				void computeTrace(
					uint8_t const * aptr,
					uint8_t const * bptr,
					int64_t const tspace,
					libmaus2::lcs::AlignmentTraceContainer & ATC,
					libmaus2::lcs::Aligner & aligner
				) const
				{
					// current point on A
					int32_t a_i = ( path.abpos / tspace ) * tspace;
					// current point on B
					int32_t b_i = ( path.bbpos );
					
					// reset trace container
					ATC.reset();
					
					for ( size_t i = 0; i < path.path.size(); ++i )
					{
						// block end point on A
						int32_t const a_i_1 = std::min ( static_cast<int32_t>(a_i + tspace), static_cast<int32_t>(path.aepos) );
						// block end point on B
						int32_t const b_i_1 = b_i + path.path[i].second;

						// block on A
						uint8_t const * asubsub_b = aptr + std::max(a_i,path.abpos);
						uint8_t const * asubsub_e = asubsub_b + a_i_1-std::max(a_i,path.abpos);
						
						// block on B
						uint8_t const * bsubsub_b = bptr + b_i;
						uint8_t const * bsubsub_e = bsubsub_b + (b_i_1-b_i);

						aligner.align(asubsub_b,(asubsub_e-asubsub_b),bsubsub_b,bsubsub_e-bsubsub_b);
						
						// add trace to full alignment
						ATC.push(aligner.getTraceContainer());
						
						// update start points
						b_i = b_i_1;
						a_i = a_i_1;
					}
				}

				/**
				 * get error block histogram
				 *
				 * @param tspace trace point spacing
				 **/
				void fillErrorHistogram(
					int64_t const tspace, 
					std::map< uint64_t, std::map<uint64_t,uint64_t> > & H) const
				{
					// current point on A
					int32_t a_i = ( path.abpos / tspace ) * tspace;
					// current point on B
					int32_t b_i = ( path.bbpos );
										
					for ( size_t i = 0; i < path.path.size(); ++i )
					{
						// block end point on A
						int32_t const a_i_1 = std::min ( static_cast<int32_t>(a_i + tspace), static_cast<int32_t>(path.aepos) );
						// block end point on B
						int32_t const b_i_1 = b_i + path.path[i].second;

						if ( 
							(a_i_1 - a_i) == tspace 
							&&
							(a_i % tspace == 0)
							&&
							(a_i_1 % tspace == 0)
						)
							H [ a_i / tspace ][path.path[i].first]++;
						
						// update start points
						b_i = b_i_1;
						a_i = a_i_1;
					}
				}

				/**
				 * get bases in full blocks and number of errors in these blocks
				 *
				 * @param tspace trace point spacing
				 **/
				std::pair<uint64_t,uint64_t> fullBlockErrors(int64_t const tspace) const
				{
					// current point on A
					int32_t a_i = ( path.abpos / tspace ) * tspace;
					// current point on B
					int32_t b_i = ( path.bbpos );
					
					uint64_t errors = 0;
					uint64_t length = 0;
										
					for ( size_t i = 0; i < path.path.size(); ++i )
					{
						// block end point on A
						int32_t const a_i_1 = std::min ( static_cast<int32_t>(a_i + tspace), static_cast<int32_t>(path.aepos) );
						// block end point on B
						int32_t const b_i_1 = b_i + path.path[i].second;

						if ( 
							(a_i_1 - a_i) == tspace 
							&&
							(a_i % tspace == 0)
							&&
							(a_i_1 % tspace == 0)
						)
						{
							errors += path.path[i].first;
							length += tspace;
						}
						
						// update start points
						b_i = b_i_1;
						a_i = a_i_1;
					}
					
					return std::pair<uint64_t,uint64_t>(length,errors);
				}

				/**
				 * fill number of spanning reads for each sparse trace point on read
				 *
				 * @param tspace trace point spacing
				 **/
				void fillSpanHistogram(
					int64_t const tspace, 
					std::map< uint64_t, uint64_t > & H,
					uint64_t const ethres,
					uint64_t const cthres
				) const
				{
					std::vector<uint64_t> M(path.path.size(),std::numeric_limits<uint64_t>::max());
				
					// current point on A
					int32_t a_i = ( path.abpos / tspace ) * tspace;
					// current point on B
					int32_t b_i = ( path.bbpos );
															
					for ( size_t i = 0; i < path.path.size(); ++i )
					{
						// block end point on A
						int32_t const a_i_1 = std::min ( static_cast<int32_t>(a_i + tspace), static_cast<int32_t>(path.aepos) );
						// block end point on B
						int32_t const b_i_1 = b_i + path.path[i].second;

						if ( 
							(a_i_1 - a_i) == tspace 
							&&
							(a_i % tspace == 0)
							&&
							(a_i_1 % tspace == 0)
							&&
							path.path[i].first <= ethres
						)
						{
							M . at ( i ) = path.path[i].first;
							// assert ( a_i / tspace == i );
						}
						
						// update start points
						b_i = b_i_1;
						a_i = a_i_1;
					}
					
					uint64_t numleft = 0;
					uint64_t numright = 0;
					for ( uint64_t i = 0; i < M.size(); ++i )
						if ( M[i] != std::numeric_limits<uint64_t>::max() )
							numright++;

					for ( uint64_t i = 0; i < M.size(); ++i )
					{
						bool const below = M[i] != std::numeric_limits<uint64_t>::max();
						
						if ( below )
							numright -= 1;
						
						if ( numleft >= cthres && numright >= cthres )
							H [ i ] += 1;

						if ( below )
							numleft += 1;
					}
				}
			};

			std::ostream & operator<<(std::ostream & out, Overlap const & P);
		}
	}
}
#endif
