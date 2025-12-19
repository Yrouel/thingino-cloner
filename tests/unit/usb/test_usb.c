#include "thingino.h"

int main() {
    printf("Testing USB initialization...\n");
    
    usb_manager_t manager;
    thingino_error_t result = usb_manager_init(&manager);
    if (result != THINGINO_SUCCESS) {
        printf("USB manager init failed: %s\n", thingino_error_to_string(result));
        return 1;
    }
    
    printf("USB manager initialized successfully\n");
    
    // Just try to get device list without processing
    device_info_t* devices;
    int count;
    result = usb_manager_find_devices(&manager, &devices, &count);
    if (result != THINGINO_SUCCESS) {
        printf("Device enumeration failed: %s\n", thingino_error_to_string(result));
        return 1;
    }
    
    printf("Found %d devices\n", count);
    
    if (devices) {
        free(devices);
    }
    
    usb_manager_cleanup(&manager);
    printf("Test completed successfully\n");
    return 0;
}