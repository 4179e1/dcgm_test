#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <unistd.h>

#include "dcgm_agent.h"
#include "dcgm_structs.h"
#include "dcgm_errors.h"

#define MAX_FIELDS 100 // Define a maximum number of elements you expect to handle

typedef struct
{
    long long interval; // usec
    long long duration; // usec
    int loops;
    unsigned short fields[MAX_FIELDS];
    int field_count;
    double ttl;
} Config;

void parse_fields(const char *str, unsigned short *fields, int *field_count)
{
    char *token;
    char *input = strdup(str); // Duplicate the input string to avoid modifying the original
    int index = 0;

    token = strtok(input, ",");
    while (token != NULL && index < MAX_FIELDS)
    {
        fields[index] = (unsigned short)strtoul(token, NULL, 10);
        index++;
        token = strtok(NULL, ",");
    }

    *field_count = index;
    free(input); // Free the duplicated string
}

void print_config(const Config *config)
{
    fprintf(stderr, "Configuration:\n");
    fprintf(stderr, "Interval: %lld\n", config->interval);
    fprintf(stderr, "Duration: %lld\n", config->duration);
    fprintf(stderr, "TTL: %f\n", config->ttl);
    fprintf(stderr, "Loops: %d\n", config->loops);
    fprintf(stderr, "Fields: ");
    for (int i = 0; i < config->field_count; i++)
    {
        fprintf(stderr, "%hu", config->fields[i]);
        if (i < config->field_count - 1)
        {
            fprintf(stderr, ", ");
        }
    }
    fprintf(stderr, "\n");
}

