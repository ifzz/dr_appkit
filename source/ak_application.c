
#include "..\include\easy_appkit\ak_application.h"
#include "..\include\easy_appkit\ak_build_config.h"
#include <easy_util/easy_util.h>
#include <stdlib.h>
#include <string.h>

struct ak_application
{
    /// The name of the application.
    char name[AK_MAX_APPLICATION_NAME_LENGTH];


    /// The size of the extra data, in bytes.
    size_t extraDataSize;

    /// A pointer to the extra data associated with the application.
    char pExtraData[1];
};


ak_application* ak_create_application(const char* pName, size_t extraDataSize, const void* pExtraData)
{
    ak_application* pApplication = malloc(sizeof(ak_application) + extraDataSize - sizeof(pApplication->pExtraData));
    if (pApplication != NULL)
    {
        if (pName != NULL) {
            strcpy_s(pApplication->name, sizeof(pApplication->name), pName);
        } else {
            strcpy_s(pApplication->name, sizeof(pApplication->name), "easy_appkit");
        }

        pApplication->extraDataSize = extraDataSize;

        if (extraDataSize > 0 && pExtraData != NULL) {
            memcpy(pApplication->pExtraData, pExtraData, extraDataSize);
        }
    }

    return pApplication;
}

void ak_delete_application(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return;
    }

    free(pApplication);
}


int ak_run_application(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return -1;
    }

    return 0;
}


const char* ak_get_application_name(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->name;
}


size_t ak_get_application_extra_data_size(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return 0;
    }

    return pApplication->extraDataSize;
}

void* ak_get_application_extra_data(ak_application* pApplication)
{
    if (pApplication == NULL) {
        return NULL;
    }

    return pApplication->pExtraData;
}
