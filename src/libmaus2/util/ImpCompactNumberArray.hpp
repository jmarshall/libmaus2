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
#if ! defined(LIBMAUS2_UTIL_IMPCOMPACTNUMBERARRAY_HPP)
#define LIBMAUS2_UTIL_IMPCOMPACTNUMBERARRAY_HPP

#include <libmaus2/math/bitsPerNum.hpp>
#include <libmaus2/math/Log.hpp>
#include <libmaus2/bitio/CompactArray.hpp>
#include <libmaus2/util/Histogram.hpp>
#include <libmaus2/huffman/huffman.hpp>
#include <libmaus2/util/MemTempFileContainer.hpp>
#include <libmaus2/wavelet/ImpExternalWaveletGeneratorHuffman.hpp>
#include <libmaus2/wavelet/ImpHuffmanWaveletTree.hpp>
#include <libmaus2/util/iterator.hpp>

namespace libmaus2
{
	namespace util
	{
		struct ImpCompactNumberArrayGenerator;

		struct ImpCompactNumberArray
		{
			typedef ImpCompactNumberArray this_type;
			typedef libmaus2::util::unique_ptr<this_type>::type unique_ptr_type;
			
			libmaus2::wavelet::ImpHuffmanWaveletTree::unique_ptr_type IHWT;
			libmaus2::autoarray::AutoArray< ::libmaus2::bitio::CompactArray::unique_ptr_type > C;	
			
			typedef libmaus2::util::ConstIterator<this_type,uint64_t> const_iterator;
			
			friend struct ImpCompactNumberArrayGenerator;
			typedef ImpCompactNumberArrayGenerator generator_type;
			
			uint64_t byteSize() const
			{
				uint64_t s =
					IHWT->byteSize() +
					C.byteSize();
					
				for ( uint64_t i = 0; i < C.size(); ++i )
					if ( C[i] )
						s += C[i]->byteSize();
					
				return s;
			}
			
			private:
			ImpCompactNumberArray() {}
			
			public:
			template<typename stream_type>
			void serialise(stream_type & out) const
			{
				IHWT->serialise(out);
				
				if ( IHWT->getN() )
					for ( int s = IHWT->enctable.minsym; s <= IHWT->enctable.maxsym; ++s )
						if ( s > 1 && IHWT->rank(s,IHWT->getN()-1) )
						{
							assert ( C[s].get() );
							C[s]->serialize(out);
						}
				
				out.flush();
			}
			
			template<typename stream_type>
			void deserialise(stream_type & in)
			{
				libmaus2::wavelet::ImpHuffmanWaveletTree::unique_ptr_type tIHWT(new libmaus2::wavelet::ImpHuffmanWaveletTree(in));
				IHWT = UNIQUE_PTR_MOVE(tIHWT);
				C = libmaus2::autoarray::AutoArray< ::libmaus2::bitio::CompactArray::unique_ptr_type >(IHWT->enctable.maxsym+1);

				if ( IHWT->getN() )
					for ( int s = IHWT->enctable.minsym; s <= IHWT->enctable.maxsym; ++s )
						if ( s > 1 && IHWT->rank(s,IHWT->getN()-1) )
						{
							::libmaus2::bitio::CompactArray::unique_ptr_type tCs(
                                                                        new ::libmaus2::bitio::CompactArray(in));
							C[s] = UNIQUE_PTR_MOVE(tCs);
						}
			}
			
			template<typename stream_type>
			static unique_ptr_type load(stream_type & in)
			{
				unique_ptr_type P(new this_type);
				P->deserialise(in);
				return UNIQUE_PTR_MOVE(P);
			}
			
			static unique_ptr_type loadFile(std::string const & filename)
			{
				libmaus2::aio::CheckedInputStream CIS(filename);
				unique_ptr_type ptr(load(CIS));
				return UNIQUE_PTR_MOVE(ptr);
			}
			
			uint64_t operator[](uint64_t const i) const
			{
				return get(i);
			}
			
			uint64_t size() const
			{
				return IHWT->n;
			}
			