int myCallback(dcgm_field_entity_group_t entityGroupId, dcgm_field_eid_t entityId, dcgmFieldValue_v1 *values, int numValues, void *userData)
{
    dcgmReturn_t result;
    dcgm_field_meta_p fieldMeta;

    for (int i = 0; i < numValues; i++)
    {
        dcgmFieldValue_v1 *v = &values[i];
        if (v->fieldId == DCGM_FI_DEV_CLOCK_THROTTLE_REASONS)
        {
            int64_t fv = v->value.i64;
            if (fv & DCGM_CLOCKS_THROTTLE_REASON_GPU_IDLE)
            {
                printf("%d,%ld,%s,%lld\n", entityId, v->ts, "throttle_reason_idle", DCGM_CLOCKS_THROTTLE_REASON_GPU_IDLE);
            }
            if (fv & DCGM_CLOCKS_THROTTLE_REASON_CLOCKS_SETTING)
            {
                printf("%d,%ld,%s,%lld\n", entityId, v->ts, "throttle_reason_clocks_settings", DCGM_CLOCKS_THROTTLE_REASON_CLOCKS_SETTING);
            }
            if (fv & DCGM_CLOCKS_THROTTLE_REASON_SW_POWER_CAP)
            {
                printf("%d,%ld,%s,%lld\n", entityId, v->ts, "throttle_reason_sw_power_cap", DCGM_CLOCKS_THROTTLE_REASON_SW_POWER_CAP);
            }
            if (fv & DCGM_CLOCKS_THROTTLE_REASON_HW_SLOWDOWN)
            {
                printf("%d,%ld,%s,%lld\n", entityId, v->ts, "throttle_reason_hw_slowdown", DCGM_CLOCKS_THROTTLE_REASON_HW_SLOWDOWN);
            }
            if (fv & DCGM_CLOCKS_THROTTLE_REASON_SYNC_BOOST)
            {
                printf("%d,%ld,%s,%lld\n", entityId, v->ts, "throttle_reason_sync_boost", DCGM_CLOCKS_THROTTLE_REASON_SYNC_BOOST);
            }
            if (fv & DCGM_CLOCKS_THROTTLE_REASON_SW_THERMAL)
            {
                printf("%d,%ld,%s,%lld\n", entityId, v->ts, "throttle_reason_sw_thermal", DCGM_CLOCKS_THROTTLE_REASON_SW_THERMAL);
            }
            if (fv & DCGM_CLOCKS_THROTTLE_REASON_HW_THERMAL)
            {
                printf("%d,%ld,%s,%lld\n", entityId, v->ts, "throttle_reason_hw_thermal", DCGM_CLOCKS_THROTTLE_REASON_HW_THERMAL);
            }
            if (fv & DCGM_CLOCKS_THROTTLE_REASON_HW_POWER_BRAKE)
            {
                printf("%d,%ld,%s,%lld\n", entityId, v->ts, "throttle_reason_hw_power_brake", DCGM_CLOCKS_THROTTLE_REASON_HW_POWER_BRAKE);
            }
            if (fv & DCGM_CLOCKS_THROTTLE_REASON_DISPLAY_CLOCKS)
            {
                printf("%d,%ld,%s,%lld\n", entityId, v->ts, "throttle_reason_display_clocks", DCGM_CLOCKS_THROTTLE_REASON_HW_POWER_BRAKE);
            }

            // should be continue the loop to suppress current_clock_throttle_reasons?
            //continue;
        }

        fieldMeta = DcgmFieldGetById(v->fieldId);
        if (fieldMeta == NULL)
        {
            fprintf(stderr, "Failed to get field meta for field ID %d: %d\n", v->fieldId, result);
            return -1;
        }
        printf("%d,%ld,%s,", entityId, v->ts, fieldMeta->tag);
        switch (v->fieldType)
        {
        case DCGM_FT_DOUBLE:
            printf("%.02f\n", v->value.dbl);
            break;
        case DCGM_FT_INT64:
            printf("%ld\n", v->value.i64);
            break;
        case DCGM_FT_STRING:
            printf("%s\n", v->value.str);
            break;
        case DCGM_FT_TIMESTAMP:
            printf("%ld", v->value.i64);
            break;
        default:
            fprintf(stderr, "Uknown type %d\n", v->fieldType);
            return -1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    dcgmReturn_t result;
    dcgmHandle_t dcgmHandle = (dcgmHandle_t)NULL;

    Config config = {
        .interval = 1000,
        .duration = 1000000,
        .loops = 2,
        .field_count = 0};
    unsigned short defaultFields[] = {
        DCGM_FI_DEV_POWER_USAGE_INSTANT,
        DCGM_FI_DEV_CLOCK_THROTTLE_REASONS};
    memcpy(config.fields, defaultFields, sizeof(defaultFields));
    config.field_count = sizeof(defaultFields) / sizeof(defaultFields[0]);

    struct option long_options[] = {
        {"interval", required_argument, 0, 'i'},
        {"duration", required_argument, 0, 'd'},
        {"loops", required_argument, 0, 'l'},
        {"fields", required_argument, 0, 'f'},
        {0, 0, 0, 0}};

    int opt;
    int option_index = 0;
    while ((opt = getopt_long(argc, argv, "i:d:l:f:", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
        case 'i':
            config.interval = atoll(optarg);
            break;
        case 'd':
            config.duration = atoll(optarg);
            break;
        case 'l':
            config.loops = atoi(optarg);
            break;
        case 'f':
            parse_fields(optarg, config.fields, &config.field_count);
            break;
        default:
            fprintf(stderr, "Usage: %s [--interval interval] [--duration duration] [--loops loops] [--fields fields]\n", argv[0]);
            fprintf(stderr, "fileds can be find in https://github.com/NVIDIA/DCGM/blob/master/dcgmlib/dcgm_fields.h\n");
            exit(EXIT_FAILURE);
        }
    }

    // double ttl = (double)(config.loops * (config.duration / 1000000) + 1);
    print_config(&config);

    result = dcgmInit();

    if (result != DCGM_ST_OK)
    {
        fprintf(stderr, "Error init DCGM %d\n", result);
        exit(1);
    }

    result = dcgmConnect("127.0.0.1", &dcgmHandle);
    if (result != DCGM_ST_OK)
    {
        fprintf(stderr, "Error in dcgmConnect %d\n", result);
        exit(1);
    }

    dcgmGpuGrp_t gpuGroupId;
    result = dcgmGroupCreate(dcgmHandle, DCGM_GROUP_DEFAULT, "my gpu group", &gpuGroupId);
    if (result != DCGM_ST_OK)
    {
        fprintf(stderr, "Error in dcgmGroupCreate %d\n", result);
        exit(1);
    }
    // printf("group id %ld\n", gpuGroupId);

    dcgmFieldGrp_t fieldGroupId;
    result = dcgmFieldGroupCreate(dcgmHandle, config.field_count, config.fields, "interesting_fields", &fieldGroupId);
    if (result != DCGM_ST_OK)
    {
        fprintf(stderr, "Error in dcgmFieldGroupCreate %d\n", result);
        exit(1);
    }

    double ttl = 60.0; // keep the data for 60 seconds
    result = dcgmWatchFields(dcgmHandle, gpuGroupId, fieldGroupId, config.interval, config.ttl, 0);
    // Check result to see if DCGM operation was successful.
    if (result != DCGM_ST_OK)
    {
        fprintf(stderr, "Error in dcgmWatchFields %s\n", errorString(result));
        exit(1);
    }

    long long sinceTimestamp = 0;
    printf("GPUId,TimeStampUs,Field,Values\n");
    for (; config.loops > 0; config.loops--)
    {
        usleep(config.duration);
        result = dcgmGetValuesSince_v2(dcgmHandle, gpuGroupId, fieldGroupId, sinceTimestamp, &sinceTimestamp, myCallback, NULL);
        if (result != DCGM_ST_OK)
        {
            fprintf(stderr, "Error in dcgmGetValuesSince_v2 %d\n", result);
            exit(1);
        }
    }
    // another sleep to allow the callbacks to execute.
    usleep(config.duration);

    return 0;
}