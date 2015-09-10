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

#ifndef LIBRDNS_LIBMILTER_EV_H
#define LIBRDNS_LIBMILTER_EV_H

#include <ev.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include "librmilter.h"

#ifdef  __cplusplus
extern "C" {
#endif

static void *rmilter_libev_add_read (void *priv_data, int fd, void *user_data);
static void rmilter_libev_del_read (void *priv_data, void *ev_data);
static void *rmilter_libev_add_write (void *priv_data, int fd, void *user_data);
static void rmilter_libev_del_write (void *priv_data, void *ev_data);
static void *rmilter_libev_add_timer (void *priv_data,
		double after,
		void *user_data);
static void *rmilter_libev_add_periodic (void *priv_data, double after,
		rmilter_periodic_callback cb, void *user_data);
static void rmilter_libev_del_periodic (void *priv_data, void *ev_data);
static void rmilter_libev_repeat_timer (void *priv_data, void *ev_data);
static void rmilter_libev_del_timer (void *priv_data, void *ev_data);
static void rmilter_libev_stop_event (void *priv_data, void *ev_data);
static void rmilter_libev_start_event (void *priv_data, void *ev_data);

struct rmilter_ev_periodic_cbdata {
	ev_timer *ev;
	rmilter_periodic_callback cb;
	void *cbdata;
};

static struct rmilter_async_context*
rmilter_gen_libev (struct rmilter_resolver *resolver, struct ev_loop *loop)
{
	static const struct rmilter_async_context ev_ctx = {
			.data = NULL,
			.add_read = rmilter_libev_add_read,
			.del_read = rmilter_libev_del_read,
			.add_write = rmilter_libev_add_write,
			.del_write = rmilter_libev_del_write,
			.add_timer = rmilter_libev_add_timer,
			.repeat_timer = rmilter_libev_repeat_timer,
			.del_timer = rmilter_libev_del_timer,
			.add_periodic = rmilter_libev_add_periodic,
			.del_periodic = rmilter_libev_del_periodic,
			.cleanup = NULL,
			.stop_event = rmilter_libev_stop_event,
			.start_event = rmilter_libev_start_event
	};
	struct rmilter_async_context *nctx;
	void *ptr;
	
	ptr = g_slice_alloc (sizeof (struct rmilter_async_context));
	if (ptr != NULL) {
		nctx = (struct rmilter_async_context *) ptr;
		memcpy (ptr, (void *) &ev_ctx, sizeof (struct rmilter_async_context));
		nctx->data = (void *) loop;
	}

	return nctx;
}

static void
rmilter_libev_read_event (struct ev_loop *loop, ev_io *ev, int revents)
{
	rmilter_process_read (ev->fd, ev->data);
}

static void
rmilter_libev_write_event (struct ev_loop *loop, ev_io *ev, int revents)
{
	rmilter_process_write (ev->fd, ev->data);
}

static void
rmilter_libev_timer_event (struct ev_loop *loop, ev_timer *ev, int revents)
{
	rmilter_process_timer (ev->data);
}

static void
rmilter_libev_periodic_event (struct ev_loop *loop, ev_timer *ev, int revents)
{
	struct rmilter_ev_periodic_cbdata *cbdata = (struct rmilter_ev_periodic_cbdata *)
			ev->data;
	cbdata->cb (cbdata->cbdata);
}

static void *
rmilter_libev_add_read (void *priv_data, int fd, void *user_data)
{
	ev_io *ev;
	void *ptr;

	ptr = g_slice_alloc (sizeof (ev_io));
	if (ptr != NULL) {
		ev = (ev_io *) ptr;
		ev_io_init (ev, rmilter_libev_read_event, fd, EV_READ);
		ev->data = user_data;
		ev_io_start ((struct ev_loop *) priv_data, ev);
	}
	return ptr;
}

static void
rmilter_libev_del_read (void *priv_data, void *ev_data)
{
	ev_io *ev = (ev_io *) ev_data;
	if (ev != NULL) {
		ev_io_stop ((struct ev_loop *) priv_data, ev);
		g_slice_free1 (sizeof (*ev), (void *) ev);
	}
}
static void *
rmilter_libev_add_write (void *priv_data, int fd, void *user_data)
{
	ev_io *ev;

	ev = (ev_io *) g_slice_alloc (sizeof (ev_io));
	if (ev != NULL) {
		ev_io_init (ev, rmilter_libev_write_event, fd, EV_WRITE);
		ev->data = user_data;
		ev_io_start ((struct ev_loop *) priv_data, ev);
	}
	return (void *) ev;
}

static void
rmilter_libev_del_write (void *priv_data, void *ev_data)
{
	ev_io *ev = (ev_io *) ev_data;
	if (ev != NULL) {
		ev_io_stop ((struct ev_loop *) priv_data, ev);
		g_slice_free1 (sizeof (*ev), (void *) ev);
	}
}

static void *
rmilter_libev_add_timer (void *priv_data, double after, void *user_data)
{
	ev_timer *ev;
	ev = (ev_timer *) g_slice_alloc (sizeof (ev_timer));
	if (ev != NULL) {
		ev_timer_init (ev, rmilter_libev_timer_event, after, after);
		ev->data = user_data;
		ev_timer_start ((struct ev_loop *) priv_data, ev);
	}
	return (void *) ev;
}

static void *
rmilter_libev_add_periodic (void *priv_data, double after,
		rmilter_periodic_callback cb, void *user_data)
{
	ev_timer *ev;
	struct rmilter_ev_periodic_cbdata *cbdata = NULL;

	ev = (ev_timer *) g_slice_alloc (sizeof (ev_timer));
	if (ev != NULL) {
		cbdata = (struct rmilter_ev_periodic_cbdata *)
				g_slice_alloc (sizeof (struct rmilter_ev_periodic_cbdata));
		if (cbdata != NULL) {
			cbdata->cb = cb;
			cbdata->cbdata = user_data;
			cbdata->ev = ev;
			ev_timer_init (ev, rmilter_libev_periodic_event, after, after);
			ev->data = cbdata;
			ev_timer_start ((struct ev_loop *) priv_data, ev);
		}
		else {
			g_slice_free1 (sizeof (*ev), (void *) ev);
			return NULL;
		}
	}
	return (void *) cbdata;
}

static void
rmilter_libev_del_periodic (void *priv_data, void *ev_data)
{
	struct rmilter_ev_periodic_cbdata *cbdata = (struct rmilter_ev_periodic_cbdata *)
			ev_data;
	if (cbdata != NULL) {
		ev_timer_stop ((struct ev_loop *) priv_data, (ev_timer *) cbdata->ev);
		g_slice_free1 (sizeof (*cbdata->ev), (void *) cbdata->ev);
		g_slice_free1 (sizeof (*cbdata), (void *) cbdata);
	}
}

static void
rmilter_libev_repeat_timer (void *priv_data, void *ev_data)
{
	ev_timer *ev = (ev_timer *) ev_data;
	if (ev != NULL) {
		ev_timer_again ((struct ev_loop *) priv_data, ev);
	}
}

static void
rmilter_libev_del_timer (void *priv_data, void *ev_data)
{
	ev_timer *ev = (ev_timer *) ev_data;
	if (ev != NULL) {
		ev_timer_stop ((struct ev_loop *) priv_data, ev);
		g_slice_free1 (sizeof (*ev), (void *) ev);
	}
}

static void
rmilter_libev_start_event (void *priv_data, void *ev_data)
{
	ev_io *ev = (ev_io *) ev_data;
	if (ev != NULL) {
		ev_io_start ((struct ev_loop *) priv_data, ev);
	}
}

static void
rmilter_libev_stop_event (void *priv_data, void *ev_data)
{
	ev_io *ev = (ev_io *) ev_data;
	if (ev != NULL) {
		ev_io_stop ((struct ev_loop *) priv_data, ev);
	}
}

#ifdef  __cplusplus
}
#endif

#endif
