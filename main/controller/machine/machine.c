#include <poll.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "machine.h"
#include "serial.h"
#include "utils/system_time.h"
#include "utils/socketq.h"
#include "modbus.h"
#include "log.h"
#include "model/model.h"
#include "gel/timer/timecheck.h"


#define TIMEOUT              100
#define DELAY                30
#define REQUEST_SOCKET_PATH  "/tmp/.application_machine_request_socket"
#define RESPONSE_SOCKET_PATH "/tmp/.application_machine_response_socket"

#define MODBUS_RESPONSE_02_LEN(data_len) (5 + ((data_len % 8) == 0 ? (data_len / 8) : (data_len / 8) + 1))
#define MODBUS_RESPONSE_03_LEN(data_len) (5 + data_len * 2)
#define MODBUS_RESPONSE_04_LEN(data_len) (5 + data_len * 2)
#define MODBUS_RESPONSE_05_LEN           8
#define MODBUS_RESPONSE_15_LEN           8
#define MODBUS_RESPONSE_16_LEN           8
#define MODBUS_COMMUNICATION_ATTEMPTS    5

#define INPUT_REGISTER_LAST_EXECUTED_TEST 0
#define INPUT_REGISTER_TEST_STATE         1
#define INPUT_REGISTER_TEST_RESULT        2
#define INPUT_REGISTER_NEEDLE_BED_STATE   3
#define INPUT_REGISTER_BOARD_PRESENT      4

#define HOLDING_REGISTER_TEST 0

#define MINION_ADDRESS 1


typedef enum {
    MACHINE_MESSAGE_TAG_RESTART,
    MACHINE_MESSAGE_TAG_READ_STATUS,
    MACHINE_MESSAGE_TAG_WRITE_HOLDING_REGISTER,
} machine_message_tag_t;


typedef struct {
    machine_message_tag_t tag;

    union {
        struct {
            uint16_t register_index;
            uint16_t register_value;
        };
    };
} machine_message_t;


typedef struct {
    uint16_t start;
    void    *pointer;
} master_context_t;


static void *serial_port_task(void *args);
static int   read_with_timeout(uint8_t *buffer, size_t len, int fd, unsigned long timeout);
static int   read_input_registers(int fd, ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                  size_t count);
static int   read_holding_registers(int fd, ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                    uint16_t count);
static int   write_holding_registers(int fd, ModbusMaster *master, uint8_t address, uint16_t index, uint16_t *values,
                                     size_t len);
static void  send_write_holding_register(uint16_t index, uint16_t value);
static void  send_message(machine_message_t *message);
static int   send_request(int fd, ModbusMaster *master, size_t expected_len);
static int   task_manage_message(machine_message_t message, ModbusMaster *master, int fd);
static void  report_error(void);
static void  send_response(machine_response_message_t *message);


static socketq_t requestq  = {0};
static socketq_t responseq = {0};


void machine_init(void) {
    int res1 = socketq_init(&requestq, REQUEST_SOCKET_PATH, sizeof(machine_message_t));
    int res2 = socketq_init(&responseq, RESPONSE_SOCKET_PATH, sizeof(machine_response_message_t));
    assert(res1 == 0 && res2 == 0);

    pthread_t id;
    pthread_create(&id, NULL, serial_port_task, NULL);
    pthread_detach(id);
}


int machine_get_response(machine_response_message_t *msg) {
    return socketq_receive_nonblock(&responseq, (uint8_t *)msg, 0);
}


void machine_read_status(void) {
    machine_message_t message = {.tag = MACHINE_MESSAGE_TAG_READ_STATUS};
    send_message(&message);
}


void machine_restart_communication(void) {
    machine_message_t message = {.tag = MACHINE_MESSAGE_TAG_RESTART};
    send_message(&message);
}


void machine_start_test(uint16_t code) {
    send_write_holding_register(HOLDING_REGISTER_TEST, code);
}


void machine_reset_test(void) {
    send_write_holding_register(HOLDING_REGISTER_TEST, 0xFF);
}


static void send_write_holding_register(uint16_t index, uint16_t value) {
    machine_message_t message = {
        .tag            = MACHINE_MESSAGE_TAG_WRITE_HOLDING_REGISTER,
        .register_index = index,
        .register_value = value,
    };
    send_message(&message);
}


static void send_message(machine_message_t *message) {
    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, REQUEST_SOCKET_PATH);
    socketq_send(&requestq, (uint8_t *)message);
}


static void setup_port(int fd) {
    serial_set_interface_attribs(fd, B115200);
    serial_set_mincount(fd, 0);
    tcflush(fd, TCIFLUSH);
}


