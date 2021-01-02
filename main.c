#include <stdio.h>
#include <string.h>

#include "machineid.h"

int
main()
{
    char buffer[MACHINEID_UUID_SIZE + 1];

    machineid_generate(buffer, MACHINEID_FLAG_AS_UUID
        | MACHINEID_FLAG_NULL_TERMINATE);

    printf("machine id: %s\n", buffer);

    return 0;
}