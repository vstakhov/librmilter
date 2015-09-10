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

#ifndef LIBRMILTER_LIBRMILTER_H_H
#define LIBRMILTER_LIBRMILTER_H_H

#include <sys/types.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

#include <glib.h>

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * Internal rmilter context
 */
struct rmilter_context;

/*
 * The shared milter structure
 */
struct rmilter_milter;

/*
 * Reply codes
 */
enum rmilter_reply {
	RMILTER_REPLY_CONTINUE = 0,
	RMILTER_REPLY_REJECT = 1,
	RMILTER_REPLY_DISCARD = 2,
	RMILTER_REPLY_ACCEPT = 3,
	RMILTER_REPLY_TEMPFAIL = 4
};

/*
 * This structure is used to
 */
struct rmilter_addr {
	enum {
		RMILTER_ADDR_IP4 = 0,
		RMILTER_ADDR_IP6,
		RMILTER_ADDR_UNIX,
		RMILTER_ADDR_UNKNOWN
	} type;

	union {
		uint32_t ip4;
		uint32_t ip6[16];
		char *path;
	} addr;
};

/*
 * Milter callbacks
 */
struct rmilter_callbacks {
	/* connection info filter */
	enum librmilter_reply (*connect) (struct rmilter_context *ctx,
			void *priv,
			const char *hostname,
			struct rmilter_addr *addr);

	/* SMTP HELO command filter */
	enum librmilter_reply (*hello) (struct rmilter_context *ctx,
			void *priv, const char *helo);

	/* envelope sender filter */
	enum librmilter_reply (*envfrom) (struct rmilter_context *ctx,
			void *priv, GPtrArray *from);

	/* envelope recipient filter */
	enum librmilter_reply (*envrcpt) (struct rmilter_context *ctx,
			void *priv, GPtrArray *rcpt);

	/* header filter */
	enum librmilter_reply (*header) (struct rmilter_context *ctx,
			void *priv, const char *name, const char *value);

	/* end of header */
	enum librmilter_reply (*eoh) (struct rmilter_context *ctx,
			void *priv);

	/* body block */
	enum librmilter_reply (*body) (struct rmilter_context *ctx,
			void *priv, unsigned char *chunk, unsigned int len);

	/* end of message */
	enum librmilter_reply (*eom) (struct rmilter_context *ctx,
			void *priv);

	/* message aborted */
	enum librmilter_reply (*abort) (struct rmilter_context *ctx,
			void *priv);

	/* connection cleanup */
	enum librmilter_reply (*close) (struct rmilter_context *ctx,
			void *priv);

	/* SMTP DATA command filter */
	enum librmilter_reply (*data) (struct rmilter_context *ctx,
			void *priv);
};

/*
 * Async bindings
 */
typedef void (*rmilter_periodic_callback) (void *user_data);

struct rmilter_async_context {
	void *data;
	void* (*add_read) (void *priv_data, int fd, void *user_data);
	void (*del_read) (void *priv_data, void *ev_data);
	void* (*add_write) (void *priv_data, int fd, void *user_data);
	void (*del_write) (void *priv_data, void *ev_data);
	void* (*add_timer) (void *priv_data, double after, void *user_data);
	void (*repeat_timer) (void *priv_data, void *ev_data);
	void (*del_timer) (void *priv_data, void *ev_data);
	void* (*add_periodic) (void *priv_data, double after,
			rmilter_periodic_callback cb, void *user_data);
	void (*del_periodic) (void *priv_data, void *ev_data);
	void (*stop_event) (void *priv_data, void *ev_data);
	void (*start_event) (void *priv_data, void *ev_data);
	void (*cleanup) (void *priv_data);
};

/*
 * Librmilter logger types
 */
/*
 * These types are somehow compatible with glib
 */
enum rmilter_log_level {
	RMILTER_LOG_ERROR = 1 << 3,
	RMILTER_LOG_WARNING = 1 << 4,
	RMILTER_LOG_INFO = 1 << 6,
	RMILTER_LOG_DEBUG = 1 << 7
};
typedef void (*rmilter_log_function) (
		void *log_data,
		enum rmilter_log_level level,
		const char *module,
		const char *id,
		const char *function,
		const char *format,
		va_list args
);

/**
 * Creates new milter and returns pointer to the opaque structure
 * @param callbacks callback functions
 * @param async asynchronous bindings
 * @param log log function callback
 * @param log_data opaque logging structure data
 */
struct rmilter_milter *rmilter_create (struct rmilter_callbacks *callbacks,
		struct rmilter_async_context *async,
		rmilter_log_function log,
		void *log_data);

/**
 * Consumes socket, creating new context using the specified milter and file
 * descriptor. File descriptor is **transferred** meaning, that you cannot
 * use it anymore. User data for the new session is passed using opaque pointer.
 *
 * @param milter milter structure
 * @param fd file descriptor to be used for the session
 * @param module module description (for logging)
 * @param id session id
 * @param ud opaque user data
 */
bool rmilter_consume_socket (struct rmilter_milter *milter, int fd,
		const char *module, const char *id, void *ud);

/**
 * Destroys milter and all its sessions
 */
void rmilter_destroy (struct rmilter_milter *milter);

/* Private functions used by async callbacks */
void rmilter_process_read (int fd, void *arg);
void rmilter_process_timer (void *arg);
void rmilter_process_write (int fd, void *arg);

#ifdef  __cplusplus
}
#endif

#endif
