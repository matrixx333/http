#include <stdio.h>
#include <string.h>
#include <stdlib.h>


enum {
    scm_has_query = 1,
    scm_has_fragment = 2
};

int main(int argc, char **argv){
    const char *url = "http://www.google.com/index.html";
    int i = 0;
    

    char seps[8] = ":/";
    char *p = seps + 2;
    int flags = scm_has_query|scm_has_fragment;

    printf("*p = %p\n", p);
    if(flags & scm_has_query){
        *p++ = '?';
        printf("*p = %d\t%p\n", *(p), p);
    }
    if(flags & scm_has_fragment){
        *p++ = '#';
        printf("*p = %p\n", p);
    }
    *p = '\0';

    for(i=0; i<8; i++)
        printf("seps[%d]: %c\t%p\n", i, seps[i], &seps[i]); 
    
    return 0;
}
