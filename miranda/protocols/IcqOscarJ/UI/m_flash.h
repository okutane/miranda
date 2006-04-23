/*
Miranda FlashAvatars Plugin
Plugin support header file
Copyright (C) 2006 Big Muscle

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

// Service functions

/**
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM not used
 */
#define MS_FAVATAR_DESTROY 		"FlashAvatar/Destroy"

/**
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM not used
 */
#define MS_FAVATAR_MAKE 		"FlashAvatar/Make"

/**
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM LPRECT
 */
#define MS_FAVATAR_RESIZE 		"FlashAvatar/Resize"

/**
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM not used
 */
#define MS_FAVATAR_GETINFO 		"FlashAvatar/GetInfo"

/**
	WPARAM FLASHAVATAR* (hContact, hParentWindow)
	LPARAM BSTR
 */
#define MS_FAVATAR_SETEMOFACE 	"FlashAvatar/SetEmoFace"

// Avatar emotion faces
#define AV_SMILE	"smile"
#define AV_SAD		"sad"
#define AV_LAUGH	"laugh"
#define AV_MAD		"mad"
#define AV_CRY		"cry"
#define AV_OFFLINE	"offline"
#define AV_BUSY		"busy"
#define AV_LOVE		"love"
#define AV_NORMAL	"stam"

typedef struct {
	HANDLE hContact;	// contact who flash avatar belongs to
	HWND hWindow;		// handle of flash avatar object
	HWND hParentWindow;	// handle of flash avatar's parent object
	char* cUrl;			// url of .swf file
	int id;				// unique number of plugin which wants to use avatar service
} FLASHAVATAR;
