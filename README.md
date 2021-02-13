# RTL8710 SDK Pick / Padding / Checksum source code

This is a source code that replicates Pick(.exe), Padding(.exe) and Checksum(.exe) Intel32 binaries that are needed to run RTL8710, RTL8711 and RTL8195 GCC SDK 3.5

Ameba since released their own sourcecode in SDK 4.0b/ SDK 4.0c packages

In addition to pick, padding and checksum, I included SetMac writtern by sharikov.

This tool amends IP Address in the calibration sector.

Example:  
rtl8710-openocd: make dump  
make cutoutcalib  
./setmac -i calib.bin -o calib2.bin -m 12:34:56+0  
make restorecalib  

Add to the makefile:  
restorecalib:
        openocd -f interface/jlink.cfg -c "transport select swd" -f script/rtl8710.ocd -c "init" -c "reset halt" -c "rtl8710_flash_auto_erase 1" \
        -c "rtl8710_flash_auto_verify 1" -c "rtl8710_flash_write calib.bin 40960" -c shutdown

cutoutcalib:
        dd skip=$((0xa000)) count=4096 bs=1 if=dump.bin of=calib.bin
