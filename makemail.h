/*
     ATP QWK MAIL READER FOR READING AND REPLYING TO QWK MAIL PACKETS.
     Copyright (C) 1992, 1993, 1997  Thomas McWilliams 
     Copyright (C) 1990  Rene Cougnenc
   
     This program is free software; you can redistribute it and/or modify
     it under the terms of the GNU General Public License as published by
     the Free Software Foundation; either version 2, or (at your option)
     any later version.
     
     This program is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     GNU General Public License for more details.
     
     You should have received a copy of the GNU General Public License
     along with this program; if not, write to the Free Software
     Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/*
makemail.h
*/

#ifndef _ATP_MAKEMAIL_H
#define _ATP_MAKEMAIL_H 1

#include "atptypes.h"

/* line length for exported messages */
#define REPLY_LINE_LEN 80

extern const unsigned char codepc[] ;
extern const unsigned char codelu[] ;
extern const unsigned char code7bit[] ;
extern const unsigned char codevt[] ; 

#endif

