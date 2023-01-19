# Ynnova Tester

## TODO

 - (DONE) Buzzer control
 - Add "wait for imei" state before "insert board" 
 - Read board code and imei via QR code reader (keyboard or serial)
 - (DONE) Choose test unit based on board code
 - (DONE) Allow to change test unit name
 - (DONE) Allow to skip failed test
 - (DONE) Required tests cannot be skipped
 - Add test codes and errors descriptions (prepare a couple of CSV files)
 - Before test 61 there must be the programming step
 - Save the outcome of the tests when the test is reset

## Notes

 - In front of a failed test you can either skip one or restart
 - One should save the outcome of the tests
 
I comandi per l'STlink x programmare il boot e l'applicazione dell'STM32 sono:
$ st-flash write data/20220926_boot.bin  0x8000000
$ st-flash write data/20221212_appl.bin 0x8008000