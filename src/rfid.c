// Co-programmed by Erik Forsberg and Elias Treutiger

#include <stdint.h>

#include "rfid.h"
#include "timer.h"

// ADDRESSES

// GPIO addresses, pins 0-31
#define GPIO_DATA ((volatile uint32_t*)0x040000e0)
#define GPIO_DIR ((volatile uint32_t*)0x040000e4)
// Use pin 0-5
#define GPIO_NSS (0)
#define GPIO_SCK (1)
#define GPIO_MOSI (2)
#define GPIO_MISO (3)
#define GPIO_RST (4)

// Set MOSI bit to value (0 or 1)
static void set_mosi_pin(uint8_t value) {
  if (value) {
    *GPIO_DATA |= (1 << GPIO_MOSI);
  } else {
    *GPIO_DATA &= ~(1 << GPIO_MOSI);
  }
}

// Read MISO bit
static uint8_t read_miso_pin(void) { return (*GPIO_DATA >> GPIO_MISO) & 1; }

// Set SCK pin to value (0 or 1)
static void set_sck_pin(uint8_t value) {
  if (value) {
    *GPIO_DATA |= (1 << GPIO_SCK);
  } else {
    *GPIO_DATA &= ~(1 << GPIO_SCK);
  }
}

// Set NSS pin to value (0 or 1)
// NSS is the chip select pin, active low
static void set_nss_pin(uint8_t value) {
  if (value) {
    *GPIO_DATA |= (1 << GPIO_NSS);
  } else {
    *GPIO_DATA &= ~(1 << GPIO_NSS);
  }
}

// Send 1 byte and receive 1 byte via SPI
static uint8_t spi_transfer(uint8_t byte_out) {
  uint8_t byte_in = 0;
  // For each bit in byte_out, send and receive corresponding bit, starting with
  // MSB
  for (int i = 7; i >= 0; i--) {
    // Set MOSI to relevant byte value
    set_mosi_pin((byte_out >> i) & 1);

    // Pulse clock high, prompting reader to sample MOSI
    set_sck_pin(1);

    // Read MISO
    if (read_miso_pin()) {
      byte_in |= (1 << i);
    }

    // Pulse clock low, prompting reader to change MISO
    set_sck_pin(0);
  }
  // Return the byte read from MISO
  return byte_in;
}

// Write an 8 bit value to a register
void rfid_write_reg(uint8_t reg, uint8_t value) {
  set_nss_pin(0);  // Select chip (active low)

  spi_transfer((reg << 1) & 0x7E);  // Send write address
  spi_transfer(value);              // Send byte value

  set_nss_pin(1);  // Deselect chip
}

uint8_t rfid_read_reg(uint8_t reg) {
  uint8_t value;

  set_nss_pin(0);  // Select chip

  spi_transfer(((reg << 1) & 0x7E) | 0x80);  // Send read address
  value = spi_transfer(0x00);  // Send dummy byte to clock out data

  set_nss_pin(1);  // Deselect chip

  return value;
}

// Initialize the RFID reader
void rfid_init(void) {
  // Set direction of MISO to input
  *GPIO_DIR &= ~(1 << GPIO_MISO);
  // Set direction of SCK, NSS, MOSI, RST to output
  *GPIO_DIR |=
      (1 << GPIO_SCK) | (1 << GPIO_NSS) | (1 << GPIO_MOSI) | (1 << GPIO_RST);
  // Drive high to Reset
  *GPIO_DATA |= (1 << GPIO_RST);
  // Set NSS high and SCK low
  *GPIO_DATA |= (1 << GPIO_NSS);
  *GPIO_DATA &= ~(1 << GPIO_SCK);

  delay(1);
  // Initialization procedure as specified in the datasheet
  // SoftReset routine 0xf
  rfid_write_reg(0x01, 0x0f);

  delay(1);

  // Timer auto
  rfid_write_reg(0x2a, 0x8d);
  rfid_write_reg(0x2B, 0xA9);  // TReloadReg Hi
  rfid_write_reg(0x2C, 0x03);  // TReloadReg Lo
  rfid_write_reg(0x2D, 0xE8);

  // Modulation & CRC Settings for ISO 14443A
  rfid_write_reg(0x15,
                 0x40);  // Enable 100% ASK

  rfid_write_reg(0x11, 0x3D);  // ModeReg: CRCPreset = 0x6363

  // Set Receiver Amplifier Gain to Maximum (48 dB)
  rfid_write_reg(0x26, 0x70);  // RFCfgReg: RxGain = 48 dB

  // Enable antennas
  rfid_write_reg(0x14, 0x83);
}

