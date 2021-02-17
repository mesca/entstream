#include <libentstream.h>

// Default values
#define RATE 200.0
#define ENABLE_STDOUT 0
#define ENABLE_PUBSUB 0
#define FRONTEND_ENDPOINT "inproc://frontend"
#define BACKEND_ENDPOINT "ipc:///tmp/backend"
#define PUBLISHER_ENDPOINT ">inproc://frontend"
#define HWM 5000
#define SERIAL NULL;

typedef struct entstream_options {
    int enable_stdout;
    int enable_pubsub;
    char *endpoint;
    char *serial;
    float rate;
} entstream_options;

typedef struct entstream_context {
    infnoise_context device;
    zsock_t *pub;
} entstream_context;
