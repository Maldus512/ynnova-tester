#include <string.h>
#include <errno.h>
#include <sys/wait.h>
#include <assert.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "utils/socketq.h"
#include "stflash.h"
#include "log.h"


#define RESPONSE_SOCKET_PATH "/tmp/.application_stflash_response_socket"


static socketq_t responseq = {0};
static pid_t     pid       = 0;



void stflash_init(void) {
    int res = socketq_init(&responseq, RESPONSE_SOCKET_PATH, sizeof(stflash_response_t));
    assert(res == 0);
}


void stflash_run(void) {
    log_info("Programming");
    if ((pid = fork()) == 0) {
        // First child

        pid_t second_pid = 0;
        if ((second_pid = fork()) == 0) {
            // Second child

#ifdef ERASE
            pid_t third_pid = 0;
            if ((third_pid = fork()) == 0) {
                // Third child
                log_info("erasing");
                execl("./data/st-flash", "st-flash", "erase", NULL);
            } else {
                // Second (as) parent
                int status = 0;
                if (waitpid(third_pid, &status, 0) > 0) {
#endif
                    log_info("Bootloader");
                    sleep(2);
                    execl("./data/st-flash", "st-flash", "write", "./data/boot.bin", "0x8000000", NULL);
                    exit(1);
#ifdef ERASE
                }
            }
#endif
        } else {
            // First (as) parent
            int status = 0;
            if (waitpid(second_pid, &status, 0) > 0) {
                if (status == 0) {
                    log_info("Application");
                    // Second child finished successfully
                    sleep(2);
                    execl("./data/st-flash", "st-flash", "write", "./data/appl.bin", "0x8008000", NULL);
                } else {
                    log_warn("Process terminating with code %i", status);
                }
                exit(1);     // If we are here exec failed
            }
        }
        // stflash_response_t message = STFLASH_RESPONSE_OK;
        // socketq_send(&responseq, (uint8_t *)&message);
        exit(1);
    } else {
        // Parent, check status in a while
        log_info("Started flashing process with pid %i", pid);
    }
}


uint8_t stflash_get_response(stflash_response_t *response) {
    int status = 0;
    if (waitpid(pid, &status, WNOHANG) > 0) {
        log_info("Flashed from pid %i with result %i", pid, status);
        *response = status == 0 ? STFLASH_RESPONSE_OK : STFLASH_RESPONSE_FAIL;
        return 1;
    } else {
        return 0;
    }
    // return socketq_receive_nonblock(&responseq, (uint8_t *)response, 0);
}