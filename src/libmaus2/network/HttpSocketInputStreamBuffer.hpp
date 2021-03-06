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
#if ! defined(LIBMAUS2_NETWORK_HTTPSOCKETINPUTSTREAMBUFFER_HPP)
#define LIBMAUS2_NETWORK_HTTPSOCKETINPUTSTREAMBUFFER_HPP

#include <libmaus2/network/HttpHeaderWrapper.hpp>
#include <libmaus2/network/HttpBodyWrapper.hpp>
#include <libmaus2/network/SocketInputStreamBuffer.hpp>

namespace libmaus2
{
	namespace network
	{
		struct HttpSocketInputStreamBuffer : public HttpHeaderWrapper, public HttpBodyWrapper, public SocketInputStreamBuffer
		{
			typedef HttpSocketInputStreamBuffer this_type;
			typedef libmaus2::util::unique_ptr<this_type>::type unique_ptr_type;
			typedef libmaus2::util::shared_ptr<this_type>::type shared_ptr_type;
		
			HttpSocketInputStreamBuffer(
				std::string const & url,
				uint64_t const bufsize, 
				uint64_t const pushbacksize = 0
			) 
			:
				libmaus2::network::HttpHeaderWrapper("GET",std::string(),url),
				libmaus2::network::HttpBodyWrapper(getHttpHeader().getStream(),getHttpHeader().isChunked(),getHttpHeader().getContentLength()),
				libmaus2::network::SocketInputStreamBuffer(getHttpBody(),bufsize,pushbacksize)
			{
			
			}
		};
	}
}
#endif
