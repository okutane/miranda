/*
Plugin of Miranda IM for communicating with users of the AIM protocol.
Copyright (C) 2005-2006 Aaron Myles Landwehr

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef FLAP_H
#define FLAP_H
#include "packets.h"
#define FLAP_SIZE 6
class FLAP
{
private:
	unsigned char type_;
	unsigned short length_;
	char* value_;
public:
	FLAP(char* buf,int num_bytes);
	unsigned short len();
	unsigned short snaclen();
	int cmp(unsigned short type);
	char* val();
};
#endif
