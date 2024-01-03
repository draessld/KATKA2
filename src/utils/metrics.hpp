#include <sys/resource.h>

long get_mem_usage()
{
    struct rusage myusage;

    getrusage(RUSAGE_SELF, &myusage);
    return myusage.ru_maxrss;
}

double get_time_usage()
{
    struct rusage myusage;

    getrusage(RUSAGE_SELF, &myusage);
    double system_time = (double)myusage.ru_stime.tv_sec + (double)myusage.ru_stime.tv_usec / 1000000.0;
    double user_time = (double)myusage.ru_utime.tv_sec + (double)myusage.ru_utime.tv_usec / 1000000.0;
    return system_time + user_time;
}
