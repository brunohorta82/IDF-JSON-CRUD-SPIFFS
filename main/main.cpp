#include <esp_log.h>
#include "esp_log.h"
#include <string>
#include <sys/unistd.h>
#include <sys/stat.h>
// INCLUDE JSON LIB
#include "cJSON.h"
// SPIFFS
#include "esp_spiffs.h"
using namespace std;
#define TAG "SPIFF JSON"
cJSON *
createDevice(int id, string name, int address, string macAddress, int deviceType, string manufacture, string firmwareVersion, string hardwareVersion, int profileId, int endpoint)
{
    cJSON *device = cJSON_CreateObject();
    cJSON_AddNumberToObject(device, "id", id);
    cJSON_AddStringToObject(device, "name", name.c_str());
    cJSON_AddNumberToObject(device, "address", address);
    cJSON_AddStringToObject(device, "mac_addr", macAddress.c_str());
    cJSON_AddNumberToObject(device, "dev_type", deviceType);
    cJSON_AddStringToObject(device, "manufacture", manufacture.c_str());
    cJSON_AddStringToObject(device, "firmware_vs", firmwareVersion.c_str());
    cJSON_AddStringToObject(device, "hardware_vs", hardwareVersion.c_str());
    cJSON_AddNumberToObject(device, "profile_id", profileId);
    cJSON_AddNumberToObject(device, "endpoint", endpoint);
    return device;
}
extern "C" void app_main()
{
    // CONFIG SPIFFS
    esp_vfs_spiffs_conf_t config = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true,
    };
    esp_vfs_spiffs_register(&config);

    // CREATE BUFFER TO STORAGE FILE DATA

    cJSON *root = cJSON_CreateObject();
    cJSON *network = cJSON_AddObjectToObject(root, "network");
    cJSON_AddStringToObject(network, "extended_panId", "00:00:00:00:00:00:00:00");
    cJSON_AddNumberToObject(network, "panId", 0);
    cJSON_AddNumberToObject(network, "chanel", 0);
    cJSON *devices = cJSON_AddArrayToObject(root, "devices");
    // CRUD

    // CREATE
    for (int id = 0; id < 10; id++)
    {
        cJSON_AddItemToArray(devices, createDevice(id, "teste\0", id + 2, "00:00:00:00:00:00\0", id + 5, "factory\0", "1.00\0", "3.00\0", id + 2, id + 1));
    }
    ESP_LOGI("JSON CREATE", "%s", cJSON_Print(root));
    // READ
    for (int id = 0; id < cJSON_GetArraySize(devices); id++)
    {
        ESP_LOGI("JSON READ", "%s", cJSON_Print(cJSON_GetArrayItem(devices, id)));
    }

    // UPDATE
    for (int id = 0; id < cJSON_GetArraySize(devices); id++)
    {
        cJSON *device = cJSON_GetArrayItem(devices, id);
        if (cJSON_GetObjectItem(device, "id")->valueint == 4)
        {
            cJSON_ReplaceItemInObject(device, "name", cJSON_CreateString("novo Nome"));
            cJSON_ReplaceItemInObject(device, "endpoint", cJSON_CreateNumber(44));
            ESP_LOGI("JSON UPDATE", "%s", cJSON_Print(device));
        }
    }

    // DELETE
    for (int id = 0; id < cJSON_GetArraySize(devices); id++)
    {
        cJSON *device = cJSON_GetArrayItem(devices, id);
        if (cJSON_GetObjectItem(device, "id")->valueint == 9)
        {
            cJSON_DeleteItemFromArray(devices, id);
        }
    }
    ESP_LOGI("JSON DELETE", "%s", cJSON_Print(devices));

    // SAVE

    // convert the cJSON object to a JSON string
    char *json_str = cJSON_PrintUnformatted(root);
    // write the JSON string to a file
    FILE *fp = fopen("/spiffs/data.json", "w");
    fputs(json_str, fp);
    fclose(fp);
    // free the JSON string and cJSON object
    cJSON_free(json_str);
    cJSON_Delete(root);

    ESP_LOGI("STORAGE", "JSON SAVED");

    ESP_LOGI("STORAGE", "OPEN FILE");
    FILE *fpr = fopen("/spiffs/data.json", "r");
    struct stat st;
    if (stat("/spiffs/data.json", &st) == 0)
    {

        ESP_LOGI("STATS", "File size:%ld", st.st_size);
        char *buf = (char *)malloc(st.st_size);
        fread(buf, 1, st.st_size, fpr);
        cJSON *dataJson = cJSON_Parse(buf);
        ESP_LOGI("JSON", "%s", cJSON_Print(dataJson));
        free(buf);
    }
}