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
#include <libmaus2/bambam/BamFlagBase.hpp>
		
std::ostream & operator<<(std::ostream & out, libmaus2::bambam::BamFlagBase::bam_flags const f)
{
	switch ( f )
	{
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FPAIRED: out << "LIBMAUS2_BAMBAM_FPAIRED"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FPROPER_PAIR: out << "LIBMAUS2_BAMBAM_FPROPER_PAIR"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FUNMAP: out << "LIBMAUS2_BAMBAM_FUNMAP"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FMUNMAP: out << "LIBMAUS2_BAMBAM_FMUNMAP"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FREVERSE: out << "LIBMAUS2_BAMBAM_FREVERSE"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FMREVERSE: out << "LIBMAUS2_BAMBAM_FMREVERSE"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FREAD1: out << "LIBMAUS2_BAMBAM_FREAD1"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FREAD2: out << "LIBMAUS2_BAMBAM_FREAD2"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FSECONDARY: out << "LIBMAUS2_BAMBAM_FSECONDARY"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FQCFAIL: out << "LIBMAUS2_BAMBAM_FQCFAIL"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FDUP: out << "LIBMAUS2_BAMBAM_FDUP"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FSUPPLEMENTARY: out << "LIBMAUS2_BAMBAM_FSUPPLEMENTARY"; break;
		case libmaus2::bambam::BamFlagBase::LIBMAUS2_BAMBAM_FCIGAR32: out << "LIBMAUS2_BAMBAM_FCIGAR32"; break;
	}
	
	return out;
}
