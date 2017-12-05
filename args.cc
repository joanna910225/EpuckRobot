#include "args.hh"
#include <getopt.h>
#include <iostream>
#include <stdlib.h>
#include <string.h>

extern char *g_name;
extern int g_logtofile;
extern char *g_host;
extern int g_turning_speed;
extern int g_sensor_triggered_threshold;
extern int g_sensor_triggered_min_interval;
extern int g_sensor_triggered_counts;

static struct option long_options[]=
{
    {"name", required_argument, 0, 'A'},
    {"host", required_argument, 0, 'B'},
    {"log", required_argument, 0, 'D'},
    {"turnspeed", required_argument, 0, 'E'},
    {"sensor_triggered_threshold", required_argument, 0, 'F'},
    {"sensor_triggered_min_interval", required_argument, 0, 'G'},
    {"sensor_triggered_counts", required_argument, 0, 'H'},
    { 0, 0, 0, 0}
};


void print_usage(int argc, char** argv)
{
    std::cout << "USAGE:  " << *argv << " [options]" << std::endl << std::endl;
    std::cout << "Where [options] can be:"
        << std::endl;
    std::cout << " --name=<value>"
        << std::endl;
    std::cout << " --host=<value>"
        << std::endl;
    std::cout << " --log=<value>"
        << std::endl;
    std::cout << " --turnspeed=<value>"
        << std::endl;
    std::cout << " --sensor_triggered_threshold=<value>"
        << std::endl;
    std::cout << " --sensor_triggered_min_interval=<value>"
        << std::endl;
    std::cout << " --sensor_triggered_counts=<value>"
        << std::endl;
}

int parse_args(int argc, char** argv)
{
    int ch;
    int option_index = 0;

    // use getopt to parse the flags
    while(-1 != (ch = getopt_long (argc, argv, "A:B:C:D:E:F:G:",
                    long_options, &option_index)))
    {
        switch(ch)
        {
            // case values must match long_options
            case  0:
                break;
            case 'A':
                g_name = strdup(optarg);
                break;
            case 'B':
                g_host = strdup(optarg);
                break;
            case 'D':
                g_logtofile = atoi(optarg);
                break;
            case 'E':
                g_turning_speed = atoi(optarg);
                break;
            case 'F':
                g_sensor_triggered_threshold = atoi(optarg);
                break;
            case 'G':
                g_sensor_triggered_min_interval = atoi(optarg);
                break;
            case 'H':
                g_sensor_triggered_counts = atoi(optarg);
                break;
            case '?': // help
            case ':':
            default:  // unknown
                print_usage(argc, argv);
                exit (-1);
        }
    }

    return (0);
} 
