#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>// include this library in order to use atoi function
#include <time.h> 
#include <stdint.h>

#define MAX_FILE_NAME 100
#define MAX_COPY 100
#define MAX_SENSOR (10000 + 1)
#define g_start_byte 0x7A
#define g_packet 0x0F
#define g_stop_byte 0x7F
typedef struct
{
    uint8_t id;
    char time[25];
    float value;
    uint32_t aqi;
    char pollution[25];
} _Typedef_sensor;
_Typedef_sensor input_sensor[MAX_SENSOR];

int read_input_filename(char file_name_input[MAX_FILE_NAME]);
time_t check_valid_time(char *s);
void handle_string(uint32_t value, char *pdest);
void log_error(int status, int line);
uint32_t floatToIEEE754(float value);
uint16_t g_n_data;
uint32_t unix_time;
uint8_t check_sum;
uint32_t pm2_5;
char str_time[20], str_aqi[10], str_concentration[20];

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        log_error(3, 0);
        return 1;
    }
    char file_name_input[MAX_FILE_NAME];
    char file_name_output[MAX_FILE_NAME];
    strcpy(file_name_output, argv[2]);
    strcpy(file_name_input, argv[1]);
    if (read_input_filename(file_name_input) != 0)
    { // read data from csv file and store into a struct
        return 1;
    }

    FILE *fd_hexfile;
    fd_hexfile = fopen(file_name_output, "w");
    if (fd_hexfile == NULL)
    {
        log_error(5, 0);
        return 1;
    }
    for (uint16_t i = 0; i < g_n_data; i++)
    {
        pm2_5 = floatToIEEE754(input_sensor[i].value);
        unix_time = check_valid_time(input_sensor[i].time);
        handle_string(unix_time, str_time);
        handle_string(input_sensor[i].aqi, str_aqi);
        handle_string(pm2_5, str_concentration);
        check_sum = ~(g_packet + input_sensor[i].id + unix_time + pm2_5 + input_sensor[i].aqi) + 1;
        fprintf(fd_hexfile, "%02X %02X %02X %s %s %s %02X %02X\n", g_start_byte,
                g_packet,
                input_sensor[i].id,
                str_time,
                str_concentration,
                str_aqi,
                check_sum,
                g_stop_byte);
    }
    fclose(fd_hexfile);
    return 0;
}
int read_input_filename(char file_name_input[MAX_FILE_NAME]) // read file input
{
    FILE *myfile;
    char *field, *endptr;
    char buff[7][40];
    memset(buff, (int)0, sizeof(buff));
    myfile = fopen(file_name_input, "r");
    if (myfile == NULL)
    {
        log_error(1, 0);
        return 1;
    }
    char line[MAX_COPY];
    uint32_t row_count = 0;
    uint32_t field_count = 0;

    while (fgets(line, MAX_COPY, myfile) != NULL) // read line of input file
    {
        row_count++;
        if (row_count == 1)
        {
            continue;
        }
        field_count = 0;
        field = strtok(line, ",");
        while (field != NULL)
        {
            strcpy(buff[field_count], field);
            field_count++;
            field = strtok(NULL, ",");
        }
        if (field_count != 5)
        {
            log_error(4, row_count);
            return 1;
        }
        input_sensor[g_n_data].id = strtol(buff[0], &endptr, 10);
        if (*endptr != 0)
        {
            log_error(2, 0);
            return 1;
        }
        strcpy(input_sensor[g_n_data].time, buff[1]);

        input_sensor[g_n_data].value = strtof(buff[2], &endptr);
        if (*endptr != 0)
        {
            log_error(2, 0);
            return 1;
        }
        input_sensor[g_n_data].aqi = strtol(buff[3], &endptr, 10);
        if (*endptr != 0)
        {
            log_error(2, 0);
            return 1;
        }

        strcpy(input_sensor[g_n_data].pollution, buff[4]);

        g_n_data++;
    }
    fclose(myfile);
    return 0;
}
time_t check_valid_time(char *s)
{
    char *token, *endptr, tmp[20];
    int n, count = 0;
    time_t ts = time(NULL);
    struct tm *time_info = localtime(&ts);
    strcpy(tmp, s);
    token = strtok(tmp, " :-");
    while (token != NULL) // declare information to struct tm *time_info
    {
        n = strtol(token, &endptr, 10);
        if (count == 0)
        {
            time_info->tm_year = n - 1900;
        }
        else if (count == 1)
        {
            time_info->tm_mon = n - 1;
        }
        else if (count == 2)
        {

            time_info->tm_mday = n;
        }
        else if (count == 3)
        {

            time_info->tm_hour = n;
        }
        else if (count == 4)
        {

            time_info->tm_min = n;
        }
        else if (count == 5)
        {

            time_info->tm_sec = n;
        }
        token = strtok(NULL, " :-");
        count++;
    }
    if (count < 6)
    {
        return -1;
    }
    ts = mktime(time_info); // Unix time
    return ts;
}

void handle_string(uint32_t value, char *pdest)
{

    char str[20];
    memset(str, (int)0, sizeof(str));
    sprintf(str, "%04X", value); // convert value to string
    uint16_t len = strlen(str);
    uint16_t j = 0;

    for (uint16_t i = 0; i < len; i += 2)
    { // handle after 2 byte will add ' '
        pdest[j++] = str[i];
        pdest[j++] = str[i + 1];
        pdest[j++] = ' ';
    }
    pdest[j - 1] = '\0';
}

uint32_t floatToIEEE754(float value)
{
    union
    {
        float f;
        uint32_t u;
    } converter;
    converter.f = value;
    return converter.u;
}
void log_error(int status, int line)
{
    FILE *log_file = fopen("task3.log", "w");
    if (log_file == NULL)
    {
        return;
    }
    switch (status)
    {
    case 1:
        fprintf(log_file, "Error 01: input file not found or not accessible");
        break;
    case 2:
        fprintf(log_file, "Error 02: invalid csv file format");
        break;
    case 3:
        fprintf(log_file, "Error 03: invalid command");
        break;
    case 4:
        fprintf(log_file, "Error 04: data is missing at line %d", line);
        break;
    case 5:
        fprintf(log_file, "Error 05: cannot override hex_filename.dat");
        break;
    };
    fclose(log_file);
}