#define PI 3.14159265359f

#include <stdio.h>
#include <stdint.h>
#include <errno.h>   // for errno
#include <limits.h>  // for INT_MAX
#include <stdlib.h>  // for strtol
#include <math.h>

int main(int argc, char *argv[])
{
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    if (argc < 4)
    {
        printf("missing arguments!\n");
        printf("argument[1]: num of samples\n");
        printf("argument[2]: min value\n");
        printf("argument[3]: max value\n");
        return -1;
    }


    char * p;

    errno = 0;
    uint16_t samples = strtol(argv[1], &p, 10);
    // Check for errors: e.g., the string does not represent an integer
    // or the integer is larger than int
    if (errno != 0 || *p != '\0' || samples > INT_MAX) {
        // Put here the handling of the error, like exiting the program with
        // an error message
        printf("invalid input argument[1]: num of samples!\n");
        return -1;
    }

    errno = 0;
    uint16_t min = strtol(argv[2], &p, 10);
    // Check for errors: e.g., the string does not represent an integer
    // or the integer is larger than int
    if (errno != 0 || *p != '\0' || min > INT_MAX) {
        // Put here the handling of the error, like exiting the program with
        // an error message
        printf("invalid input argument[2]: min value!\n");
        return -1;
    }

    errno = 0;
    uint16_t max = strtol(argv[3], &p, 10);
    // Check for errors: e.g., the string does not represent an integer
    // or the integer is larger than int
    if (errno != 0 || *p != '\0' || max > INT_MAX) {
        // Put here the handling of the error, like exiting the program with
        // an error message
        printf("invalid input argument[3]: max value!\n");
        return -1;
    }

    double ac_sine[samples];
    double rms[samples];
    uint16_t lookup[samples];

    FILE *fp;


    // ac wave sample buffer
    fp = fopen("wave.txt", "w+");
    fprintf(fp, "double const ac_sine[] = {");
    ac_sine[0] = 0.0f;
    fprintf(fp, "%.7ff", ac_sine[0]);
    for(uint16_t i = 1; i <= samples; i++)
    {
        ac_sine[i] = (sin(2.f*PI*50.f*((0.02f/(float)samples)/2.f)*i));
        fprintf(fp, ", %.7ff", ac_sine[i]);
    }
    fprintf(fp, "}");
    fclose(fp);


    // rms sample buffer
    double sum;
    fp = fopen("rms.txt", "w+");
    fprintf(fp,"double const rms[] = {");
    sum += ac_sine[0] * ac_sine[0];
    rms[0] = sqrt(sum / samples);
    fprintf(fp,"%.7ff", rms[0]);
    for(uint16_t i = 1; i <= samples; i++)
    {
        sum += ac_sine[i] * ac_sine[i];
        rms[i] = sqrt(sum / samples);
        fprintf(fp, ", %.7ff", rms[i]);
    }
    fprintf(fp,"}");
    fclose(fp);

    // val sample buffer
    uint16_t val = 0;
    fp = fopen("out.txt", "w+");
    fprintf(fp, "uint16_t const lookup[] = {");
    lookup[0] = min;
    fprintf(fp, "%d", lookup[0]);
    for(uint16_t i = 1; i <= samples; i++)
    {
        float fTarget = (i*(1/sqrt(2)) / (float)samples);
        if (fTarget < min)
        {
            lookup[i] = min;
        }
        if (fTarget > max)
        {
            lookup[i] = max;
        }
        else
        {
            while(fTarget > rms[val])
            {
                if(val == samples) break;
                val++;
            }
            lookup[i] = val;
        }
        fprintf(fp, ", %d", lookup[i]);
    }
    fprintf(fp, "}");
    fclose(fp);

    return 0;
}