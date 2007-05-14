/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci, Nightwish

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/


#ifndef __POLL_H__
# define __POLL_H__

#include <windows.h>

struct ThreadQueue
{
	SortedList *queue;
	CRITICAL_SECTION cs;
	int waitTime;
};

extern ThreadQueue requestQueue;

struct QueueItem
{
	HANDLE hContact;
	DWORD check_time;
};


void InitPolls();
void FreePolls();

// Add an contact to a queue
void QueueAdd(ThreadQueue &queue, HANDLE hContact);





#endif // __POLL_H__