static int look_for_hardware_port() {
    char name[64];
    char types[] = "/dev/ttyUSB%i";
    int  fd;

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 20; j++) {
            snprintf(name, 64, types, j);
            if ((fd = serial_open_tty(name)) > 0) {
                log_info("Porta trovata: %s", name);
                return fd;
            }
        }
    }
    return -1;
}


static ModbusError data_callback(const ModbusMaster *master, const ModbusDataCallbackArgs *args) {
    master_context_t *ctx = modbusMasterGetUserPointer(master);
    // log_info("data cb %i %i %i", args->type, args->index, args->value);

    if (ctx != NULL) {
        switch (args->type) {
            case MODBUS_HOLDING_REGISTER: {
                uint16_t *buffer                 = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }

            case MODBUS_DISCRETE_INPUT: {
                uint8_t *buffer                  = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }

            case MODBUS_INPUT_REGISTER: {
                uint16_t *buffer                 = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }

            case MODBUS_COIL: {
                uint8_t *buffer                  = ctx->pointer;
                buffer[args->index - ctx->start] = args->value;
                break;
            }
        }
    }

    return MODBUS_OK;
}


static ModbusError masterExceptionCallback(const ModbusMaster *master, uint8_t address, uint8_t function,
                                           ModbusExceptionCode code) {
    (void)master;
    log_info("Received exception (function %d) from slave %d code %d\n", function, address, code);
    return MODBUS_OK;
}


static void *serial_port_task(void *args) {
    (void)args;
    int communication_error = 0;

    int fd = look_for_hardware_port();
    if (fd < 0) {
        log_warn("Nessuna porta trovata");
        communication_error = 1;
        report_error();
    }
    setup_port(fd);

    ModbusMaster    master;
    ModbusErrorInfo err = modbusMasterInit(&master,
                                           data_callback,               // Callback for handling incoming data
                                           masterExceptionCallback,     // Exception callback (optional)
                                           modbusDefaultAllocator,      // Memory allocator used to allocate request
                                           modbusMasterDefaultFunctions,        // Set of supported functions
                                           modbusMasterDefaultFunctionCount     // Number of supported functions
    );

    // Check for errors
    assert(modbusIsOk(err) && "modbusMasterInit() failed");
    machine_message_t not_delivered = {0};

    for (;;) {
        machine_message_t message = {0};

        if (socketq_receive(&requestq, (uint8_t *)&message)) {
            /* received a new message */
            if ((communication_error) && message.tag == MACHINE_MESSAGE_TAG_RESTART) {
                if (fd >= 0) {
                    close(fd);
                }

                fd = look_for_hardware_port();
                if (fd < 0) {
                    log_warn("Nessuna porta trovata");
                    communication_error = 1;
                    report_error();
                    continue;
                } else {
                    setup_port(fd);

                    if ((communication_error = task_manage_message(not_delivered, &master, fd))) {
                        report_error();
                        continue;
                    }
                }
            } else if (!(communication_error)) {
                if ((communication_error = task_manage_message(message, &master, fd))) {
                    not_delivered = message;
                    report_error();
                }
            }
        }

        usleep(DELAY * 1000);
    }

    close(fd);
    pthread_exit(NULL);
    return NULL;
}


static void send_response(machine_response_message_t *message) {
    struct sockaddr_un remote;
    remote.sun_family = AF_UNIX;
    strcpy(remote.sun_path, RESPONSE_SOCKET_PATH);
    socketq_send(&responseq, (uint8_t *)message);
}


static void report_error(void) {
    log_warn("Communication error!");
    machine_response_message_t message = {.tag = MACHINE_RESPONSE_MESSAGE_TAG_ERROR};
    send_response(&message);
}


static int task_manage_message(machine_message_t message, ModbusMaster *master, int fd) {
    int res = 0;
    modbusMasterSetUserPointer(master, NULL);

    switch (message.tag) {
        case MACHINE_MESSAGE_TAG_WRITE_HOLDING_REGISTER:
            res =
                write_holding_registers(fd, master, MINION_ADDRESS, message.register_index, &message.register_value, 1);
            break;

        case MACHINE_MESSAGE_TAG_READ_STATUS: {
            machine_response_message_t response     = {0};
            uint16_t                   registers[5] = {0};
            res = read_input_registers(fd, master, registers, MINION_ADDRESS, INPUT_REGISTER_LAST_EXECUTED_TEST, 5);
            if (res == 0) {
                response.tag                = MACHINE_RESPONSE_MESSAGE_TAG_STATUS;
                response.last_executed_test = registers[0];

                // log_info("%i %i %i %i %i", registers[0], registers[1], registers[2], registers[3], registers[4]);

                if (registers[3] && registers[4]) {
                    response.board_state = BOARD_STATE_READY;
                } else if (registers[4]) {
                    response.board_state = BOARD_STATE_PRESENT_OPEN;
                } else {
                    response.board_state = BOARD_STATE_ABSENT;
                }

                switch (registers[1]) {
                    case 0:
                        response.test_state = TEST_STATE_STARTING;
                        break;
                    case 1:
                        response.test_state = TEST_STATE_IN_PROGRESS;
                        break;
                    default:
                        response.test_state = TEST_STATE_DONE;
                        break;
                }

                response.test_result = registers[2];

                response.board_state = BOARD_STATE_READY;

                send_response(&response);
            }
            break;
        }

        case MACHINE_MESSAGE_TAG_RESTART:
            break;
    }

    return res;
}


