#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdint.h>

FILE *output_file;
struct tm *timeinfo;
time_t now, end_time;
int8_t num_sensors = -1, sampling = -1, status;
int32_t interval = -1;
char *endptr;

enum error_t
{
    error_1 = 1, //Wrong command-line statement
    error_2,     //Invalid value of the command-line argument
    error_3,     //Error permission access file
};

int get_argument(uint8_t argc, char **argv);
void log_error(int8_t status);

int main(uint8_t argc, char **argv)
{
    status = get_argument(argc, argv);
    if(status != 0)
    {
        //printf("%d", status);
        log_error(status);
        return 1;
    }
    output_file = fopen("dust_sensor.csv", "w");
    if(output_file == NULL)
    {
        log_error(error_3);
    }
    end_time = time(NULL);
    now = end_time - interval;
    srand(now);
    fprintf(output_file, "id,time,values\n");   // title
    while(now <= end_time)
    {
        for(int8_t id = 1; id <= num_sensors; id++)
        {
            timeinfo = localtime(&now);
            fprintf(output_file,"%d,%d:%02d:%02d %02d:%02d:%02d,%0.1f\n", id, (timeinfo->tm_year) + 1900, 
                     (timeinfo->tm_mon) + 1, 
                      timeinfo->tm_mday,
                      timeinfo->tm_hour, 
                      timeinfo->tm_min, 
                      timeinfo->tm_sec, (rand() % 5001) / 5.0);
        }
        now += sampling;
    }


   fclose(output_file);
   return 0;

   
}
int get_argument(uint8_t argc, char **argv)
{
    for(uint8_t i = 1; i < argc; i += 2)
    {
        if(i + 1 >= argc)
        {
            return error_1;
        }
        if(strcmp(argv[i], "-n") == 0)
        {   
            if(num_sensors != -1)
            {
                return error_1;
            }
            num_sensors = strtol(argv[i + 1], &endptr, 10);
            if (num_sensors == 0) 
            {
                return error_1;
            }
            else if(*endptr != 0)
            {
                return error_2;
            }
        }
        else if(strcmp(argv[i], "-st") == 0)
        {
            if(sampling != -1)
            {
                return error_1;
            }
            sampling = strtol(argv[i + 1], &endptr, 10);
            
            if(*endptr != 0 || sampling < 0)
            {
                return error_2;
            }
        }
        else if(strcmp(argv[i], "-si") == 0)
        {
            if(interval != -1)
            {
                return error_1;
            }
            interval = strtol(argv[i + 1], &endptr, 10); // strtol function return integral number type long int
            interval = interval * 3600;
            if(*endptr != 0)
            {
                return error_2;
            } 
        }
        else{
            return error_1;
        }
    }

    if (num_sensors == -1)
    {
         num_sensors = 1;
    }
    if (sampling == -1)
    {
          sampling = 30;
    }
    if (interval == -1)
    {
         interval = 86400;
    }
   
    
    return 0; // OK
}
void log_error(int8_t status){
    FILE *err_file;
    err_file = fopen("task1.log", "w");
    if(err_file == NULL)
    {
        printf("Can't open file");
    }
    switch (status)
    {
    case error_1:
        fprintf(err_file,"Error 01: invalid command");
        break;
    case error_2:
        fprintf(err_file,"Error 02: invalid argument");
        break;
    case error_3:
        fprintf(err_file,"Error 03: dust_sensor.csv access denied");
        break;
    }

    fclose(err_file);
}