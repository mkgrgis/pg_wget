#include "postgres.h"

#include "fmgr.h"
#include "lib/stringinfo.h"
#include "utils/builtins.h"

#include <errno.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(pg_wget);

Datum
pg_wget(PG_FUNCTION_ARGS)
{
	text		   *url_text;
	char		   *url;
	int				pipefd[2];
	pid_t			pid;
	StringInfoData  buf;
	char			read_buf[4096];
	ssize_t			bytes_read;
	int				status;

	if (PG_ARGISNULL(0))
		PG_RETURN_NULL();

	url_text = PG_GETARG_TEXT_PP(0);
	url = text_to_cstring(url_text);

	/* Basic validation to prevent flag injection (e.g., passing "-O /etc/...") */
	if (strncmp(url, "http://", 7) != 0 && strncmp(url, "https://", 8) != 0)
		ereport(ERROR,
				(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
				 errmsg("URL must start with http:// or https://")));

	if (pipe(pipefd) == -1)
		ereport(ERROR,
				(errcode_for_file_access(),
				 errmsg("could not create pipe: %m")));

	pid = fork();
	if (pid == -1)
	{
		close(pipefd[0]);
		close(pipefd[1]);
		ereport(ERROR,
				(errcode(ERRCODE_SYSTEM_ERROR),
				 errmsg("could not fork process: %m")));
	}

	if (pid == 0)
	{
		/* Child process: redirect stdout to pipe and execute wget */
		close(pipefd[0]);
		if (dup2(pipefd[1], STDOUT_FILENO) == -1)
			_exit(127);
		close(pipefd[1]);

		/* -q: quiet (no progress bar), -O -: output document to stdout */
		execlp("wget", "wget", "-T", "10", "-q", "-O", "-", url, (char *) NULL);
		_exit(127);
	}

	/* Parent process: read stdout from pipe */
	close(pipefd[1]);
	initStringInfo(&buf);

	while ((bytes_read = read(pipefd[0], read_buf, sizeof(read_buf))) > 0)
	{
		appendBinaryStringInfo(&buf, read_buf, bytes_read);
	}

	close(pipefd[0]);

	if (waitpid(pid, &status, 0) == -1)
		ereport(ERROR,
				(errcode(ERRCODE_SYSTEM_ERROR),
				 errmsg("waitpid failed: %m")));

	if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
		ereport(ERROR,
				(errcode(ERRCODE_EXTERNAL_ROUTINE_EXCEPTION),
				 errmsg("wget exited with non-zero status: %d",
						WIFEXITED(status) ? WEXITSTATUS(status) : -1)));

	PG_RETURN_TEXT_P(cstring_to_text_with_len(buf.data, buf.len));
}
