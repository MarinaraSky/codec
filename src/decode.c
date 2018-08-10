#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "zergStructs.h"
#include "zergProtos.h"

int
main(
    int argc,
    char *argv[])
{
    if (argc > 1)
    {
        FILE           *psychicCapture;

        psychicCapture = fopen(argv[1], "rb");
        if (psychicCapture == NULL)
        {
            printf("Cannot open %s\n", argv[1]);
            exit(1);
        }
        readPcapHeader(psychicCapture);
        while (!feof(psychicCapture))
        {
            if (getc(psychicCapture) == EOF)
            {
                break;
            }
            else
            {
                fseek(psychicCapture, -1, SEEK_CUR);
            }
            parseCapture(psychicCapture);
        }
        fclose(psychicCapture);
    }
    return 0;
}
