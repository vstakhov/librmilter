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

#include <stdio.h>
#include <time.h>
#include "logger.h"
#include "librmilter_internal.h"

#define LOG_ID 6

void rmilter_logger_internal (void *log_data,
		enum rmilter_log_level level,
		const char *module,
		const char *id,
		const char *function,
		const char *format,
		va_list args)
{
	struct rmilter_milter *m = log_data;
	const char *str_level = "";
	time_t now;
	struct tm *tms;
	char timebuf[32];

	switch (level) {
	case RMILTER_LOG_ERROR:
		str_level = "ERR";
		break;
	case RMILTER_LOG_WARNING:
		str_level = "WARN";
		break;
	case RMILTER_LOG_INFO:
		str_level = "INFO";
		break;
	case RMILTER_LOG_DEBUG:
		str_level = "DEBUG";
		break;
	}

	fprintf (stderr, "%s ", str_level);

	/* Format time */
	now = time (NULL);
	tms = localtime (&now);

	strftime (timebuf, sizeof (timebuf), "%F %H:%M:%S", tms);

	fprintf (stderr, "%s ", timebuf);

	if (module) {
		fprintf (stderr, "%s; ", module);
	}

	if (id) {
		fprintf (stderr, "<%.*s>; ", LOG_ID, id);
	}

	fprintf (stderr, "%s: ", function);
	vfprintf (stderr, format, args);
	fprintf (stderr, "\n");
}

void rmilter_logger_helper (struct rmilter_milter *m,
		enum rmilter_log_level level,
		const char *module,
		const char *id,
		const char *function,
		const char *format, ...)
{
	va_list va;

	if (m->log != NULL) {
		va_start (va, format);
		m->log (m->log_data, level, module, id, function, format, va);
		va_end (va);
	}
}