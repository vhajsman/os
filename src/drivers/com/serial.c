#include "serial.h"
#include "ioport.h"
#include "types.h"
#include "string.h"


int serial_initPort(u16 port) {
    outportb(port + SERIAL_OFFSET_IRQ_ENABLE, 0x00);    // Disable interrupts
    outportb(port + SERIAL_OFFSET_LINE_CONTROL, 0x80);  // Enable DLAB
    
    // Set max speed to 38400 bps
    outportb(port + SERIAL_OFFSET_DATA, 0x03);
    outportb(port + SERIAL_OFFSET_IRQ_ENABLE, 0x00);

    // Disable DLAB, set word length to 8 bits
    outportb(port + SERIAL_OFFSET_LINE_CONTROL, 0x03);

    outportb(port + SERIAL_OFFSET_FIFO_CONTROL, 0xC7);
    outportb(port + SERIAL_OFFSET_MODEM_CONTROL, 0x0B); 
    
    outportb(port + SERIAL_OFFSET_IRQ_ENABLE, 0x01);    // Enable interrupts


    // Check faulty
    outportb(port, 0xAE);
    if(inportb(port) != 0xAE) {
        return 1;
    }

    outportb(port + 4, 0x0F);
    return 0;
}

void serial_init() {
    serial_initPort(COM1);
    serial_initPort(COM2);
}

int serial_received(u16 port) {
    return inportb(port + SERIAL_OFFSET_DATA) & 1;
}

char serial_readByte(u16 port) {
    while(serial_received(port) == 0);
    return inportb(port + SERIAL_OFFSET_DATA);
}

int serial_available(u16 port) {
    return inportb(port + SERIAL_OFFSET_DATA) & 0x20;
}

void serial_writeByte(u16 port, char data) {
    outportb(port + SERIAL_OFFSET_DATA, data);
    // while(serial_available(port) == 0);
}

void serial_write(u16 port, const char* data) {
    for(size_t i = 0; i < strlen(data); i ++)
        serial_writeByte(port, data[i]);
}
