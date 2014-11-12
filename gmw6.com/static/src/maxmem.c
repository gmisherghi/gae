/**
 * Copyright (c) 2007, Ghassan Misherghi
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 *  * Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following disclaimer
 *    in the documentation and/or other materials provided with the
 *    distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

int pid;
long maxmem;

void update()
{
    char statfilename[1000];
    long memsize;
    long unused;
    FILE *stat;

    snprintf(statfilename, 999, "/proc/%d/statm", pid);
    stat= fopen(statfilename, "r");
    if (!stat) { perror("fopen"); exit(1); }
    /* man proc for information.  this uses the rss field, rather than the inacurate "total" field */
    if (fscanf(stat, "%ld %ld", &unused, &memsize) != 2) { perror("fscanf"); exit(1); }
    fclose(stat);
    if (memsize > maxmem) maxmem= memsize;
}

int main(int argc, char **argv)
{
    int wc, i;
    FILE *dumpf;

    if (argc == 1) { fprintf(stderr, "%s command [args]\n", argv[0]); exit(1); }
    switch (pid= fork()) {
    case 0: 
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        execvp(argv[1], argv+1);
        perror( "execvp" );
        exit(1);
    default:
        wait(&wc); 
        for (i= 0; !WIFEXITED(wc); i++) {
            if (i>3) { update(); } //allow initial exec
            if (ptrace(PTRACE_SYSCALL, pid, 0, 0) != 0) { perror("ptrace"); exit(1); }
            wait(&wc);
        }
    }

    dumpf= fopen("maxmem.log", "a");
    if (!dumpf) { perror("fopen"); exit(1); }
    fprintf(dumpf, "%s:\t\t%ld bytes\n", argv[1], maxmem*getpagesize());
    fclose(dumpf);
    exit(0);
}
