#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libentstream.h>


// Output buffer
uint8_t outBuf[BUFLEN];

// Errors are sent to stderr.
// We return false for convenience.
bool printError(char *message) {
    fprintf(stderr, "%s\n", message);
    return false;
}

bool initInfnoise(infnoise_context *context, char *serial) {
    prepareOutputBuffer();
    // Initialize USB
    if (!initializeUSB(context, serial)) {
        // Sometimes we have to do it twice - not sure why
        if (!initializeUSB(context, serial)) {
            return false;
        }
    }
    return true;
}

void deinitInfnoise(infnoise_context *context) {
    ftdi_usb_close(&context->ftdic);
    ftdi_deinit(&context->ftdic);
}

void prepareOutputBuffer() {
    uint32_t i;
    // Set SW1EN and SW2EN alternately
    for (i = 0u; i < BUFLEN; i += 2) {
        // Alternate Ph1 and Ph2
        outBuf[i] = (1 << SWEN1);
        outBuf[i + 1] = (1 << SWEN2);
    }
}

// Extract the INM output from the data received. Basically, either COMP1 or COMP2
// changes, not both, so alternate reading bits from them.  We get 1 INM bit of output
// per byte read.
void extractBytes(uint8_t *bytes, uint8_t *inBuf) {
    uint32_t i;
    for (i = 0u; i < ENTLEN; i++) {
        uint32_t j;
        uint8_t byte = 0u;
        for (j = 0u; j < 8u; j++) {
            uint8_t val = inBuf[i * 8u + j];
            uint8_t evenBit = (val >> COMP2) & 1u;
            uint8_t oddBit = (val >> COMP1) & 1u;
            bool even = j & 1u; // Use the even bit if j is odd
            uint8_t bit = even ? evenBit : oddBit;
            byte = (byte << 1u) | bit;
        }
        bytes[i] = byte;
    }
}

bool isSuperUser(void) {
    return (geteuid() == 0);
}

int listDevices(infnoise_device **devices) {
    *devices = NULL;
    int r, i;
    struct ftdi_context ftdic;
    struct ftdi_device_list *list, *cursor;
    char serial[128];
    ftdi_init(&ftdic);
    r = ftdi_usb_find_all(&ftdic, &list, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID);
    if (r == 0) {
        fprintf(stderr, "No device found.\n");
        return 0;
    }
    if (r < 0) {
        fprintf(stderr, "Error: %d (%s)\n", r, ftdi_get_error_string(&ftdic));
        return 0;
    }
    i = 0;
    for (cursor = list; cursor != NULL; i++) {
       r = ftdi_usb_get_strings(&ftdic, cursor->dev, NULL, 0, NULL, 0, serial, 128);
       if (r < 0) {
            fprintf(stderr, "Can't check device: %d (%s)\n", r, ftdi_get_error_string(&ftdic));
            return 0;
        }
        infnoise_device *device = malloc(sizeof(infnoise_device));
        strcpy(device->serial, serial);
        device->next = *devices;
        *devices = device;
        cursor = cursor->next;
    }
    ftdi_list_free(&list);
    ftdi_deinit(&ftdic);
    return i;
}

// Initialize the Infinite Noise Multiplier USB interface.
bool initializeUSB(infnoise_context *context, char *serial) {
    ftdi_init(&context->ftdic);
    // Get first device if no serial
    if (serial == NULL) {
        infnoise_device *devices;
        int found = listDevices(&devices);
        if (found == 0) return false;
        serial = devices->serial;
    }
    // Copy serial number to context
    strcpy(context->serial, serial);
    // Connect
    if (ftdi_usb_open_desc(&context->ftdic, INFNOISE_VENDOR_ID, INFNOISE_PRODUCT_ID, NULL, serial) < 0) {
        if (!isSuperUser()) {
            return printError("Can't open Infinite Noise Multiplier. Try running as super user?");
        } else {
            return printError("Can't open Infinite Noise Multiplier.");
        }
    }
    // Set high baud rate
    switch (ftdi_set_baudrate(&context->ftdic, 30000)) {
    case -1:
        return printError("Invalid baud rate.");
    case -2:
        return printError("Setting baud rate failed.");
    case -3:
        return printError("Infinite Noise Multiplier unavailable.");
    default:
        break;
    }
    // Enable bit-bang mode
    switch (ftdi_set_bitmode(&context->ftdic, MASK, BITMODE_SYNCBB)) {
    case -1:
        return printError("Can't enable bit-bang mode.");
    case -2:
        return printError("Infinite Noise Multiplier unavailable.");
    default:
        break;
    }
    // Just test to see that we can write and read
    uint8_t buf[64u] = {0u,};
    if (ftdi_write_data(&context->ftdic, buf, sizeof(buf)) != sizeof(buf)) {
        return printError("USB write failed.");
    }
    if (ftdi_read_data(&context->ftdic, buf, sizeof(buf)) != sizeof(buf)) {
        return printError("USB read failed.");
    }
    return true;
}

bool readData(infnoise_context *context, uint8_t *result) {
    uint8_t inBuf[BUFLEN];
    // Write clock signal
    if (ftdi_write_data(&context->ftdic, outBuf, sizeof(outBuf)) != sizeof(outBuf)) {
        return printError("USB write failed.");
    }
    // Read 512 bytes from the internal buffer (in synchronous bitbang mode)
    if (ftdi_read_data(&context->ftdic, inBuf, sizeof(inBuf)) != sizeof(inBuf)) {
        return printError("USB read failed.");
    }
    extractBytes(result, inBuf);
    return true;
}
