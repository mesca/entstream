#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <popt.h>
#include <clock.h>
#include <utils.h>
#include <timespec.h>
#include <entstream.h>


entstream_options options;
volatile sig_atomic_t stop;
struct timespec start;
double offset;
zactor_t *proxy;


void inthand() {
    stop = 1;
}

void init_broker() {
    zsys_handler_set(NULL); // Remove CZMQ signal handler
    proxy = zactor_new(zproxy, NULL);
    zstr_sendx(proxy, "FRONTEND", "XSUB", FRONTEND_ENDPOINT, NULL);
    zstr_sendx(proxy, "BACKEND", "XPUB", options.endpoint, NULL);
    // TODO: set high water mark
}

void terminate_broker() {
    zactor_destroy(&proxy);
}

void stream(entstream_context *context, struct timespec tick, uint8_t *data) {

    if (options.enable_stdout) {
        fwrite(data, 1, ENTLEN, stdout);
    }

    if (options.enable_pubsub) {

        size_t size = sizeof(double);

        // Monotonic timestamp struct to system-wide timestamp byte array
        uint8_t timestamp[size];
        double timestamp_double = timespec_to_double(tick) + offset;
        memcpy(&timestamp, &timestamp_double, size);

        // Hamming weight
        int setbits = count_set_bits(data, ENTLEN);

        // Z-score
        double z_double = z_score(setbits, ENTLEN);
        uint8_t z[size];
        memcpy(&z, &z_double, size);

        // Publish
        zsock_send(context->pub, "ssbbib", "entropy", context->device.serial, timestamp, size, data, ENTLEN, setbits, z, size);
    }

}

bool loop(char *serial) {

    // Initialize device
    entstream_context context;
    if (!initInfnoise(&context.device, serial)) return false;
    fprintf(stderr, "Connected: %s\n", context.device.serial);

    // Initialize publisher
    if (options.enable_pubsub) context.pub = zsock_new_pub(PUBLISHER_ENDPOINT);

    struct timespec interval, tick, end;
    double ellapsed;
    int missed, i;

    interval = timespec_from_double(1 / options.rate);
    tick = start;

    // Poll until an interrupt is captured
    while (!stop) {

        // Get random data
        uint8_t result[ENTLEN];
        if (!readData(&context.device, result)) return false;

        // Send data
        stream(&context, tick, result);

        // Mesure execution time
        clock_gettime(CLOCK_MONOTONIC, &end);
        ellapsed = timespec_to_double(timespec_sub(end, tick));
        double rate = 1 / ellapsed;
        missed = (int) (options.rate / rate) - 1;

        // Wait for next tick
        if (options.rate > 0) {
            if (missed > 0) {
                // We missed our deadline, schedule next tick accordingly
                for (i = 0; i <= missed; i++) {
                    tick = timespec_add(tick, interval);
                }
                fprintf(stderr, "%s\tCongestion: missed %d packet(s). Ellapsed: %lf. Current rate: %lf. Expected rate: %lf.\n", context.device.serial, missed, ellapsed, rate, options.rate);
            } else {
                // We're on time, schedule next tick
                tick = timespec_add(tick, interval);
            }
            // Use an absolute timer to prevent drift issues
            clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &tick, NULL);
        } else {
            // Do not wait, go as fast as possible
            clock_gettime(CLOCK_MONOTONIC, &tick);
            fprintf(stderr, "%s\tRate: %lf\n", context.device.serial, rate);
        }
    }

    // Deinitialize publisher
    if (options.enable_pubsub) zsock_destroy(&context.pub);

    deinitInfnoise(&context.device);
    return true;
}

void *worker(void *param) {
    if (!loop(param)) exit(1);
    return NULL;
}

void launch() {
    infnoise_device *devices;
    int found = listDevices(&devices);
    fprintf(stderr, "Found %d devices.\n", found);
    if (found == 0) return;
    pthread_t thread_ids[found];
    int i = 0;
    infnoise_device *cursor = devices;
    while (cursor != NULL) {
        pthread_create(&thread_ids[i], NULL, worker, cursor->serial);
        cursor = cursor->next;
        i++;
    }
    for (i = 0; i < found; i++) {
        pthread_join(thread_ids[i], NULL);
    }
}

int run() {

    // Interrupt gracefully
    signal(SIGINT, inthand);

    // Initialize the Pub/Sub broker
    if (options.enable_pubsub) init_broker();

    // Set a common starting point so all threads are in sync
    // Compute the offset between the system-wide clock and the monotonic clock
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &start);
    clock_gettime(CLOCK_REALTIME, &now);
    offset = timespec_to_double(timespec_sub(now, start));

    if (options.serial != NULL) {
        // Launch first device in main thread
        loop(options.serial);
    } else {
        // Launch as many threads as devices
        launch();
    }

    // Terminate the Pub/Sub broker
    if (options.enable_pubsub) terminate_broker();

    return 0;
}

int main(int argc, char *argv[]) {

    // Default options
    options.enable_stdout = ENABLE_STDOUT;
    options.enable_pubsub = ENABLE_PUBSUB;
    options.endpoint = BACKEND_ENDPOINT;
    options.serial = SERIAL;
    options.rate = RATE;

    // Parse arguments
    struct poptOption po[] = {
        {"enable-stdout", 'o', POPT_ARG_NONE, &options.enable_stdout, 0, "Print entropy to stdout", 0},
        {"enable-pubsub", 'p', POPT_ARG_NONE, &options.enable_pubsub, 0, "Send entropy to a Pub/Sub broker", 0},
        {"endpoint", 'e', POPT_ARG_STRING, &options.endpoint, 0, "Endpoint address", "ADDRESS"},
        {"serial", 's', POPT_ARG_STRING, &options.serial, 0, "If set, connect only to the specifed device", "SERIAL"},
        {"rate", 'r', POPT_ARG_FLOAT, &options.rate, 0, "If 0, will run as fast as possible", "RATE"},
        POPT_AUTOHELP
        POPT_TABLEEND
    };
    poptContext pc = poptGetContext(NULL, argc, (const char **)argv, po, 0);
    int r = poptGetNextOpt(pc);
    if (r != -1) {
        poptPrintUsage(pc, stderr, 0);
        return(1);
    }

    // Start acquisition
    return run();

}
