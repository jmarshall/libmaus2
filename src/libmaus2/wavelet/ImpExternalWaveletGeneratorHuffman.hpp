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

#if ! defined(IMPEXTERNALWAVELETGENERATORHUFFMAN_HPP)
#define IMPEXTERNALWAVELETGENERATORHUFFMAN_HPP

#include <libmaus2/util/TempFileNameGenerator.hpp>
#include <libmaus2/bitio/BufferIterator.hpp>
#include <libmaus2/bitio/BitVectorConcat.hpp>
#include <libmaus2/util/NumberSerialisation.hpp>
#include <libmaus2/rank/ERankBase.hpp>
#include <libmaus2/huffman/HuffmanTreeNode.hpp>
#include <libmaus2/huffman/HuffmanTreeInnerNode.hpp>
#include <libmaus2/huffman/HuffmanTreeLeaf.hpp>
#include <libmaus2/rank/ImpCacheLineRank.hpp>
#include <libmaus2/util/TempFileContainer.hpp>

#include <libmaus2/util/unordered_map.hpp>

#include <libmaus2/aio/OutputStreamFactoryContainer.hpp>

namespace libmaus2
{
	namespace wavelet
	{
		struct ImpExternalWaveletGeneratorHuffman
		{
			typedef ImpExternalWaveletGeneratorHuffman this_type;
			typedef ::libmaus2::util::unique_ptr<this_type>::type unique_ptr_type;
			
			// rank type
			typedef ::libmaus2::rank::ImpCacheLineRank rank_type;
			// bit output stream contexts
			typedef rank_type::WriteContextExternal context_type;
			// pointer to output context
			typedef context_type::unique_ptr_type context_ptr_type;
			// vector of contexts
			typedef ::libmaus2::autoarray::AutoArray<context_ptr_type> context_vector_type;
			// pair of context pointer and bit
			typedef std::pair < context_type * , bool > bit_type;
			// vector of bit type
			typedef ::libmaus2::autoarray::AutoArray < bit_type > bit_vector_type;
			// vector of bit vector type
			typedef ::libmaus2::autoarray::AutoArray < bit_vector_type > bit_vectors_type;

			private:
			static uint64_t const bufsize = 64*1024;
		
			::libmaus2::huffman::HuffmanTreeNode const * root;
			::libmaus2::util::TempFileContainer & tmpcnt;
			// ::libmaus2::util::TempFileNameGenerator & tmpgen;
			
			// std::vector < std::string > outputfilenames;
			
			::libmaus2::autoarray::AutoArray<context_ptr_type> contexts;
			bit_vectors_type bv;
			std::map<int64_t,uint64_t> leafToId;

			uint64_t symbols;

			void flush()
			{
				for ( uint64_t i = 0; i < contexts.size(); ++i )
				{
					contexts[i]->writeBit(0);
					contexts[i]->flush();
					// std::cerr << "Flushed context " << i << std::endl;
				}
			}
			
