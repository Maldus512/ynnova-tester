#include <stdlib.h>
#include "error.h"

const char *error_to_string(int error) {
    switch (error) {
        case 1:
            return "Successo";
        case 100:
            return "Guasto Generico";
        case 101:
            return "";
        case 102:
            return "";
        case 103:
            return "";
        case 104:
            return "";
        case 105:
            return "";
        case 106:
            return "STEP 2: NO 8V TP101";
        case 107:
            return "STEP 2: NO 8V TP102";
        case 108:
            return "STEP 2: NO 8V TP103";
        case 109:
            return "STEP 2: NO 0V TP107";
        case 110:
            return "STEP 2: NO 0V TP109";
        case 111:
            return "STEP 2: NO 0V TP115";
        case 112:
            return "STEP 2: NO 8V TP116";
        case 113:
            return "STEP 2: NO 3,3V TP85";
        case 114:
            return "STEP 2: NO  0,8V TP111";
        case 115:
            return "STEP 3: NO 8V TP107";
        case 116:
            return "STEP 3: NO 7,4V TP109";
        case 117:
            return "STEP 3: NO 7,4V TP115";
        case 118:
            return "STEP 3: NO 0V TP116";
        case 119:
            return "STEP 3: NO 5,2V TP110";
        case 120:
            return "STEP 3: NO 3,3V TP113";
        case 121:
            return "STEP 3: NO 5,2V TP92";
        case 122:
            return "STEP 3: NO 0V TP93";
        case 123:
            return "STEP 3: NO 5,2V TP106";
        case 124:
            return "STEP 3: NO 3,3V TP112";
        case 125:
            return "STEP 3: NO 3,3V TP65";
        case 126:
            return "STEP 3: NO 3,8V TP68";
        case 127:
            return "STEP 3: NO 0V TP314";
        case 128:
            return "STEP 3: NO 0V TP66";
        case 129:
            return "STEP 3: NO 3,3V TP128";
        case 130:
            return "STEP 3: NO 0V TP124";
        case 131:
            return "STEP4: NO 3,3V TP314";
        case 132:
            return "STEP4: NO 3,3V TP124";
        case 133:
            return "STEP5: NO 3,3V TP124";
        case 134:
            return "STEP6: NO 24V TP101";
        case 135:
            return "STEP6: NO 24V TP102";
        case 136:
            return "STEP6: NO 24V TP103";
        case 137:
            return "STEP6: NO 24V TP107";
        case 138:
            return "STEP6:NO 11V TP111";
        case 139:
            return "STEP7: NO 0V TP332";
        case 140:
            return "STEP7: NO 1,8V TP144";
        case 141:
            return "STEP7: NO 0V TP326";
        case 142:
            return "STEP8: NO 24V TP122";
        case 143:
            return "STEP8: NO 0V TP122";
        case 144:
            return "STEP9: NO 0V TP62";
        case 145:
            return "STEP10: NO24V TP62";
        case 146:
            return "STEP11: NO 0V TP64";
        case 147:
            return "STEP12: NO 24V TP64";
        case 148:
            return "STEP13: NO 0V TP73";
        case 149:
            return "STEP14: NO 5V TP73";
        case 150:
            return "STEP15: NO 3V TP315";
        case 151:
            return "STEP15: NO 3V TP316";
        case 152:
            return "STEP15: NO 3V TP317";
        case 153:
            return "STEP15: NO 3V TP318";
        case 154:
            return "STEP16: NO 5V TP315";
        case 155:
            return "STEP16: NO 5V TP316";
        case 156:
            return "STEP16: NO 5V TP317";
        case 157:
            return "STEP16: NO 5V TP318";
        case 158:
            return "STEP17: NO 3,3V TP133";
        case 159:
            return "STEP18: NO 0V TP133";
        case 160:
            return "STEP19: NO 2,1V TP134";
        case 161:
            return "STEP20: NO 1,6V TP323";
        case 162:
            return "STEP20: NO 1,6V TP324";
        case 163:
            return "STEP20: TP323 DIVERSO DA TP324";
        case 164:
            return "STEP21: NO 24V TP79";
        case 165:
            return "STEP22: NO 0V TP79";
        case 166:
            return "STEP23: NO 12V TP79";
        case 167:
            return "STEP24: NO 24V TP125";
        case 168:
            return "STEP25: NO 0V TP125";
        case 169:
            return "STEP26: NO 12V TP125";
        case 170:
            return "STEP27: NO 0V TP34";
        case 171:
            return "STEP28: NO 2,1V TP34";
        case 172:
            return "STEP29: NO 1,1V TP34";
        case 173:
            return "STEP30: NO 0V TP34";
        case 174:
            return "STEP31: TP34 OLTRE 3.9V ";
        case 175:
            return "STEP31: TP321 OLTRE 4V ";
        case 176:
            return "STEP32: NO 0,48V TP34 A 4mA";
        case 177:
            return "STEP32: NO 1,2V TP34 A 10mA";
        case 178:
            return "STEP32: NO 2,4V TP34 A 20mA";
        case 179:
            return "STEP33: NO 0V TP35 A 0V";
        case 180:
            return "STEP33: NO 1,2V TP35 A 2,5V";
        case 181:
            return "STEP33: NO 2,5V TP35 A 5V";
        case 182:
            return "STEP34: NO 0V TP34 A 0V";
        case 183:
            return "STEP34: NO 1,2V TP34 A 5V";
        case 184:
            return "STEP34: NO 2,48V TP34 A 10V";
        case 185:
            return "STEP35: NO 0V TP41";
        case 186:
            return "STEP36: NO 2,1V TP41";
        case 187:
            return "STEP37: NO 1,1V TP41";
        case 188:
            return "STEP38: NO 0V TP41";
        case 189:
            return "STEP39: TP41 OLTRE 3.9V ";
        case 190:
            return "STEP39: TP321 OLTRE 4V ";
        case 191:
            return "STEP40: NO 0,48V TP41 A 4mA";
        case 192:
            return "STEP40: NO 1,2V TP41 A 10mA";
        case 193:
            return "STEP40: NO 2,4V TP41 A 20mA";
        case 194:
            return "STEP41: NO 0V TP35 A 0V";
        case 195:
            return "STEP41: NO 1,2V TP35 A 2,5V";
        case 196:
            return "STEP41: NO 2,5V TP35 A 5V";
        case 197:
            return "STEP42: NO 0V TP41 A 0V";
        case 198:
            return "STEP42: NO 1,2V TP41 A 5V";
        case 199:
            return "STEP42: NO 2,48V TP41 A 10V";
        case 200:
            return "TEST BOARD CANBUS FAIL";
        case 201:
            return "STEP48: CAN1 NON RICEVE";
        case 202:
            return "STEP48: CAN1 RICEVE CON ERRORI";
        case 203:
            return "STEP47: CAN2 NON RICEVE";
        case 204:
            return "STEP47: CAN2 RICEVE CON ERRORI";
        case 205:
            return "STEP50: CAN3 NON RICEVE";
        case 206:
            return "STEP50: CAN3 RICEVE CON ERRORI";
        case 207:
            return "STEP49: CAN4 NON RICEVE";
        case 208:
            return "STEP49: CAN4 RICEVE CON ERRORI";
        case 209:
            return "STEP51: UART5 NON RICEVE";
        case 210:
            return "STEP52: UART2 NON RICEVE";
        case 211:
            return "STEP53: UART DEBUG NON RICEVE";
        case 212:
            return "STEP55: RS485 NON TRASMETTE";
        case 213:
            return "STEP56: RS485 NON RICEVE";
        case 214:
            return "STEP57: ERRORE I2C RTC";
        case 215:
            return "STEP58: ERRORE I2C EEPROM";
        case 216:
            return "";
        case 217:
            return "";
        case 218:
            return "";
        case 219:
            return "";
        case 220:
            return "TEST43: FDI1 NO SEGNALE TP59";
        case 221:
            return "TEST44: FDI1 NO SEGNALE TP59";
        case 222:
            return "TEST45: FDI2 NO SEGNALE TP76";
        case 223:
            return "TEST46: FDI2 NO SEGNALE TP76";
        case 224:
            return "STEP59: NO 0V TP144";
        case 225:
            return "STEP59: NO 3,3V TP332";
        case 226:
            return "";
        case 227:
            return "";
        case 228:
            return "";
        case 229:
            return "";
        case 230:
            return "";
        case 231:
            return "";
        case 232:
            return "";
        case 233:
            return "";
        case 234:
            return "";
        case 235:
            return "";
        case 236:
            return "";
        case 237:
            return "";
        case 238:
            return "";
        case 239:
            return "";
        case 240:
            return "";
        case 241:
            return "";
        case 242:
            return "";
        case 243:
            return "";
        case 244:
            return "";
        case 245:
            return "";
        case 246:
            return "";
        case 247:
            return "";
        case 248:
            return "";
        case 249:
            return "";
        case 250:
            return "";
        default:
            return NULL;
    }
}
