#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

#define MAX_BUF_SIZE 2048
#define PATTERN_CPU "Cpu"
#define PATTERN_PERC "([0-9]{1,3}([.,][0-9])?)[[:space:]]+id"

int main(){
    regex_t cpu_regex, perc_regex;
    regmatch_t match[2];
    char *buf = calloc(MAX_BUF_SIZE, sizeof(char));
    char num[16];
    FILE *fp = popen("LC_ALL=C top -b", "r");

    if(fp == NULL){
        fprintf(stderr, "Error: Unable to open top command\n");
        return 1;
    }

    if(regcomp(&cpu_regex, PATTERN_CPU, REG_NOSUB | REG_EXTENDED) != 0 || 
       regcomp(&perc_regex, PATTERN_PERC, REG_EXTENDED) != 0){
        fprintf(stderr, "Error: Unable to compile regex\n");
        return 1;
    }

    while(fgets(buf, MAX_BUF_SIZE, fp) != NULL){
        buf[strcspn(buf, "\n")] = 0;
        if(regexec(&cpu_regex, buf, 0, (regmatch_t*)NULL, 0) == 0){

            if(regexec(&perc_regex, buf, 2, match, 0) == 0){

                int len = (int)(match[1].rm_eo - match[1].rm_so);

                if(len < (int)sizeof(num)){

                    memcpy(num, &buf[match[1].rm_so], len);
                    num[len] = '\0';

                    double idle = strtod(num, NULL);
                    double usage = 100.0 - idle;

                    printf("\r\x1b[K\x1b[37;42mCPU Usage: %.1f%% \x1b[?25l\x1b[0m", usage);
                    fflush(stdout);
                }       
            }else{
                fprintf(stderr, "Error: Unable to find CPU usage percentage in line: %s\n", buf);
            }
        }
    }

    pclose(fp);
    free(buf);
    regfree(&cpu_regex);
    regfree(&perc_regex);

    return 0;
}