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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <unistd.h>
#include "librmilter.h"
#include "librmilter_internal.h"

static void
rmilter_session_dtor (void *d)
{
	struct rmilter_session *s = d;
	struct rmilter_reply_element *rep, *tmp;
	GHashTableIter it;
	gpointer k, v;

	if (s->read_ev) {
		s->m->async->del_read (s->m->async->data, s->read_ev);
	}

	if (s->write_ev) {
		s->m->async->del_write (s->m->async->data, s->write_ev);
	}

	if (s->timeout_ev) {
		s->m->async->del_timer (s->m->async->data, s->timeout_ev);
	}

	if (s->fd != -1) {
		close (s->fd);
	}

	if (s->cmd_buf) {
		g_string_free (s->cmd_buf, TRUE);
	}

	if (s->macros) {
		g_hash_table_iter_init (&it, s->macros);

		while (g_hash_table_iter_next (&it, &k, &v)) {
			g_string_free (k, TRUE);
			g_string_free (v, TRUE);
		}

		g_hash_table_unref (s->macros);
	}

	DL_FOREACH_SAFE (s->replies, rep, tmp) {
		if (rep->data) {
			g_string_free (rep->data, TRUE);
		}

		g_slice_free1 (sizeof (*rep), rep);
	}

	g_queue_delete_link (s->m->sessions, s->parent_link);

	/* Release refcount on the parent object */
	REF_RELEASE (s->m);
	g_slice_free1 (sizeof (*s), s);
}

static void
rmilter_milter_dtor (void *d)
{
	struct rmilter_milter *m = d;

	/* At this point we assume that all sessions pending are dead */
	g_assert (m->sessions->length == 0);

	g_slice_free1 (sizeof (*m), m);
}

struct rmilter_milter *
rmilter_create (struct rmilter_callbacks *callbacks,
		struct rmilter_async_context *async,
		rmilter_log_function log,
		void *log_data)
{

	struct rmilter_milter *m;

	g_assert (callbacks != NULL);
	g_assert (async != NULL);

	m = g_slice_alloc0 (sizeof (*m));
	m->async = async;
	m->cb = callbacks;

	if (log == NULL) {
		m->log = rmilter_logger_internal;
		m->log_data = m;
	}
	else {
		m->log = log;
		m->log_data = log_data;
	}

	m->sessions = g_queue_new ();

	REF_INIT_RETAIN (m, rmilter_milter_dtor);

	return m;
}

bool
rmilter_consume_socket (struct rmilter_milter *milter, int fd,
		const char *module, const char *id, void *ud)
{
	struct rmilter_session *s;

	g_assert (milter != NULL);

	if (milter->wanna_die) {
		return false;
	}

	s = g_slice_alloc0 (sizeof (*s));
	s->m = milter;
	s->cmd_buf = g_string_sized_new (64);
	s->macros = g_hash_table_new ((GHashFunc)g_string_hash,
			(GEqualFunc)g_string_equal);
	s->state = len_1;
	s->fd = fd;
	s->id = id;
	s->module = module;

	REF_INIT_RETAIN (s, rmilter_session_dtor);
	/* Grab reference from the parent */
	REF_RETAIN (s->m);

	g_queue_push_head (milter->sessions, s);
	s->parent_link = g_queue_peek_head_link (milter->sessions);

	return true;
}

void
rmilter_destroy (struct rmilter_milter *milter)
{
	struct rmilter_session *s;
	GList *cur;

	g_assert (milter != NULL);

	/* Stop new sessions from being added */
	milter->wanna_die = TRUE;
	cur = milter->sessions->tail;

	while (cur) {
		s = cur->data;

		rmilter_session_close (s);
		cur = g_list_next (cur);
	}

	/* Release ownership to allow destruction when all sessions are dead */
	REF_RELEASE (milter);
}