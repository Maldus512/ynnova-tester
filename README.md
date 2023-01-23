# Ynnova Tester

## TODO

 - (DONE) Buzzer control
 - (DONE) Add "wait for imei" state before "insert board" 
 - (DONE) Read board code and imei via QR code reader (keyboard or serial)
 - (DONE) Choose test unit based on board code
 - (DONE) Allow to change test unit name
 - (DONE) Allow to skip failed test
 - (DONE) Required tests cannot be skipped
 - (DONE) Add test codes and errors descriptions (prepare a couple of CSV files)
 - (DONE) Add test 60 - programming
 - (DONE) Before test 61 there must be the programming step
 - (DONE) Save the outcome of the tests when the test is reset

## Notes

I comandi per l'STlink x programmare il boot e l'applicazione dell'STM32 sono:
$ st-flash write data/20220926_boot.bin  0x8000000
$ st-flash write data/20221212_appl.bin 0x8008000