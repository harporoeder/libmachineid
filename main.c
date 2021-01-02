#include <stdio.h>

#include "machineid.h"

int
main()
{
    char buffer[256];

    machineid_raw(buffer, sizeof(buffer));

    printf("buffer %s", buffer);

    return 0;
}