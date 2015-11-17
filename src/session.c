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
#include <errno.h>
#include <string.h>
#include "librmilter.h"
#include "librmilter_internal.h"
#include "session.h"

static void
rmilter_session_state_machine (struct rmilter_session *s, gssize rlen)
{
	guchar *p = s->cmd_buf->data, *end;
	gssize to_copy;

	end = p + rlen;

	while (p < end) {
		switch (s->state) {
		case st_read_cmd:
			s->cmd.cmd = *p;
			s->state = st_len_1;
			s->cmd.cmdlen = 0;
			g_byte_array_set_size (s->cmd.data, 0);
			p ++;
			break;
		case st_len_1:
			/* The first length byte in big endian order */
			s->cmd.cmdlen |= *p << 24;
			s->state = st_len_2;
			p++;
			break;
		case st_len_2:
			/* The second length byte in big endian order */
			s->cmd.cmdlen |= *p << 16;
			s->state = st_len_3;
			p++;
			break;
		case st_len_3:
			/* The third length byte in big endian order */
			s->cmd.cmdlen |= *p << 8;
			s->state = st_len_4;
			p++;
			break;
		case st_len_4:
			/* The fourth length byte in big endian order */
			s->cmd.cmdlen |= *p;
			s->state = st_read_data;
			p++;
			break;
		case st_read_data:
			to_copy = MIN (s->cmd.cmdlen, end - p);

			if (to_copy > 0) {
				g_byte_array_append (s->cmd.data, p, to_copy);
				p += to_copy;
			}

			/* Check if we have read the complete command */
			if (s->cmd.cmdlen == s->cmd.data->len) {
				g_byte_array_set_size (s->cmd.data, 0);
				/* Read the next command */
				s->state = st_read_cmd;
			}
			break;
		default:
			break;
		}
	}
}

void
rmilter_session_want_read (struct rmilter_session *s)
{
	gssize r;

	r = read (s->fd, s->cmd_buf, s->cmd_buf->len);

	if (r == -1) {
		if (errno == EINTR) {
			rmilter_session_want_read (s);
		}
		else {
			s->m->cb->abort (s, s->ud);
			msg_err_session ("cannot read data from server: %s",
					strerror (errno));
			rmilter_session_close (s);
		}
	}
	else if (r == 0) {
		/* This means that server has nothing to pass or end-of-session */
		msg_debug_session ("read 0 bytes from the server");
		rmilter_session_close (s);
	}
	else {
		rmilter_session_state_machine (s, r);
	}
}

void
rmilter_session_want_write (struct rmilter_session *s)
{

}

void
rmilter_session_close (struct rmilter_session *s)
{
	msg_debug_session ("closing session: %p", s);

	if (s->m->cb->close) {
		s->m->cb->close (s, s->ud);
	}

	s->m->async->del_read (s->read_ev, s->m->async->data);
	s->m->async->del_timer (s->timeout_ev, s->m->async->data);

	/* Release reference that is handled by milter itself */
	REF_RELEASE (s);
}

void
rmilter_session_start (struct rmilter_session *s)
{
	/* cmd is the initial state */
	s->state = st_read_cmd;
	/* Create read and timeout events */
	s->read_ev = s->m->async->add_read (s->m->async->data, s->fd, s);
	s->timeout_ev = s->m->async->add_timer (s->m->async->data,
			s->m->io_timeout, s);
}
