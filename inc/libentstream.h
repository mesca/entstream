#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__FreeBSD__) || defined(__APPLE__)
#include <limits.h>
#else
#include <linux/limits.h>
#endif
#if defined(__OpenBSD__) || defined(__NetBSD__) || defined(__DragonFly__) || defined(__FreeBSD__)
#include <fcntl.h>
#endif
#include <ftdi.h>
#include <czmq.h>


  //////////////////
 //  PUBLIC API  //
//////////////////

typedef struct infnoise_context {
    struct ftdi_context ftdic;
    char serial[128];
} infnoise_context;

typedef struct infnoise_device {
    char serial[128];
    struct infnoise_device *next;
} infnoise_device;

int listDevices(infnoise_device **devices);
bool initInfnoise(infnoise_context *context, char *serial);
void deinitInfnoise(infnoise_context *context);
bool readData(infnoise_context *context, uint8_t *result);


  ///////////////////
 //  PRIVATE API  //
///////////////////

// The FT240X has a 512-byte buffer
#define BUFLEN 512u

// We get one bit of entropy per byte read
#define ENTLEN BUFLEN / 8u

// Device identifiers
#define INFNOISE_VENDOR_ID 0x0403
#define INFNOISE_PRODUCT_ID 0x6015

// Bit-bang mode
#define BITMODE_SYNCBB 0x4

// This defines which pins on the FT240X are used
#define COMP1 1u
#define COMP2 4u
#define SWEN1 2u
#define SWEN2 0u

// All data bus bits of the FT240X are outputs, except COMP1 and COMP2
#define MASK (0xffu & ~(1u << COMP1) & ~(1u << COMP2))

bool initializeUSB(infnoise_context *context, char *serial);
void prepareOutputBuffer();
void extractBytes(uint8_t *bytes, uint8_t *inBuf);
