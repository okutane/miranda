/*
 * libyahoo2: yahoo_httplib.h
 *
 * Copyright (C) 2002-2004, Philip S Tellis <philip.tellis AT gmx.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef YAHOO_HTTPLIB_H
#define YAHOO_HTTPLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "yahoo2_types.h"

char *yahoo_urlencode(const char *instr);
char *yahoo_urldecode(const char *instr);
char *yahoo_xmldecode(const char *instr);

int yahoo_tcp_readline(char *ptr, int maxlen, int fd);
void yahoo_http_post(int id, const char *url, const char *cookies, long size,
		yahoo_get_fd_callback callback, void *data);
void yahoo_http_get(int id, const char *url, const char *cookies,
		yahoo_get_fd_callback callback, void *data);
void yahoo_get_url_fd(int id, const char *url, const struct yahoo_data *yd,
		yahoo_get_url_handle_callback callback, void *data);

int url_to_host_port_path(const char *url, char *host, int *port, char *path);

#ifdef __cplusplus
}
#endif

#endif
