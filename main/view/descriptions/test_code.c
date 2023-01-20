#include <stdlib.h>
#include "test_code.h"

const char *test_code_to_string(int test_code) {
    switch (test_code) {
        case 1:
            return "-";
        case 2:
            return "ALIMENTAZIONE 8V SENZA CHIAVE";
        case 3:
            return "ALIMENTAZIONE 8V CONSENSO CHIAVE";
        case 4:
            return "ACCENSIONE MODULO WIFI";
        case 5:
            return "VERIFICA AUTO-RITENUTA";
        case 6:
            return "ALIMENTAZIONE 24V";
        case 7:
            return "ACCENSIONE MODULO 4GG";
        case 8:
            return "VERIFICA VO-SENS";
        case 9:
            return "VERIFICA OC1";
        case 11:
            return "VERIFICA OC2";
        case 13:
            return "VERIFICA 5V USB";
        case 15:
            return "VERIFICA LED";
        case 17:
            return "VERIFICA RESET";
        case 19:
            return "VERIFICA ALIMENTAZIONE";
        case 20:
            return "VERIFICA SONDE TEMPERATURA";
        case 21:
            return "VERIFICA O1PP";
        case 24:
            return "VERIFICA O2PP";
        case 27:
            return "VERIFICA IN1";
        case 35:
            return "VERIFICA IN2";
        case 43:
            return "VERIFICA FDI1";
        case 45:
            return "VERIFICA FDI2";
        case 47:
            return "VERIFICA CAN1 E CAN2";
        case 49:
            return "VERIFICA CAN3 E CAN4";
        case 51:
            return "VERIFICA UART2 E UART5";
        case 53:
            return "VERIFICA UART5 E UART-DBG";
        case 55:
            return "VERIFICA 485";
        case 57:
            return "VERIFICA RTC";
        case 58:
            return "VERIFICA EEPROM";
        case 59:
            return "SPEGNIMENTO MODULO 4G";
        case 60:
            return "PROGRAMMAZIONE STM32";
        case 61:
            return "VERIFICA TENUTA SUPERCAP";
        case 62:
            return "RILASCIO AUTORITENUTA";
        default:
            return NULL;
    }
}
