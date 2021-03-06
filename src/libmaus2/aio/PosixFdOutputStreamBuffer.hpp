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
#if ! defined(LIBMAUS2_AIO_POSIXFDOUTPUTSTREAMBUFFER_HPP)
#define LIBMAUS2_AIO_POSIXFDOUTPUTSTREAMBUFFER_HPP

#include <ostream>
#include <libmaus2/autoarray/AutoArray.hpp>
#include <libmaus2/aio/PosixFdInput.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace libmaus2
{
	namespace aio
	{
		struct PosixFdOutputStreamBuffer : public ::std::streambuf
		{
			private:
			static uint64_t getDefaultBlockSize()
			{
				return 64*1024;
			}

			static int64_t getOptimalIOBlockSize(int const fd, std::string const & fn)
			{
				int64_t const fsopt = libmaus2::aio::PosixFdInput::getOptimalIOBlockSize(fd,fn);
				
				if ( fsopt <= 0 )
					return getDefaultBlockSize();
				else
					return fsopt;
			}
			
			int fd;
			bool closefd;
			int64_t const optblocksize;
			uint64_t const buffersize;
			::libmaus2::autoarray::AutoArray<char> buffer;
			uint64_t writepos;

			void doSeekAbsolute(uint64_t const p)
			{
				while ( ::lseek(fd,p,SEEK_SET) == static_cast<off_t>(-1) )
				{
					int const error = errno;
					
					switch ( error )
					{
						case EINTR:
						case EAGAIN:
							break;
						default:
						{
							libmaus2::exception::LibMausException se;
							se.getStream() << "PosixOutputStreamBuffer::doSeekkAbsolute(): lseek() failed: " << strerror(error) << std::endl;
							se.finish();
							throw se;
						}
					}					
				}
			}

			void doClose()
			{
				while ( close(fd) < 0 )
				{
					int const error = errno;
					
					switch ( error )
					{
						case EINTR:
						case EAGAIN:
							break;
						default:
						{
							libmaus2::exception::LibMausException se;
							se.getStream() << "PosixOutputStreamBuffer::doClose(): close() failed: " << strerror(error) << std::endl;
							se.finish();
							throw se;
						}
					}					
				}
			}

			int doOpen(std::string const & filename)
			{
				int fd = -1;
				
				while ( (fd = open(filename.c_str(),O_WRONLY | O_CREAT | O_TRUNC,0644)) < 0 )
				{
					int const error = errno;
					
					switch ( error )
					{
						case EINTR:
						case EAGAIN:
							break;
						default:
						{
							libmaus2::exception::LibMausException se;
							se.getStream() << "PosixOutputStreamBuffer::doOpen(): open("<<filename<<") failed: " << strerror(error) << std::endl;
							se.finish();
							throw se;
						}
					}					
				}
				
				return fd;
			}

			void doFlush()
			{
				while ( fsync(fd) < 0 )
				{
					int const error = errno;
					
					switch ( error )
					{
						case EINTR:
						case EAGAIN:
							break;
						case EROFS:
						case EINVAL:
							// descriptor does not support flushing
							return;
						default:
						{
							libmaus2::exception::LibMausException se;
							se.getStream() << "PosixOutputStreamBuffer::doSync(): fsync() failed: " << strerror(error) << std::endl;
							se.finish();
							throw se;
						}
					}					
				}
			}
			
			void doSync()
			{
				uint64_t n = pptr()-pbase();
				pbump(-n);

				char * p = pbase();

				while ( n )
				{
					ssize_t const w = ::write(fd,p,n);
					
					if ( w < 0 )
					{
						int const error = errno;
						
						switch ( error )
						{
							case EINTR:
							case EAGAIN:
								break;
							default:
							{
								libmaus2::exception::LibMausException se;
								se.getStream() << "PosixOutputStreamBuffer::doSync(): write() failed: " << strerror(error) << std::endl;
								se.finish();
								throw se;
							}
						}
					}
					else
					{
						assert ( w <= static_cast<int64_t>(n) );
						n -= w;
						writepos += w;
					}
				}
				
				assert ( ! n );
			}
			

			public:
			PosixFdOutputStreamBuffer(int const rfd, int64_t const rbuffersize)
			: fd(rfd), 
			  closefd(false), 
			  optblocksize((rbuffersize < 0) ? getOptimalIOBlockSize(fd,std::string()) : rbuffersize),
			  buffersize(optblocksize), 
			  buffer(buffersize,false),
			  writepos(0)
			{
				setp(buffer.begin(),buffer.end()-1);
			}

			PosixFdOutputStreamBuffer(std::string const & fn, int64_t const rbuffersize)
			: 
			  fd(doOpen(fn)), 
			  closefd(true), 
			  optblocksize((rbuffersize < 0) ? getOptimalIOBlockSize(fd,fn) : rbuffersize),
			  buffersize(optblocksize), 
			  buffer(buffersize,false),
			  writepos(0)
			{
				setp(buffer.begin(),buffer.end()-1);
			}

			~PosixFdOutputStreamBuffer()
			{
				sync();
				if ( closefd )
					doClose();
			}
			
			int_type overflow(int_type c = traits_type::eof())
			{
				if ( c != traits_type::eof() )
				{
					*pptr() = c;
					pbump(1);
					doSync();
				}

				return c;
			}
			
			int sync()
			{
				doSync();
				doFlush();
				return 0; // no error, -1 for error
			}			

                        /**
                         * seek to absolute position
                         **/
                        ::std::streampos seekpos(::std::streampos sp, ::std::ios_base::openmode which = ::std::ios_base::in | ::std::ios_base::out)
                        {
                        	if ( (which & ::std::ios_base::out) )
                        	{
                        		doSync();
                        		doSeekAbsolute(sp);
                        		writepos = sp;
                        		return sp;
				}
				else
				{
					return -1;
				}
                        }

			/**
			 * relative seek
			 **/
			::std::streampos seekoff(::std::streamoff off, ::std::ios_base::seekdir way, ::std::ios_base::openmode which)
			{
				// seek relative to current position
				if ( (way == ::std::ios_base::cur) && (which == std::ios_base::out) && (off == 0) )
					return writepos + (pptr()-pbase());
				else if ( (way == ::std::ios_base::beg) && (which == std::ios_base::out) )
					return seekpos(off,which);
				else
					return -1;
			}

		};
	}
}
#endif