static int read_input_registers(int fd, ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                size_t count) {
    int    res     = 0;
    size_t counter = 0;

    master_context_t ctx = {.pointer = registers, .start = start};
    if (registers == NULL) {
        modbusMasterSetUserPointer(master, NULL);
    } else {
        modbusMasterSetUserPointer(master, &ctx);
    }


    do {
        res                 = 0;
        ModbusErrorInfo err = modbusBuildRequest04RTU(master, address, start, count);
        assert(modbusIsOk(err));

        if (send_request(fd, master, MODBUS_RESPONSE_04_LEN(count))) {
            res = 1;
            usleep(DELAY * 1000);
        }
    } while (res && ++counter < MODBUS_COMMUNICATION_ATTEMPTS);

    if (res) {
        log_warn("Unable to read inputs");
    }

    return res;
}


static int read_holding_registers(int fd, ModbusMaster *master, uint16_t *registers, uint8_t address, uint16_t start,
                                  uint16_t count) {
    ModbusErrorInfo err;
    int             res     = 0;
    size_t          counter = 0;

    master_context_t ctx = {.pointer = registers, .start = start};
    if (registers == NULL) {
        modbusMasterSetUserPointer(master, NULL);
    } else {
        modbusMasterSetUserPointer(master, &ctx);
    }

    do {
        res = 0;
        err = modbusBuildRequest03RTU(master, address, start, count);
        assert(modbusIsOk(err));

        if (send_request(fd, master, MODBUS_RESPONSE_03_LEN(count))) {
            res = 1;
            usleep(DELAY * 1000);
        }
    } while (res && ++counter < MODBUS_COMMUNICATION_ATTEMPTS);

    if (res) {
        log_warn("Unable to read holding registers");
    }

    return res;
}


static int write_holding_registers(int fd, ModbusMaster *master, uint8_t address, uint16_t index, uint16_t *values,
                                   size_t len) {
    int    res     = 0;
    size_t counter = 0;

    do {
        res                 = 0;
        ModbusErrorInfo err = modbusBuildRequest16RTU(master, address, index, len, values);
        assert(modbusIsOk(err));

        if (send_request(fd, master, MODBUS_RESPONSE_16_LEN)) {
            res = 1;
            usleep(DELAY * 1000);
        }
    } while (res && ++counter < MODBUS_COMMUNICATION_ATTEMPTS);

    if (res) {
        log_warn("Unable to write holding registers");
    }

    return res;
}


static int read_with_timeout(uint8_t *buffer, size_t len, int fd, unsigned long timeout) {
    size_t        buffer_index = 0;
    unsigned long startts      = get_millis();

    do {
        int res = read(fd, &buffer[buffer_index], len - buffer_index);

        if (res > 0) {
            buffer_index += res;
        }
    } while (buffer_index != len && !is_expired(startts, get_millis(), timeout));

    return buffer_index;
}


static int send_request(int fd, ModbusMaster *master, size_t expected_len) {
    uint8_t buffer[expected_len];
    tcflush(fd, TCIFLUSH);
    memset(buffer, 0, expected_len);
    int tosend = modbusMasterGetRequestLength(master);

    if (write(fd, modbusMasterGetRequest(master), tosend) != tosend) {
        log_error("Unable to write to serial: %s", strerror(errno));
        return -1;
    }

    int             len = read_with_timeout(buffer, sizeof(buffer), fd, TIMEOUT);
    ModbusErrorInfo err = modbusParseResponseRTU(master, modbusMasterGetRequest(master),
                                                 modbusMasterGetRequestLength(master), buffer, len);

    if (!modbusIsOk(err)) {
        log_warn("Modbus error: %i %i (%i)", err.source, err.error, len);
        return 1;
    } else {
        return 0;
    }
}