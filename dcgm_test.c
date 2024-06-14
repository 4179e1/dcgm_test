#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <dlfcn.h>

#include "dcgm_agent.h"
#include "dcgm_structs.h"
#include "dcgm_errors.h"


int myCallback(dcgm_field_entity_group_t entityGroupId, dcgm_field_eid_t entityId, dcgmFieldValue_v1 *values, int numValues, void *userData)
{
    for (int i = 0; i < numValues; i++)
    {
        dcgmFieldValue_v1 *v = &values[i];
        assert(v->fieldType == DCGM_FT_DOUBLE);
        printf("%d,%.02f\n", entityId, v->value.dbl);
    }
    return 0;
}

int main(int argc, char *argv[])
{
    dcgmReturn_t result;
    dcgmHandle_t dcgmHandle = (dcgmHandle_t)NULL;

    // Frequency of DCGM sampling
    // DCGM do not make sure it's able to meet the frequency.
    long long loopIntervalUsec = 1000;    // 1 miliseconds
    // The frequency we are going to pull the data
    long long loopDurationUsec = 1000000; // 1 second
    // number of loops
    int loops = 2;
    if (argc >= 2) {
        loopIntervalUsec = strtoll(argv[1], NULL, 10);
        if (argc >= 3) {
            loopDurationUsec = strtoll(argv[2], NULL, 10);
            if (argc >= 4) {
                loops = strtol(argv[3], NULL, 10);
            }
        }
    }
    fprintf(stderr, "loopIntervalUsec %lld, loopDurationUsec %lld, loops %d\n", loopIntervalUsec, loopDurationUsec, loops);

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
    result = dcgmGroupCreate(dcgmHandle, DCGM_GROUP_DEFAULT , "my gpu group", &gpuGroupId);
    if (result != DCGM_ST_OK)
    {
        fprintf(stderr, "Error in dcgmGroupCreate %d\n", result);
        exit(1);
    }
    //printf("group id %ld\n", gpuGroupId);

    dcgmFieldGrp_t fieldGroupId;
    unsigned short fieldIds[] = {
        //DCGM_FI_DEV_POWER_USAGE,
        DCGM_FI_DEV_POWER_USAGE_INSTANT
    };
    int numC2cFieldIds = sizeof(fieldIds) / sizeof(fieldIds[0]);

    result = dcgmFieldGroupCreate(dcgmHandle, numC2cFieldIds, fieldIds, "interesting_fields", &fieldGroupId);
    if (result != DCGM_ST_OK)
    {
        fprintf(stderr, "Error in dcgmFieldGroupCreate %d\n", result);
        exit(1);
    }
    //printf("Field group id %ld\n", fieldGroupId);

    double ttl = (double)(loops * (loopDurationUsec / 1000000) + 1);
    result = dcgmWatchFields(dcgmHandle, gpuGroupId, fieldGroupId, loopIntervalUsec, ttl, 0);
    // Check result to see if DCGM operation was successful.
    if (result != DCGM_ST_OK)
    {
        fprintf(stderr, "Error in dcgmWatchFields %s\n", errorString(result));
        exit(1);
    }

    long long sinceTimestamp = 0;
    printf("GPUId,Watts\n");
    for (;loops > 0; loops--)
    {
        usleep(loopDurationUsec);
        result = dcgmGetValuesSince_v2(dcgmHandle, gpuGroupId, fieldGroupId, sinceTimestamp, &sinceTimestamp, myCallback, NULL);
        if (result != DCGM_ST_OK)
        {
            fprintf(stderr, "Error in dcgmGetValuesSince_v2 %d\n", result);
            exit(1);
        }
    }
    // another sleep to allow the callbacks to execute. 
    usleep(loopDurationUsec);

    return 0;
}