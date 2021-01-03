#include "machineid.h"

#include <stdio.h>

int
main()
{
    unsigned char buffer[MACHINEID_UUID_SIZE + 1];

    machineid_generate(buffer, MACHINEID_FLAG_AS_UUID
        | MACHINEID_FLAG_NULL_TERMINATE);

    printf("machine id: %s\n", buffer);

    return 0;
}
