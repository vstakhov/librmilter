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

#ifndef LIBRDNS_LIBRMILTER_EVENT_H
#define LIBRDNS_LIBRMILTER_EVENT_H


#include <event.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "librmilter.h"

#ifdef  __cplusplus
extern "C" {
#endif

static void *rmilter_libevent_add_read (void *priv_data,
		int fd,
		void *user_data);
static void rmilter_libevent_del_read (void *priv_data, void *ev_data);
static void *rmilter_libevent_add_write (void *priv_data,
		int fd,
		void *user_data);
static void rmilter_libevent_del_write (void *priv_data, void *ev_data);
static void *rmilter_libevent_add_timer (void *priv_data,
		double after,
		void *user_data);
static void *rmilter_libevent_add_periodic (void *priv_data, double after,
		rmilter_periodic_callback cb, void *user_data);
static void rmilter_libevent_del_periodic (void *priv_data, void *ev_data);
static void rmilter_libevent_repeat_timer (void *priv_data, void *ev_data);
static void rmilter_libevent_del_timer (void *priv_data, void *ev_data);

struct rmilter_event_periodic_cbdata {
	struct event *ev;
	rmilter_periodic_callback cb;
	void *cbdata;
};

static struct rmilter_async_context *
rmilter_gen_libevent (struct rmilter_resolver *resolver,
		struct event_base *ev_base)
{
	static const struct rmilter_async_context ev_ctx = {
			.add_read = rmilter_libevent_add_read,
			.del_read = rmilter_libevent_del_read,
			.add_write = rmilter_libevent_add_write,
			.del_write = rmilter_libevent_del_write,
			.add_timer = rmilter_libevent_add_timer,
			.add_periodic = rmilter_libevent_add_periodic,
			.del_periodic = rmilter_libevent_del_periodic,
			.repeat_timer = rmilter_libevent_repeat_timer,
			.del_timer = rmilter_libevent_del_timer,
			.cleanup = NULL
	};
	struct rmilter_async_context *nctx;

	nctx = g_slice_alloc (sizeof (struct rmilter_async_context));
	if (nctx != NULL) {
		memcpy (nctx, &ev_ctx, sizeof (struct rmilter_async_context));
		nctx->data = ev_base;
	}

	return nctx;
}

static void
rmilter_libevent_read_event (int fd, short what, void *ud)
{
	rmilter_process_read (fd, ud);
}

static void
rmilter_libevent_write_event (int fd, short what, void *ud)
{
	rmilter_process_write (fd, ud);
}

static void
rmilter_libevent_timer_event (int fd, short what, void *ud)
{
	rmilter_process_timer (ud);
}

static void
rmilter_libevent_periodic_event (int fd, short what, void *ud)
{
	struct rmilter_event_periodic_cbdata *cbdata = ud;
	cbdata->cb (cbdata->cbdata);
}

static void *
rmilter_libevent_add_read (void *priv_data, int fd, void *user_data)
{
	struct event *ev;
	ev = g_slice_alloc (sizeof (struct event));
	if (ev != NULL) {
		event_set (ev,
				fd,
				EV_READ | EV_PERSIST,
				rmilter_libevent_read_event,
				user_data);
		event_base_set (priv_data, ev);
		event_add (ev, NULL);
	}
	return ev;
}

static void
rmilter_libevent_del_read (void *priv_data, void *ev_data)
{
	struct event *ev = ev_data;
	if (ev != NULL) {
		event_del (ev);
		g_slice_free1 (sizeof (*ev), ev);
	}
}
static void *
rmilter_libevent_add_write (void *priv_data, int fd, void *user_data)
{
	struct event *ev;
	ev = g_slice_alloc (sizeof (struct event));
	if (ev != NULL) {
		event_set (ev, fd, EV_WRITE | EV_PERSIST,
				rmilter_libevent_write_event, user_data);
		event_base_set (priv_data, ev);
		event_add (ev, NULL);
	}
	return ev;
}

static void
rmilter_libevent_del_write (void *priv_data, void *ev_data)
{
	struct event *ev = ev_data;
	if (ev != NULL) {
		event_del (ev);
		g_slice_free1 (sizeof (*ev), ev);
	}
}

#define rmilter_event_double_to_tv(dbl, tv) do {                            \
    (tv)->tv_sec = (int)(dbl);                                              \
    (tv)->tv_usec = ((dbl) - (int)(dbl))*1000*1000;                         \
} while(0)

static void *
rmilter_libevent_add_timer (void *priv_data, double after, void *user_data)
{
	struct event *ev;
	struct timeval tv;
	ev = g_slice_alloc (sizeof (struct event));
	if (ev != NULL) {
		rmilter_event_double_to_tv (after, &tv);
		event_set (ev,
				-1,
				EV_TIMEOUT | EV_PERSIST,
				rmilter_libevent_timer_event,
				user_data);
		event_base_set (priv_data, ev);
		event_add (ev, &tv);
	}
	return ev;
}

static void *
rmilter_libevent_add_periodic (void *priv_data, double after,
		rmilter_periodic_callback cb, void *user_data)
{
	struct event *ev;
	struct timeval tv;
	struct rmilter_event_periodic_cbdata *cbdata = NULL;

	ev = g_slice_alloc (sizeof (struct event));
	if (ev != NULL) {
		cbdata = g_slice_alloc (sizeof (struct rmilter_event_periodic_cbdata));
		if (cbdata != NULL) {
			rmilter_event_double_to_tv (after, &tv);
			cbdata->cb = cb;
			cbdata->cbdata = user_data;
			cbdata->ev = ev;
			event_set (ev,
					-1,
					EV_TIMEOUT | EV_PERSIST,
					rmilter_libevent_periodic_event,
					cbdata);
			event_base_set (priv_data, ev);
			event_add (ev, &tv);
		}
		else {
			g_slice_free1 (sizeof (*ev), ev);
			return NULL;
		}
	}
	return cbdata;
}

static void
rmilter_libevent_del_periodic (void *priv_data, void *ev_data)
{
	struct rmilter_event_periodic_cbdata *cbdata = ev_data;
	if (cbdata != NULL) {
		event_del (cbdata->ev);
		g_slice_free1 (sizeof (*cbdata->ev), (void *) cbdata->ev);
		g_slice_free1 (sizeof (*cbdata), (void *) cbdata);
	}
}

static void
rmilter_libevent_repeat_timer (void *priv_data, void *ev_data)
{
	/* XXX: libevent hides timeval, so timeouts are persistent here */
}

#undef rmilter_event_double_to_tv

static void
rmilter_libevent_del_timer (void *priv_data, void *ev_data)
{
	struct event *ev = ev_data;
	if (ev != NULL) {
		event_del (ev);
		g_slice_free1 (sizeof (*ev), ev);
	}
}

#ifdef  __cplusplus
}
#endif

#endif
