// -------------------------------------------------------------include library--------------------------------------------
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>// include this library in order to use atoi function
#include <time.h> 
#include <stdint.h>

// -------------------------------------------------------------macro definitions------------------------------------------------
#define MAX_FILE_NAME 100 // max length of file name
#define MAX_LINE 100
#define MAX_SENSORS 100000 // max number of sensor
#define MAX_COPY 100

// typedef struct
typedef struct{
    int id;
    char time[20];
    float value;
    char parameter[5];
    float aqi;
    char pollution[25];
    uint8_t duration;
   
} sensor;

// -----------------------------------------------------------global variable------------------------------------------------
sensor sensors[MAX_SENSORS];            
sensor invalid_data_sensors[MAX_SENSORS];
sensor valid_data_sensors[MAX_SENSORS];
sensor aqi_data_sensor[MAX_SENSORS];
sensor summary_data_sensor[MAX_SENSORS];
sensor statistic_data_sensor[MAX_SENSORS];
char tmp[40], *endptr;
int g_n_data, n_invalid_data, n_valid_data;
uint8_t g_max_id;
long long min_ts, max_ts;
uint32_t g_n_aqi = 0;

// ------------------------------------------------------------function prototype--------------------------------------------
int read_file_csv(char filename[MAX_FILE_NAME]);
int task2_1();
uint32_t task2_2();
int task2_3();
int task2_4();
void bubbleSort(sensor arr[], int n);
time_t check_valid_time(char *s);
void log_error(int status, int line);

int main(int argc, char **argv){
    if(argc < 2){
        log_error(3, 0);
        return 1;
    }
    char input_file[MAX_FILE_NAME];
    strcpy(input_file, argv[1]);
    if(read_file_csv(input_file) != 0){          // read data from csv file and store into a struct
        return 1;
    }
    // print the result into screen
    task2_1();
    task2_2();
    task2_3();
    task2_4();
}

