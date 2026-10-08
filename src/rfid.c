#include <stdint.h>

#include "rfid.h"
#include "timer.h"

extern void print(const char* str);
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

static uint8_t read_miso_pin(void) { return (*GPIO_DATA >> GPIO_MISO) & 1; }

static void set_sck_pin(uint8_t value) {
  if (value) {
    *GPIO_DATA |= (1 << GPIO_SCK);
  } else {
    *GPIO_DATA &= ~(1 << GPIO_SCK);
  }
}

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
  return byte_in;
}

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
                 0x40);  // TxASKReg: Force100ASK = 1 (Required for MIFARE cards

  rfid_write_reg(0x11, 0x3D);  // ModeReg: CRCPreset = 0x6363

  // Set Receiver Amplifier Gain to Maximum (48 dB)
  rfid_write_reg(0x26, 0x70);  // RFCfgReg: RxGain = 48 dB

  // Enable antennas
  rfid_write_reg(0x14, 0x83);
}

// Reads the 4-byte UID of a detected card.
// Returns 1 on success (UID valid & BCC matched), 0 on failure.
uint8_t rfid_anticoll(uint8_t* uid_buffer) {
  // 1. Reset CommandReg to Idle
  rfid_write_reg(0x01, 0x00);

  // 2. Flush FIFO buffer
  rfid_write_reg(0x0A, 0x80);

  // 3. Clear interrupts
  rfid_write_reg(0x04, 0x7F);
  rfid_write_reg(0x02, 0xA0);

  // 4. Ensure full 8-bit frame alignment (TxLastBits = 0)
  rfid_write_reg(0x0D, 0x00);

  // 5. Write Anti-Collision payload bytes into FIFO
  rfid_write_reg(0x09, 0x93);  // PICC_CMD_SEL_CL1
  rfid_write_reg(0x09, 0x20);  // NVB (2 bytes)

  // 6. Execute Transceive command
  rfid_write_reg(0x01, 0x0C);

  // 7. Start transmission
  rfid_write_reg(0x0D, 0x80);  // StartSend = 1

  // 8. Wait for card response or timeout
  int timeout = 2000;
  while (timeout > 0) {
    print("coll_read");
    uint8_t irq = rfid_read_reg(0x04);
    if (irq & 0x21) break;  // RxIRq or TimerIRq
    for (volatile int d = 0; d < 100; d++);
    timeout--;
  }

  // Stop transmission
  rfid_write_reg(0x0D, 0x00);

  // 9. Check if 5 response bytes were received in FIFO
  uint8_t bytes_in_fifo = rfid_read_reg(0x0A);
  if (timeout > 0 && bytes_in_fifo >= 5) {
    // Read 4 UID bytes
    for (int i = 0; i < 4; i++) {
      uid_buffer[i] = rfid_read_reg(0x09);
    }

    // Read 5th byte (BCC checksum)
    uint8_t bcc = rfid_read_reg(0x09);

    // 10. Verify BCC checksum: UID[0] ^ UID[1] ^ UID[2] ^ UID[3] == BCC
    uint8_t calculated_bcc =
        uid_buffer[0] ^ uid_buffer[1] ^ uid_buffer[2] ^ uid_buffer[3];
    if (calculated_bcc == bcc) {
      return 1;  // Success! UID is valid and verified.
    }
  }

  return 0;  // Failed to read UID or checksum error
}

uint8_t rfid_request(void) {
  // Set commandreg idle
  rfid_write_reg(0x01, 0x00);
  // Flush fifo
  rfid_write_reg(0x0a, 0x80);
  // Reset interrupt
  rfid_write_reg(0x04, 0x7F);

  // Enable Rx and Idle interrupt bits
  rfid_write_reg(0x02, 0xA0);

  // Request byte
  rfid_write_reg(0x09, 0x26);
  // Transceive command
  rfid_write_reg(0x01, 0x0c);
  // Set startsend and bit frame to 7
  rfid_write_reg(0x0D, 0x87);
  // Start transmission

  int timeout = 2000;
  while (timeout > 0) {
    uint8_t irq = rfid_read_reg(
        0x04);  // Bit 5 is RxIRq (Data received), Bit 0 is TimerIRq (Timeout)

    // Read success
    if (irq & 0x20) break;
    // Timeout
    if (irq & 0x21) break;
    delay(1);
    timeout--;
  }
  // Stop transmission phase
  rfid_write_reg(0x0D,
                 0x00);  // 7\. Check if data was received (FIFOLevelReg &gt; 0)
  uint8_t bytes_in_fifo = rfid_read_reg(0x0A);
  if (bytes_in_fifo >
      0) {  // Read the actual response byte from FIFODataReg (0x09)
    print("reached");
    return rfid_read_reg(0x09);
  }

  return 0x00;  // Return 0 if no card detected or timed out
}