			public:
			ImpExternalWaveletGeneratorHuffman(
				::libmaus2::huffman::HuffmanTreeNode const * rroot, 
				::libmaus2::util::TempFileContainer & rtmpcnt
			)
			: root(rroot), tmpcnt(rtmpcnt), symbols(0)
			{
				std::map < ::libmaus2::huffman::HuffmanTreeNode const * , ::libmaus2::huffman::HuffmanTreeInnerNode const * > parentMap;
				std::map < ::libmaus2::huffman::HuffmanTreeInnerNode const *, uint64_t > nodeToId;
				std::map < int64_t, ::libmaus2::huffman::HuffmanTreeLeaf const * > leafMap;
				
				std::stack < ::libmaus2::huffman::HuffmanTreeNode const * > S;
				S.push(root);
				
				while ( ! S.empty() )
				{
					::libmaus2::huffman::HuffmanTreeNode const * cur = S.top();
					S.pop();

					if ( cur->isLeaf() )
					{
						::libmaus2::huffman::HuffmanTreeLeaf const * leaf = dynamic_cast< ::libmaus2::huffman::HuffmanTreeLeaf const *>(cur);
						leafMap [ leaf->symbol ] = leaf;
					}
					else
					{
						::libmaus2::huffman::HuffmanTreeInnerNode const * node = dynamic_cast< ::libmaus2::huffman::HuffmanTreeInnerNode const *>(cur);
						uint64_t const id = nodeToId.size();

						// assert ( id == outputfilenames.size() );
						nodeToId [ node ] = id;
						// outputfilenames.push_back(tmpgen.getFileName());
						
						parentMap [ node->left ] = node;
						parentMap [ node->right ] = node;

						// push children
						S.push ( node->right );
						S.push ( node->left );
					}
				}

				// set up bit writer contexts
				contexts = ::libmaus2::autoarray::AutoArray<context_ptr_type>(nodeToId.size());
				for ( uint64_t i = 0; i < contexts.size(); ++i )
				{
					context_ptr_type tcontextsi(new context_type(
                                                tmpcnt.openOutputTempFile(i), 0, false /* no header */));
					contexts[i] = UNIQUE_PTR_MOVE(tcontextsi);
				}
					
				bv = ::libmaus2::autoarray::AutoArray < bit_vector_type >(leafMap.size());
				uint64_t lid = 0;
				for ( std::map < int64_t, ::libmaus2::huffman::HuffmanTreeLeaf const * >::const_iterator ita = leafMap.begin();
					ita != leafMap.end(); ++ita, ++lid )
				{
					uint64_t d = 0;
					::libmaus2::huffman::HuffmanTreeLeaf const * const leaf = ita->second;
					::libmaus2::huffman::HuffmanTreeNode const * cur = leaf;
					
					// compute code length
					while ( parentMap.find(cur) != parentMap.end() )
					{
						cur = parentMap.find(cur)->second;
						++d;
					}
					
					bv [ lid ] = bit_vector_type(d);
					leafToId[leaf->symbol] = lid;
					
					cur = leaf;
					d = 0;
					
					// store code vector and adjoint contexts
					while ( parentMap.find(cur) != parentMap.end() )
					{
						::libmaus2::huffman::HuffmanTreeInnerNode const * parent = parentMap.find(cur)->second;
						uint64_t const parentid = nodeToId.find(parent)->second;

						if ( cur == parent->left )
							bv [lid][d] = bit_type(contexts[parentid].get(),false);
						else
							bv [lid][d] = bit_type(contexts[parentid].get(),true);						

						cur = parent;
						++d;
					}
				}
			}
			
			void putSymbol(int64_t const s)
			{
				assert ( leafToId.find(s) != leafToId.end() );
				bit_vector_type const & b = bv [ leafToId.find(s)->second ];
				
				
				for ( uint64_t i = 0; i < b.size(); ++i )
					b [ i ] . first -> writeBit( b[i].second );

				symbols += 1;

				// std::cerr << "Putting symbol " << s << " code length " << b.size() << " symbols now " << symbols << std::endl;
			}

			template<typename stream_type>
			uint64_t createFinalStream(stream_type & out)
			{			
				flush();

				uint64_t p = 0;
				p += ::libmaus2::util::NumberSerialisation::serialiseNumber(out,symbols); // n
				p += root->serialize(out); // huffman code tree
				p += ::libmaus2::util::NumberSerialisation::serialiseNumber(out,contexts.size()); // number of bit vectors
				
				std::vector<uint64_t> nodeposvec;

				for ( uint64_t i = 0; i < contexts.size(); ++i )
				{
					nodeposvec.push_back(p);
				
					uint64_t const blockswritten = contexts[i]->blockswritten;
					uint64_t const datawordswritten = 6*blockswritten;
					uint64_t const allwordswritten = 8*blockswritten;
						
					contexts[i].reset();
					tmpcnt.closeOutputTempFile(i);	
					
					// bits written
					p += ::libmaus2::serialize::Serialize<uint64_t>::serialize(out,64*datawordswritten);
					// auto array header (words written)
					p += ::libmaus2::serialize::Serialize<uint64_t>::serialize(out,allwordswritten);
					//std::string const filename = outputfilenames[i];
					std::istream & istr = tmpcnt.openInputTempFile(i);
					// std::ifstream istr(filename.c_str(),std::ios::binary);
					// std::cerr << "Copying " << allwordswritten << " from stream " << filename << std::endl;
					::libmaus2::util::GetFileSize::copy (istr, out, allwordswritten, sizeof(uint64_t));
					p += allwordswritten * sizeof(uint64_t);
					tmpcnt.closeInputTempFile(i);

					// remove(filename.c_str());
				}
				
				uint64_t const indexpos = p;
				p += ::libmaus2::util::NumberSerialisation::serialiseNumberVector(out,nodeposvec);
				p += ::libmaus2::util::NumberSerialisation::serialiseNumber(out,indexpos);
					
				out.flush();
				
				return p;
			}
			
			void createFinalStream(std::string const & filename)
			{
				libmaus2::aio::OutputStream::unique_ptr_type Postr(libmaus2::aio::OutputStreamFactoryContainer::constructUnique(filename));
				std::ostream & ostr = *Postr;
				createFinalStream(ostr);
				ostr.flush();
				Postr.reset();
			}
		};
	}
}
#endif