// Reads the 4-byte UID of a detected card.
// Returns 1 on valid UID & matching checksum, 0 on failure
// Places the UID bytes into uid_buffer
uint8_t rfid_anticoll(uint8_t* uid_buffer) {
  // Set reader to idle and cancel any executing commands
  rfid_write_reg(0x01, 0x00);

  // Flush FIFO
  rfid_write_reg(0x0A, 0x80);

  // Reset interrupts
  rfid_write_reg(0x04, 0x7F);
  rfid_write_reg(0x02, 0xA0);

  // Ensure correct frame alignment
  rfid_write_reg(0x0D, 0x00);

  // Load payload for anti-collision
  rfid_write_reg(0x09, 0x93);
  rfid_write_reg(0x09, 0x20);

  // Transceive, sending FIFO (REQA payload) data and listening for a reply.
  // Arms the engine for this, but doesnt actually start transmitting yet.
  rfid_write_reg(0x01, 0x0c);

  //  Starts the command.
  rfid_write_reg(0x0D, 0x80);

  // Wait for card response or timeout
  int timeout = 2000;
  while (timeout > 0) {
    uint8_t irq = rfid_read_reg(0x04);
    if (irq & 0x21) break;  // RxIRq or TimerIRq
    // Very short delay
    for (volatile int d = 0; d < 100; d++);
    timeout--;
  }

  // Stop transmission
  rfid_write_reg(0x0D, 0x00);

  // Check if 5 response bytes were received in FIFO
  // Fewer than 5 bytes and we have an incomplete response
  uint8_t bytes_in_fifo = rfid_read_reg(0x0A);
  if (timeout > 0 && bytes_in_fifo >= 5) {
    // Read 4 UID bytes
    for (int i = 0; i < 4; i++) {
      uid_buffer[i] = rfid_read_reg(0x09);
    }

    // Read 5th byte containing checksum
    uint8_t bcc = rfid_read_reg(0x09);

    // Calculate and compare checksum: UID[0] ^ UID[1] ^ UID[2] ^ UID[3] == BCC
    uint8_t calculated_bcc =
        uid_buffer[0] ^ uid_buffer[1] ^ uid_buffer[2] ^ uid_buffer[3];
    if (calculated_bcc == bcc) {
      return 1;  // ID is valid and verified.
    }
  }

  return 0;  // Failed to read UID or checksum error
}

// Request a card from the RFID reader
// Returns an ATQA response byte if a card is detected, 0 otherwise
uint8_t rfid_request(void) {
  // Set reader to idle and cancel any executing commands
  rfid_write_reg(0x01, 0x00);

  // Flushes FIFO removing any bytes left from earlier operations
  rfid_write_reg(0x0a, 0x80);

  // Clears old interrupt flags by writing 1s to ComIrqReg
  rfid_write_reg(0x04, 0x7F);

  // Load REQA payload in FIFO. REQA requests a card to respond with ATQA.
  rfid_write_reg(0x09, 0x26);

  // Transceive, sending FIFO (REQA payload) data and listening for a reply.
  // Arms the engine for this, but doesnt actually start transmitting yet.
  rfid_write_reg(0x01, 0x0c);

  //  Starts the command.
  rfid_write_reg(0x0D, 0x87);

  // Poll ComIrqReg until a response arrives, the reader's timer expires, or
  // this software timeout (about 2 seconds) runs out.
  int timeout = 2000;
  int received = 0;
  while (timeout > 0) {
    uint8_t irq = rfid_read_reg(0x04);

    if (irq & 0x20) {  // RxIRq: response data has arrived in the FIFO.
      received = 1;
      break;
    }
    if (irq & 0x01) {  // TimerIRq: the reader stopped waiting for a card.
      break;
    }
    delay(1);
    timeout--;
  }

  // Stop the transceive operation.
  rfid_write_reg(0x0D, 0x00);

  // If no response was received, return 0 to indicate failure.
  if (!received) {
    return 0;
  }

  // Read how many bytes are in FIFO from LevelReg. If none, return 0, else
  // return the first byte of the response.
  uint8_t bytes_in_fifo = rfid_read_reg(0x0A);
  if (bytes_in_fifo > 0) {
    return rfid_read_reg(0x09);
  }

  return 0;  // No response byte was received.
}
