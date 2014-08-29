/*
 * Copyright (c) YASUOKA Masahiko <yasuoka@yasuoka.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/param.h>

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <pwd.h>
#include <sysexits.h>
#include <stdio.h>
#include <unistd.h>

static void	 open_file (const char *);

static void
usage(void)
{
	extern char	*__progname;

	fprintf(stderr,
	    "usage: %s [-h][-o path,mode,fd] user cmd [cmdarg]...\n",
	    __progname);
}

int
main(int argc, char *argv[])
{
	int		 ch, status;
	struct passwd	*pw;

	if (getuid() != 0)
		errx(EX_USAGE, "must be run as root");

	while ((ch = getopt(argc, argv, "o:h")) != -1)
		switch (ch) {
		case 'o':
			open_file(optarg);
			break;
ex_usage:
		case 'h':
			usage();
			exit(EX_USAGE);
			/* NOTREACHED */
		default:
			exit(EX_USAGE);
		}

	argc -= optind;
	argv += optind;

	if (argc < 2)
		goto ex_usage;

	/* Drop privileges */
	if ((pw = getpwnam(*argv)) == NULL)
		errx(EX_USAGE, "getpwnam(%s) failed", *argv);
	if (setgroups(1, &pw->pw_gid) ||
	    setresgid(pw->pw_gid, pw->pw_gid, pw->pw_gid) ||
	    setresuid(pw->pw_uid, pw->pw_uid, pw->pw_uid))
		err(EX_OSERR, "cannot drop privileges");

	argc--;
	argv++;
	status = execvp(*argv, argv);
	err(status, "execvp(%s)", *argv);
}

static void
open_file(const char *pathd)
{
	char		*comma, *mode, path[MAXPATHLEN];
	int		 newf, tmpf;
	const char	*errstr;
	FILE		*fp;

	if ((comma = strchr(pathd, ',')) == NULL)
		errx(EX_USAGE, "path description is wrong");
	if (comma - pathd + 1 > (int)sizeof(path))
		errx(EX_USAGE, "path description is too long");
	memcpy(path, pathd, comma - pathd);
	path[comma - pathd] = '\0';

	mode = ++comma;
	if ((comma = strchr(mode, ',')) == NULL)
		errx(EX_USAGE, "path description is wrong");

	*(comma++) = '\0';
	newf = strtonum(comma, 3, 64, &errstr);
	if (errstr != NULL)
		errx(EX_USAGE, "assigning fd number must be 3-64");

	if ((fp = fopen(path, mode)) == NULL)
		err(EX_OSERR, "fopen(%s, %s)", path, mode);

	if ((tmpf = dup(fileno(fp))) < 0)
		err(EX_OSERR, "dup()");
	fclose(fp);

	if (tmpf != newf) {
		if (dup2(tmpf, newf) < 0)
			err(EX_OSERR, "dup2()");
		close(tmpf);
	}
}