			uint64_t get(uint64_t const i) const
			{
				std::pair<int64_t,uint64_t> const P = IHWT->inverseSelect(i);
				
				if ( P.first < 2 )
					return P.first;
				else
						return (1ull << (P.first-1)) | C[P.first]->get(P.second);			
			}
			
			const_iterator begin() const
			{
				return const_iterator(this,0);
			}
			const_iterator end() const
			{
				return const_iterator(this,IHWT->n);
			}
		};

		struct ImpCompactNumberArrayGenerator
		{
			typedef ImpCompactNumberArrayGenerator this_type;
			typedef libmaus2::util::unique_ptr<this_type>::type unique_ptr_type;
			
			private:
			std::map<int64_t,uint64_t> const probs;
			libmaus2::huffman::HuffmanTreeNode::shared_ptr_type HN;
			libmaus2::huffman::EncodeTable<1> E;
			libmaus2::util::MemTempFileContainer MTFC;
			libmaus2::wavelet::ImpExternalWaveletGeneratorHuffman IEWGH;
			ImpCompactNumberArray::unique_ptr_type ICNA;
			::libmaus2::autoarray::AutoArray<uint64_t> D;

			public:
			ImpCompactNumberArrayGenerator(libmaus2::util::Histogram & hist)
			: probs(hist.getByType<int64_t>()), HN(libmaus2::huffman::HuffmanBase::createTree(probs)), 
			  E(HN.get()), IEWGH(HN.get(), MTFC), ICNA(new ImpCompactNumberArray), D(E.maxsym+1)
			{
				ICNA->C = libmaus2::autoarray::AutoArray< ::libmaus2::bitio::CompactArray::unique_ptr_type >(E.maxsym+1);

				for ( int i = E.minsym; i <= E.maxsym; ++i )
					if ( i > 1 && E.checkSymbol(i) )
					{
						uint64_t const numsyms = probs.find(i)->second;
						::libmaus2::bitio::CompactArray::unique_ptr_type ICNACi(
                                                                new ::libmaus2::bitio::CompactArray(numsyms,i-1)
                                                        );
						ICNA->C[i] = UNIQUE_PTR_MOVE(ICNACi);
					}
			}
			
			void add(uint64_t const v)
			{
				unsigned int const b = libmaus2::math::bitsPerNum(v);

				IEWGH.putSymbol(b);
				if ( b > 1 )
				{
					uint64_t const m = v & libmaus2::math::lowbits(b-1);
					ICNA->C[b]->set(D[b]++,m);
				}	
			}
			
			ImpCompactNumberArray::unique_ptr_type createFinal()
			{
				std::ostringstream hwtostr;
				IEWGH.createFinalStream(hwtostr);
				
				std::istringstream hwtistr(hwtostr.str());
				libmaus2::wavelet::ImpHuffmanWaveletTree::unique_ptr_type tICNAIHWT(
                                                new libmaus2::wavelet::ImpHuffmanWaveletTree(hwtistr)
                                        );
				ICNA->IHWT = UNIQUE_PTR_MOVE(tICNAIHWT);
				
				return UNIQUE_PTR_MOVE(ICNA);		
			}
			
			template<typename iterator_in>
			ImpCompactNumberArray::unique_ptr_type construct(iterator_in it_in, uint64_t const n)
			{
				for ( uint64_t i = 0; i < n; ++i )
					add(*(it_in++));
				ImpCompactNumberArray::unique_ptr_type ptr(createFinal());
				
				return UNIQUE_PTR_MOVE(ptr);
			}

			template<typename iterator_in>
			static ImpCompactNumberArray::unique_ptr_type constructFromArray(iterator_in it_in, uint64_t const n)
			{
				libmaus2::util::Histogram hist;
				for ( iterator_in it_a = it_in; it_a != it_in+n; ++it_a )
					hist ( libmaus2::math::bitsPerNum(*it_a) );

				ImpCompactNumberArrayGenerator gen(hist);

				ImpCompactNumberArray::unique_ptr_type ptr(gen.construct(it_in,n));
				
				return UNIQUE_PTR_MOVE(ptr);
			}
		};
	}
}
#endif
