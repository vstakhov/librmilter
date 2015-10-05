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

#ifndef LIBRDNS_LOGGER_H
#define LIBRDNS_LOGGER_H

#include <stdarg.h>
#include "librmilter.h"

void rmilter_logger_internal (void *log_data,
		enum rmilter_log_level level,
		const char *module,
		const char *id,
		const char *function,
		const char *format,
		va_list args);

void rmilter_logger_helper (struct rmilter_milter *m,
		enum rmilter_log_level level,
		const char *module,
		const char *id,
		const char *function,
		const char *format, ...);

#define msg_err_milter(...) do { rmilter_logger_helper (m, \
	RMILTER_LOG_ERROR, "milter", NULL, G_STRFUNC, __VA_ARGS__); } while (0)
#define msg_warn_milter(...) do { rmilter_logger_helper (m, \
    RMILTER_LOG_WARNING, "milter", NULL, G_STRFUNC, __VA_ARGS__); } while (0)
#define msg_info_milter(...) do { rmilter_logger_helper (m, \
	RMILTER_LOG_INFO, "milter", NULL, G_STRFUNC, __VA_ARGS__); } while (0)
#define msg_debug_milter(...) do { rmilter_logger_helper (m, \
	RMILTER_LOG_DEBUG, "milter", NULL, G_STRFUNC, __VA_ARGS__); } while (0)

#define msg_err_session(...) do { rmilter_logger_helper (s->m, \
	RMILTER_LOG_ERROR, s->module, s->id, G_STRFUNC, __VA_ARGS__); } while (0)
#define msg_warn_session(...) do { rmilter_logger_helper (s->m, \
	RMILTER_LOG_WARNING, s->module, s->id, G_STRFUNC, __VA_ARGS__); } while (0)
#define msg_info_session(...) do { rmilter_logger_helper (s->m, \
	RMILTER_LOG_INFO, s->module, s->id, G_STRFUNC, __VA_ARGS__); } while (0)
#define msg_debug_session(...) do { rmilter_logger_helper (s->m, \
	RMILTER_LOG_DEBUG, s->module, s->id, G_STRFUNC, __VA_ARGS__); } while (0)

#endif
