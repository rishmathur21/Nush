#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <assert.h>
#include "tokens.h"
#include "svec.h"

int findToken(svec* cmd, char* tok);
void doSemicolon(svec* cmd);
void doRightBranch(svec* cmd, char* token);
void execute(svec* cmd);

int 
findToken(svec* cmd, char* tok)
{
	// go through each token and check if it is the operator
	for (int ii = 0; ii < cmd->size; ii++)
	{
		// if it is return 1
		if (strcmp(tok, svec_get(cmd, ii)) == 0)
		{
			return ii;
		}
	}
	// if not found return 0
	return 0;
}


void
doSemicolon(svec* cmd)
{
	svec* leftbranch = make_svec();
	svec* rightbranch = make_svec();
	int tokenindexinvec = findToken(cmd, ";\0");

	// populate left_branch with all the tokens left of semicolon
	for (int ii = 0; ii < tokenindexinvec; ii++)
	{
		svec_push_back(leftbranch, svec_get(cmd, ii));
	}
	// populate right branch with all the tokens right of the semicolon
	for (int ii = tokenindexinvec + 1; ii < cmd->size; ii++)
	{
		svec_push_back(rightbranch, svec_get(cmd, ii));

	}
	
	execute(leftbranch);
	free(leftbranch);
	execute(rightbranch);
	free(rightbranch);
	return;
}

void
doRightBranch(svec* cmd, char* token)
{
	svec* rightBranch = make_svec();
	int tokenIndexInVec;
	tokenIndexInVec = findToken(cmd, token);
	// populate right branch with all the tokens right of the semicolon
	for (int ii = tokenIndexInVec + 1; ii < cmd->size; ii++)
	{
		svec_push_back(rightBranch, svec_get(cmd, ii));
		if (strcmp(svec_get(cmd, ii), "exit\0") == 0)
		{
			exit(0);
		}
	}
	execute(rightBranch);
	free_svec(rightBranch);
}


void
execute(svec* cmd)
{
	int cpid;
	int pipes[2];
	int rv = pipe(pipes);
	assert(rv != -1);

	// change directory case
	if (strcmp("cd\0", svec_get(cmd, 0)) == 0) 
	{
		chdir(svec_get(cmd, 1));
		return;
	}

	// if you find a semicolon
	else if (findToken(cmd, ";\0"))
	{
	
		doSemicolon(cmd);
		return;
	}

	// parse through the tokens linearly
	if ((cpid = fork()))
	{
		// parent process	
		int status;

		if (findToken(cmd, "|\0"))
		{
			// close the right end of the pipe
			close(pipes[1]);
			close(0);
			rv = dup(pipes[0]);
			assert(rv != -1);

			doRightBranch(cmd, "|\0");
			return;
		}

		// background process
		if (findToken(cmd, "&\0"))
		{
			return;
		}
		waitpid(cpid, &status, 0);
		

		// if child returns true
		if(WEXITSTATUS(status) == 0)
		{
			// and the operator is AND run the rightBranch
			if (findToken(cmd, "&&\0"))
			{
				doRightBranch(cmd, "&&\0");
				return;
			}

		}
		// if child was false however, 
		else
		{
			// and the operator is OR	
			if (findToken(cmd, "||\0"))
			{
				doRightBranch(cmd, "||\0");
				return;
			}
		}
	}
	else
	{
		// child
		int branchSize;
		char** args;
		
		// REDIRECT input
		if (findToken(cmd, "<\0"))
		{
			// get the file name from right after the <
			char* filename = svec_get(cmd, (findToken(cmd, "<\0") + 1));
			char* command = svec_get(cmd, 0);
			int fd = open(filename, O_RDONLY);
			
			// close stdin
			close(0);
			// copy file descriptor into lowest unused file descriptor
			dup(fd);
			close(fd);
			// run the command with the input from file
			char* argsRedir[] = { command, 0 };
			execvp(command, argsRedir);
			return;
		}
		else if(findToken(cmd, ">\0"))
		{
			// get the file name from right after the <
			char* filename = svec_get(cmd, (findToken(cmd, ">\0") + 1));
			int fd = open(filename, O_CREAT | O_APPEND | O_WRONLY, 0644);

			// close stdout
			close(1);
			// copy file descriptor into lowest unused file descriptor
			dup(fd);
			close(fd);
			branchSize = findToken(cmd, ">\0");			
		}

		else if(findToken(cmd, "&\0"))
		{
			branchSize = findToken(cmd, "&\0");
		}
		else if (findToken(cmd, "||\0"))
		{
			branchSize = findToken(cmd, "||\0");
		}
		else if (findToken(cmd, "&&\0"))
		{
			branchSize = findToken(cmd, "&&\0");
		}
		else if (findToken(cmd, "|\0"))
		{
			branchSize = findToken(cmd, "|\0");
			close(pipes[0]);
			close(1);
			rv = dup(pipes[1]);
			assert(rv != -1);
		}
		else
		{
			branchSize = cmd->size;
		}

		args = malloc((branchSize + 1) * sizeof(char*));
		for (int ii = 0; ii < branchSize; ii++)
		{
			args[ii] = svec_get(cmd, ii);
		}
		args[branchSize] = 0; 
		execvp(svec_get(cmd, 0), args);
	//	free(args);
	//	printf("Can't get here, exec only returns on error.\n");
		exit(1);
	}
	return;

}

int
main(int argc, char* argv[])
{
	char cmd[256];
	if (argc == 1) {
		printf("nush$ ");
		fflush(stdout);
		while(fgets(cmd, 256, stdin) != NULL)
		{
			// check for exit
			if (strcmp(cmd, "exit\n") == 0)
			{
				return 0;
			}

			// tokenizes the input, stores in an svec (in the order they are read)
			svec* tokens = tokenize(cmd);
			execute(tokens);
			free_svec(tokens);	

			printf("nush$ ");
			fflush(stdout);
		}
	}
	else {
		FILE* fp;
		fp = fopen(argv[1], "r");

		while(fgets(cmd, 256, fp) != NULL)
		{
			// check for exit
			if (strcmp(cmd, "exit\n") == 0)
			{
				fclose(fp);
				return 0;
			}

			// tokenizes the input, stores in an svec (in the order they are read)
			svec* tokens = tokenize(cmd);
			execute(tokens);
			free_svec(tokens);	

		}
	}
	return 0;
}
