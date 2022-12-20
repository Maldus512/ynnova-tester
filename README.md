# Ynnova Tester

## Notes

 - Required tests cannot be skipped
 - In front of a failed test you can either skip one or restart
 - One should save the outcome of the tests
 
I comandi per l'STlink x programmare il boot e l'applicazione dell'STM32 sono:
$ st-flash write data/20220926_boot.bin  0x8000000
$ st-flash write data/20221212_appl.bin 0x8008000