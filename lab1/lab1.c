#include <sys/resource.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ulimit.h>


int main(int argc, char* argv[]) {
	int opt;
	struct rlimit core_f;
	extern char** environ;
	char** env;
	while ((opt = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
		switch (opt) {
		case('i'):
			printf("Real userid = %d\n", getuid());
			printf("Effective userid = %d\n", geteuid());
			printf("Real groupid = %d\n", getgid());
			printf("Effective groupid = %d\n", getegid());
			break;

		case('s'):
			setpgid(0, 0);
			break;

		case('p'):
			printf("Process number = %d\n", getpid());
			printf("Parent process number = %d\n", getppid());
			printf("Group process number = %d\n", getpgid(0));
			break;

		case('u'):
			printf("ulimit = %ld\n", ulimit(UL_GETFSIZE) * 512);
			break;

		case('U'):
			long int new_ulimit = atol(optarg);
			new_ulimit = ulimit(UL_SETFSIZE, new_ulimit / 512);
			break;

		case('c'):
			getrlimit(RLIMIT_CORE, &core_f);
			printf("core size = %ld\n", core_f.rlim_cur);
			break;

		case('C'):
			//check
			getrlimit(RLIMIT_CORE, &core_f);
			core_f.rlim_cur = atol(optarg);
			if (setrlimit(RLIMIT_CORE, &core_f) == -1)
				fprintf(stderr, "Must be super-user to increase core\n");
			break;

		case('d'):
			printf("current working directory is: %s\n",
				getcwd(NULL, 100));
			break;

		case('v'):
			env = environ;
			while (*env) {
				printf("%s\n", *env);
				env++;
			}
			break;

		case('V'):
			putenv(optarg);
			break;
		}
	}
}