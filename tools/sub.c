// gcc -lzmq -lczmq -o sub sub.c

#include <czmq.h>

int main(void) {

    zsock_t *sub = zsock_new_sub("ipc:///tmp/backend", "entropy");
    //zsock_t *sub = zsock_new_sub("tcp://127.0.0.1:5000", "entropy");
    size_t size_double, size_data;
    char *topic, *serial;
    byte *timestamp_bytes, *z_bytes, *data;
    double timestamp, z;
    int setbits;

    while (!zsys_interrupted) {
        zsock_recv(sub, "ssbbib", &topic, &serial, &timestamp_bytes, &size_double, &data, &size_data, &setbits, &z_bytes, &size_double);
        memcpy(&timestamp, timestamp_bytes, sizeof(double));
        memcpy(&z, z_bytes, sizeof(double));
        printf("%s - %s - %lf - %s - %d - %lf\n", topic, serial, timestamp, data, setbits, z);
        free(topic);
        free(serial);
        free(timestamp_bytes);
        free(z_bytes);
        free(data);
    }

    zsock_destroy(&sub);

    return 0;
}