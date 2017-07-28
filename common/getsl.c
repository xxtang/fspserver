#include "tweak.h"
#include <stdio.h>
#include "my-string.h"
#include "common_def.h"
#include "co_extern.h"

char *getsl(char *s, int l) {
    int i;
    char *rv = fgets(s, l, stdin);
    if(rv && (i = strlen(rv)) > 0) if(rv[i - 1] == '\n') rv[i - 1] = '\0';
    return rv;
}
