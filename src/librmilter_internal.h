/*
 * Copyright (c) 2015, Vsevolod Stakhov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *	 * Redistributions of source code must retain the above copyright
 *	   notice, this list of conditions and the following disclaimer.
 *	 * Redistributions in binary form must reproduce the above copyright
 *	   notice, this list of conditions and the following disclaimer in the
 *	   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR ''AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LIBRDNS_LIBMILTER_INTERNAL_H
#define LIBRDNS_LIBMILTER_INTERNAL_H

#include "librmilter.h"
#include "ref.h"
#include "utlist.h"
#include "logger.h"

enum rmilter_session_state {
	len_1,
	len_2,
	len_3,
	len_4,
	cmd,
	read_data,
	send_reply
};

struct rmilter_command {
	char cmd;
	guint cmdlen;
};

struct rmilter_reply_element {
	char code;
	GString *data;
	struct rmilter_reply_element *next, *prev;
};

struct rmilter_session {
	struct rmilter_milter *m;
	GList *parent_link;
	const char *module;
	const char *id;
	GHashTable *macros;
	GString *cmd_buf;
	struct rmilter_reply_element *replies;
	struct rmilter_command cmd;
	gint fd;
	enum rmilter_session_state state;
	void *ud;
	void *read_ev;
	void *write_ev;
	void *timeout_ev;
	ref_entry_t ref;
};

struct rmilter_milter {
	struct rmilter_callbacks *cb;
	struct rmilter_async_context *async;
	rmilter_log_function log;
	void *log_data;
	GQueue *sessions;
	gboolean wanna_die;
	ref_entry_t ref;
};

#endif