// ------------------------------------------function definition--------------------------------------------------------------
//    function read csv file and store in a struct
int read_file_csv(char filename[MAX_FILE_NAME]){
    FILE *myfile;
    char *field, *endptr;
    myfile = fopen(filename, "r");
    if (myfile == NULL){
        log_error(1, 0);
        printf("Can't open file\n");
        return 1;
    }
    // process
    char line[MAX_LINE]; // store the first 1024 lines into buffer
    uint32_t row_count = 0;
    uint8_t field_count = 0;
    char buff[4][40];
    time_t ts;

    g_n_data = 0;
    g_max_id = 1;
    min_ts = 99999999999;
    max_ts = 0;
    while (fgets(line, MAX_COPY, myfile) != NULL){
        row_count++;
        if (row_count == 1){
            continue; // we did not get the label from row 1
        }
        field_count = 0;
        field = strtok(line, ",");
        while(field != NULL){
            if(field_count >= 3){
                log_error(2, 0);
            }
            strcpy(buff[field_count], field);
            field_count++;
            field = strtok(NULL, ",");
        }
        //printf("%d", field_count);
        if(field_count < 3){
            printf("missing data in line %d", row_count);
            log_error(4, row_count);
            return 1;
        }
        // get id from buff
        sensors[g_n_data].id = strtol(buff[0], &endptr, 10);
        if(*endptr != 0){
            log_error(2, 0);
            return 1;
        }
        if(g_max_id < sensors[g_n_data].id){
            g_max_id = sensors[g_n_data].id;
        }
        // get time from buff
        ts = check_valid_time(buff[1]);
        printf("%d", ts);
        if(ts == -1){
            log_error(2, 0);
            return 1;
        }     
        if(min_ts > ts){
            min_ts = ts;
        }    
        if(max_ts < ts){
            max_ts = ts;
        }
        strcpy(sensors[g_n_data].time, buff[1]);
        // get valule from buff
        sensors[g_n_data].value = strtof(buff[2], &endptr);
        if(*endptr != '\n'){
            log_error(2, 0);
            return 1;
        }  
        g_n_data++;
    }

    fclose(myfile);
    return 0;
}
// function have to filter invalid data
int task2_1(){
    int j = 0, k = 0;
    // process
    for (int i = 0; i < g_n_data; i++){
        if ((sensors[i].value >= 5.0 && sensors[i].value <= 550.5)){
            valid_data_sensors[k].id = sensors[i].id; // invalid_data
            strcpy(valid_data_sensors[k].time, sensors[i].time);
            valid_data_sensors[k].value = sensors[i].value;
            k++;
            continue;
        }
        else{
            invalid_data_sensors[j].id = sensors[i].id; // invalid_data
            strcpy(invalid_data_sensors[j].time, sensors[i].time);
            invalid_data_sensors[j].value = sensors[i].value;
            j++;
        }
    }
    n_valid_data = k;
    n_invalid_data = j;
    
    // store result in file outlier_data.csv
    FILE *fd_task2_1;
    fd_task2_1 = fopen("dust_outlier.csv", "w");
    if (fd_task2_1 == NULL){
        printf("Can't open file\n");
        log_error(4, 0);
        return 1;
    }
    fprintf(fd_task2_1, "number of outliers: %d\n", n_invalid_data);
    fprintf(fd_task2_1, "id\t,time\t,values\n");
    for (int i = 0; i < n_invalid_data; i++){
        fprintf(fd_task2_1, "%d,%s,%.1f\n", invalid_data_sensors[i].id, invalid_data_sensors[i].time, invalid_data_sensors[i].value);

    }
    fclose(fd_task2_1);
}
// task2.2
uint32_t task2_2(){
    float sum = 0;
    float f_average = 0;
    uint32_t k = 0, idx =0;
    int count = 0;
    uint8_t hour, day;
    char ptr[12], pday[3];
    char dest[3];
    char dest1[3];
    memset(dest,(int)0,3);
    memset(dest1,(int)0,3);
    int8_t cmp = 0;

    for(int id = 1; id <= g_max_id; id++){
        for(int j = 0; j < n_valid_data; j++){
            strncpy(dest, valid_data_sensors[j].time + 11, 2);
            strncpy(dest1, valid_data_sensors[j + 1].time + 11, 2);
            cmp = strcmp(dest, dest1);
            /* if((cmp != 0) && (valid_data_sensors[j].id == g_max_id))
            {
                cmp = 0;
            } */
            if(cmp == 0) 
            {
                if(valid_data_sensors[j].id == id)
                {
                    count++;
                    sum += valid_data_sensors[j].value;
                }else{
                    continue;
                }
                f_average = sum / count;
            }else{
                aqi_data_sensor[idx].value = f_average;
                aqi_data_sensor[idx].id = id; 

                strncpy(aqi_data_sensor[idx].time, valid_data_sensors[j].time, 8); //2023:06:
                strncpy(pday, valid_data_sensors[j].time + 8, 2);
                hour = atoi(dest) + 1;
                if(hour >=24){
                    hour = 0;
                    day = atoi(pday) + 1;
                    sprintf(pday,"%02d", day);
                    /*Can handle string_day*/
                    sprintf(ptr," %02d:00:00", hour);
                    strcat(aqi_data_sensor[idx].time, pday);
                    strcat(aqi_data_sensor[idx].time,ptr);
                }else{
                    strcat(aqi_data_sensor[idx].time, pday);
                    sprintf(ptr," %02d:00:00", hour);
                    strcat(aqi_data_sensor[idx].time,ptr);
                }
                //strcpy(aqi_data_sensor[idx].time , valid_data_sensors[j].time);

                if(f_average < 12 && f_average >= 0){
                    aqi_data_sensor[idx].aqi = (f_average * 50) / (12 - 0);
                    strcpy(aqi_data_sensor[idx].pollution, "Good");
                }else if(f_average < 35.5 && f_average >= 12){
                    aqi_data_sensor[idx].aqi = (f_average *2.12) + 24.46;
                    strcpy(aqi_data_sensor[idx].pollution, "Moderate");
                }else if(f_average < 55.5 && f_average >= 35.5){
                    aqi_data_sensor[idx].aqi = (f_average *2.5) + 11.25;
                    strcpy(aqi_data_sensor[idx].pollution, "Slightly unhealthy");
                }else if(f_average < 150.5 && f_average >= 55.5){
                    aqi_data_sensor[idx].aqi = (f_average *0.52) + 120.78;
                    strcpy(aqi_data_sensor[idx].pollution, "Unhealthy");
                }else if(f_average < 250.5 && f_average >= 150.5){
                    aqi_data_sensor[idx].aqi = (f_average *1) + 49.5;
                    strcpy(aqi_data_sensor[idx].pollution, "Very unhealthy");
                }else if(f_average < 350.5 && f_average >= 250.5){
                    aqi_data_sensor[idx].aqi = (f_average *1) + 49.5;
                    strcpy(aqi_data_sensor[idx].pollution, "Hazardous");
                }else if(f_average <=  550.5 && f_average >= 350.5){
                     aqi_data_sensor[idx].aqi = (f_average *0.5) + 224.75;
                    strcpy(aqi_data_sensor[idx].pollution, "Extremely hazardous");
                }
                        /* code */
                idx++; 
                    
                sum = 0;
                count = 0;  
                f_average = 0;
            }
        }
    }
    g_n_aqi = idx;
    bubbleSort(aqi_data_sensor, idx);
    FILE *fd_task2_2;
    fd_task2_2 = fopen("dust_aqi.csv", "w");
    if (fd_task2_2 == NULL){
        printf("Can't open file\n");
        return 1;
    }
    fprintf(fd_task2_2, "id\t,time\t,values\t,aqi\t,pollution\n");
    for(uint8_t i = 0; i < idx; i++)
    {
        fprintf(fd_task2_2,"%d,%s,%0.1f,%0.0f,%s\n",aqi_data_sensor[i].id,
                                                aqi_data_sensor[i].time, 
                                                aqi_data_sensor[i].value, 
                                                aqi_data_sensor[i].aqi,
                                                aqi_data_sensor[i].pollution);
    }
    fclose(fd_task2_2);
    return 0;
}
// task2.3
int task2_3(){
    int j;
    // process
    float sum = 0;
    float f_average = 0;
    int count = 0;
    float max_speed;
    float min_speed;
    char min_time[25];
    char max_time[25];
    int k = 0;

    // compute simulation interval time
    time_t interval = max_ts - min_ts;

    for (int idx = 1; idx <= g_max_id; idx++){
        for (int j = 0; j < n_valid_data; j++){
            if (sensors[j].id == idx){ // check data if it has same id
                count++;        // count number of id
                if (count == 1){ // when you get an id at firts, then create the defaul value for max_speed, min_speed, max_time and min time
                    max_speed = valid_data_sensors[j].value;
                    min_speed = valid_data_sensors[j].value;
                    strcpy(max_time, valid_data_sensors[j].time);
                    strcpy(min_time, valid_data_sensors[j].time);
                }
                sum += valid_data_sensors[j].value;                // add value with sum
                if (max_speed < valid_data_sensors[j].value){ // check to find out the max speed in the same id
                    max_speed = valid_data_sensors[j].value;
                    strcpy(max_time, valid_data_sensors[j].time);
                }
                if (min_speed > valid_data_sensors[j].value){ // check to find out the min speed in the same id
                    min_speed = valid_data_sensors[j].value;
                    strcpy(min_time, valid_data_sensors[j].time);
                }
            }
            else{
                continue;
            }
        }
        f_average = sum / count;             // caculate the mean value
        // store max value
        summary_data_sensor[k].id = idx;
        strcpy(summary_data_sensor[k].time, max_time);
        strcpy(summary_data_sensor[k].parameter, "max");
        summary_data_sensor[k].value = max_speed;
        // store min value
        summary_data_sensor[k + 1].id = idx;
        strcpy(summary_data_sensor[k + 1].time, min_time);
        strcpy(summary_data_sensor[k + 1].parameter, "min");
        summary_data_sensor[k + 1].value = min_speed;
        // store mean value
        summary_data_sensor[k + 2].id = idx;
        sprintf(summary_data_sensor[k + 2].time, "%02d:%02d:%02d", interval / 3600, interval % 3600 / 60, interval % 60);
        strcpy(summary_data_sensor[k + 2].parameter, "mean");
        summary_data_sensor[k + 2].value = f_average;

        f_average = 0;
        k = k + 3;
        sum = 0;
        count = 0;
    }
    FILE *fd_task2_3;
    fd_task2_3 = fopen("dust_summary.csv", "w");
    if (fd_task2_3 == NULL){
        printf("Can't open file\n");
        return 1;
    }
    fprintf(fd_task2_3, "id\t,parameter\t,time\t,values\n");
    for (int i = 0; i < k; i++){
        fprintf(fd_task2_3,"%d,%s,%s,%.1f\n", summary_data_sensor[i].id, summary_data_sensor[i].parameter, summary_data_sensor[i].time, summary_data_sensor[i].value);
    }
    fclose(fd_task2_3);
    return 0;   // OK
}
// task2.4
int task2_4()
{
    int8_t count1 = 0;
    int8_t count2 = 0;
    int8_t count3 = 0;
    int8_t count4 = 0;
    int8_t count5 = 0;
    int8_t count6 = 0;
    int8_t count7 = 0;
    int8_t n_statistics;
    for(uint8_t id = 1; id <= g_max_id; id++){
        for(uint8_t j = 0; j < g_n_aqi; j++){
            if(aqi_data_sensor[j].id == id)
            {
                if(strcmp(aqi_data_sensor[j].pollution,"Good") == 0)
                { 
                    count1++;
                }else if(strcmp(aqi_data_sensor[j].pollution,"Moderate") == 0){
                
                    count2++;
                }else if(strcmp(aqi_data_sensor[j].pollution,"Slightly unhealthy") == 0){
                
                    count3++;
                }else if(strcmp(aqi_data_sensor[j].pollution,"Unhealthy") == 0){
                
                    count4++;
                }else if(strcmp(aqi_data_sensor[j].pollution,"Very unhealthy") == 0){
                
                    count5++;
                }else if(strcmp(aqi_data_sensor[j].pollution,"Hazardous") == 0){
                
                    count6++;
                }else if(strcmp(aqi_data_sensor[j].pollution,"Extremely hazardous") == 0){
               
                    count7++;
                }
            }
        
        }
        statistic_data_sensor[n_statistics].id = id;
        strcpy(statistic_data_sensor[n_statistics].pollution,"Good");
        statistic_data_sensor[n_statistics].duration = count1;

        statistic_data_sensor[n_statistics + 1].id = id;
        strcpy(statistic_data_sensor[n_statistics + 1].pollution,"Moderate");
        statistic_data_sensor[n_statistics + 1].duration = count2;

        statistic_data_sensor[n_statistics + 2].id = id;
        strcpy(statistic_data_sensor[n_statistics + 2].pollution,"Slightly unhealthy");
        statistic_data_sensor[n_statistics + 2].duration = count3;

        statistic_data_sensor[n_statistics + 3].id = id;
        strcpy(statistic_data_sensor[n_statistics + 3].pollution,"Unhealthy");
        statistic_data_sensor[n_statistics + 3].duration = count4;

        statistic_data_sensor[n_statistics + 4].id = id;
        strcpy(statistic_data_sensor[n_statistics + 4].pollution,"Very unhealthy");
        statistic_data_sensor[n_statistics + 4].duration = count5;

        statistic_data_sensor[n_statistics + 5].id = id;
        strcpy(statistic_data_sensor[n_statistics + 5].pollution,"Hazardous");
        statistic_data_sensor[n_statistics + 5].duration = count6;

        statistic_data_sensor[n_statistics + 6].id = id;
        strcpy(statistic_data_sensor[n_statistics + 6].pollution,"Extremely hazardous");
        statistic_data_sensor[n_statistics + 6].duration = count7;

        n_statistics = n_statistics + 7;
        count1 = 0;
        count2 = 0;
        count3 = 0;
        count4 = 0;
        count5 = 0;
        count6 = 0;
        count7 = 0;
    }
    FILE *fd_task2_4;
    fd_task2_4 = fopen("dust_statistics.csv", "w");
    if (fd_task2_4 == NULL){
        printf("Can't open file\n");
        return 1;
    }
    fprintf(fd_task2_4, "id\t,pollution\t,duration\n");
    for(uint8_t i = 0; i < n_statistics; i++)
    {
        fprintf(fd_task2_4,"%d\t,%s\t,%d\n", statistic_data_sensor[i].id,
                                 statistic_data_sensor[i].pollution,
                                 statistic_data_sensor[i].duration);
    }
    fclose(fd_task2_4);
    return 0;   // OK

}
time_t check_valid_time(char *s) {
    char *token, *endptr, tmp[20];
    int n, count = 0;
    time_t ts = time(NULL);
    struct tm *timeinfo = localtime(&ts);
    strcpy(tmp, s);
    token = strtok(tmp, " :-");
    
    while (token != NULL) {
        n = strtol(token, &endptr, 10);
       
        if (count == 0) {
            timeinfo->tm_year = n - 1900;
           
        }
        else if (count == 1) {
            timeinfo->tm_mon = n - 1;
            
        }
        else if (count == 2) {
            timeinfo->tm_mday = n;
            
        }
        else if (count == 3) {
            timeinfo->tm_hour = n;
            
        }
        else if (count == 4) {
            timeinfo->tm_min = n;
           
        }
        else if (count == 5) {
            timeinfo->tm_sec = n;
            
        }
        token = strtok(NULL, " :-");
        count++;
    }
    if(count < 6){
        return -1;
    }
    ts = mktime(timeinfo);
    return ts;
}

void log_error(int status, int line){
    FILE *log_file = fopen("task2.log", "w");
    if(log_file == NULL){
        return;
    }
    switch(status){
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
        
    };
    fclose(log_file);
}
void bubbleSort(sensor arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (strcmp(arr[j].time, arr[j + 1].time) > 0) {
                sensor temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}