#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h> // For usleep

// Protocol Constants [cite: 130, 306, 351]
#define START_FLAG         0xA5
#define CMD_STOP           0x25
#define CMD_RESET          0x40
#define CMD_SCAN           0x20
#define CMD_GET_HEALTH     0x52

#define EXPECTED_HEALTH_DESC_LEN 7
#define EXPECTED_SCAN_DESC_LEN   7

// --- Communication Helpers (To be implemented based on your OS/Hardware) ---
// You must provide functions to write and read bytes from your UART port.
extern void uart_write(uint8_t *data, uint16_t len);
extern int  uart_read(uint8_t *buffer, uint16_t len, uint32_t timeout_ms);

/**
 * Sends a request packet to the RPLIDAR[cite: 113, 114].
 */
void send_request(uint8_t command) {
    uint8_t packet[2] = {START_FLAG, command};
    uart_write(packet, 2);
}

/**
 * Validates the device health before scanning[cite: 683, 866].
 */
bool check_health() {
    printf("Checking RPLIDAR health...\n");
    send_request(CMD_GET_HEALTH);

    uint8_t descriptor[EXPECTED_HEALTH_DESC_LEN];
    if (uart_read(descriptor, EXPECTED_HEALTH_DESC_LEN, 100) != EXPECTED_HEALTH_DESC_LEN) {
        return false;
    }

    // Verify response descriptor for Single Response [cite: 187, 679]
    // Expected: A5 5A 03 00 00 00 06
    if (descriptor[0] != 0xA5 || descriptor[1] != 0x5A) return false;

    uint8_t health_data[3];
    uart_read(health_data, 3, 100);

    uint8_t status = health_data[0]; // [cite: 701]
    if (status == 0) {
        printf("Status: Good\n");
        return true;
    } else if (status == 1) {
        printf("Status: Warning (Error Code: %d)\n", health_data[1]);
        return true; 
    } else {
        printf("Status: Error. Hard reset required.\n");
        return false;
    }
}

/**
 * Starts the scanning operation and parses measurement samples[cite: 342, 356].
 */
void start_scanning() {
    send_request(CMD_SCAN);

    uint8_t descriptor[EXPECTED_SCAN_DESC_LEN];
    if (uart_read(descriptor, EXPECTED_SCAN_DESC_LEN, 500) == EXPECTED_SCAN_DESC_LEN) {
        // Verify Multiple Response descriptor [cite: 351]
        // Expected: A5 5A 05 00 00 40 81
        if (descriptor[0] == 0xA5 && descriptor[6] == 0x81) {
            printf("Scanning started. Waiting for stable rotation...\n");
            
            while (true) {
                uint8_t sample[5];
                if (uart_read(sample, 5, 1000) == 5) {
                    // Parse 5-byte measurement node [cite: 385, 391]
                    bool start_node = sample[0] & 0x01; // S bit
                    uint8_t quality = sample[0] >> 2;
                    
                    // Angle calculation: angle_q6 / 64.0 [cite: 413]
                    uint16_t angle_raw = ((uint16_t)sample[2] << 7) | (sample[1] >> 1);
                    float angle_deg = angle_raw / 64.0f;

                    // Distance calculation: distance_q2 / 4.0 
                    uint16_t dist_raw = ((uint16_t)sample[4] << 8) | sample[3];
                    float distance_mm = dist_raw / 4.0f;

                    if (dist_raw != 0) {
                        printf("[%s] Angle: %.2f deg, Distance: %.2f mm, Quality: %d\n", 
                               start_node ? "NEW SCAN" : "DATA", angle_deg, distance_mm, quality);
                    }
                }
            }
        }
    }
}

int main() {
    // 1. Reset Core to ensure clean state [cite: 327, 867]
    send_request(CMD_RESET);
    usleep(500000); // Wait 500ms for reboot 

    // 2. Check health [cite: 865]
    if (check_health()) {
        // 3. Start retrieval [cite: 901]
        start_scanning();
    }

    return 0;
}

// After this point it's specificly for a linux raspberry pi 5
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>

// Protocol Constants [cite: 306, 309, 349, 351]
#define START_FLAG         0xA5
#define CMD_STOP           0x25
#define CMD_RESET          0x40
#define CMD_SCAN           0x20
#define CMD_GET_HEALTH     0x52

int fd;

// Initialize UART on Raspberry Pi 5
bool init_uart(const char* port, int baud) {
    fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("Error opening serial port");
        return false;
    }

    struct termios tty;
    if (tcgetattr(fd, &tty) != 0) return false;

    cfsetospeed(&tty, baud);
    cfsetispeed(&tty, baud);

    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8; // 8-bit chars
    tty.c_iflag &= ~IGNBRK;                     // disable break processing
    tty.c_lflag = 0;                            // no signaling chars, no echo
    tty.c_oflag = 0;                            // no remapping, no delays
    tty.c_cc[VMIN]  = 0;                        // read doesn't block
    tty.c_cc[VTIME] = 5;                        // 0.5 seconds read timeout

    tty.c_cflag |= (CLOCAL | CREAD);            // ignore modem controls
    tty.c_cflag &= ~(PARENB | PARODD);          // shut off parity
    tty.c_cflag &= ~CSTOPB;
    tty.c_cflag &= ~CRTSCTS;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) return false;
    return true;
}

void send_request(uint8_t command) {
    uint8_t packet[2] = {START_FLAG, command}; [cite: 114, 130]
    write(fd, packet, 2);
}

int read_block(uint8_t* buffer, int len) {
    int total_read = 0;
    while (total_read < len) {
        int r = read(fd, buffer + total_read, len - total_read);
        if (r <= 0) break;
        total_read += r;
    }
    return total_read;
}

void process_scan() {
    send_request(CMD_SCAN); [cite: 342]
    
    uint8_t desc[7];
    if (read_block(desc, 7) == 7 && desc[0] == 0xA5 && desc[1] == 0x5A) { [cite: 183, 351]
        printf("Scan mode started. Parsing data...\n");
        while (1) {
            uint8_t node[5];
            if (read_block(node, 5) == 5) { [cite: 354, 385]
                // Byte 0: Quality and Start Bit [cite: 365, 367]
                bool start_bit = node[0] & 0x01; 
                uint8_t quality = node[0] >> 2; 

                // Byte 1-2: Angle [cite: 370, 375]
                // angle_q6[14:0] = (Byte2 << 7) | (Byte1 >> 1)
                uint16_t angle_q6 = ((uint16_t)node[2] << 7) | (node[1] >> 1);
                float angle_deg = angle_q6 / 64.0f; [cite: 413]

                // Byte 3-4: Distance [cite: 379, 383]
                uint16_t dist_q2 = ((uint16_t)node[4] << 8) | node[3];
                float distance_mm = dist_q2 / 4.0f; [cite: 414]

                if (dist_q2 > 0) {
                    printf("[%s] Angle: %.2f, Dist: %.2fmm, Quality: %d\n", 
                           start_bit ? "SYNC" : "DATA", angle_deg, distance_mm, quality);
                }
            }
        }
    }
}

int main() {
    // Default Raspberry Pi UART1 or UART0
    if (!init_uart("/dev/ttyAMA0", B115200)) return 1;

    printf("Resetting RPLIDAR...\n");
    send_request(CMD_RESET); [cite: 328]
    usleep(500000); // Must wait >= 500ms [cite: 333, 341]

    process_scan();

    close(fd);
    return 0;
}

//With the get health and the get infos 

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

// Protocol Commands [cite: 306]
#define CMD_STOP           0x25
#define CMD_RESET          0x40
#define CMD_SCAN           0x20
#define CMD_GET_INFO       0x50
#define CMD_GET_HEALTH     0x52

int fd;

// Initialize UART at 115200 (Common for C1)
bool init_serial(const char* port) {
    fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) return false;
    struct termios tty;
    tcgetattr(fd, &tty);
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_lflag = 0;
    tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0; tty.c_cc[VTIME] = 10; // 1s timeout
    return tcsetattr(fd, TCSANOW, &tty) == 0;
}

// Basic request sender [cite: 114, 130]
void send_cmd(uint8_t cmd) {
    uint8_t pkt[2] = {0xA5, cmd};
    write(fd, pkt, 2);
}

// 1. Get Device Info [cite: 607, 617]
void get_device_info() {
    send_cmd(CMD_GET_INFO);
    uint8_t desc[7], info[20];
    if (read(fd, desc, 7) == 7 && desc[0] == 0xA5) {
        read(fd, info, 20);
        // C1 MajorModel is 4 
        printf("Model ID: %d, Firmware: %d.%d\n", info[0], info[2], info[1]);
        printf("Serial: ");
        for(int i=0; i<16; i++) printf("%02X", info[4+i]);
        printf("\n");
    }
}

// 2. Get Health Status [cite: 671, 683]
bool check_device_health() {
    send_cmd(CMD_GET_HEALTH);
    uint8_t desc[7], health[3];
    if (read(fd, desc, 7) == 7 && desc[0] == 0xA5) {
        read(fd, health, 3);
        uint8_t status = health[0]; // 0:Good, 1:Warning, 2:Error [cite: 701]
        if (status == 2) {
            printf("Error detected! Code: %02X%02X\n", health[2], health[1]);
            return false;
        }
        printf("Health Status: %s\n", status == 0 ? "Good" : "Warning");
        return true;
    }
    return false;
}

// 3. Main Logic [cite: 865, 901]
int main() {
    if (!init_serial("/dev/ttyAMA0")) return 1;

    printf("Stopping & Resetting...\n");
    send_cmd(CMD_STOP);
    usleep(10000); // 10ms wait [cite: 315]
    send_cmd(CMD_RESET);
    usleep(500000); // 500ms wait [cite: 333]

    get_device_info();

    if (check_device_health()) {
        printf("Starting Scan...\n");
        send_cmd(CMD_SCAN);
        uint8_t desc[7];
        read(fd, desc, 7); // Consume descriptor [cite: 351]
        
        while (1) {
            uint8_t pkt[5];
            if (read(fd, pkt, 5) == 5) {
                float angle = (((uint16_t)pkt[2] << 7) | (pkt[1] >> 1)) / 64.0f; // [cite: 413]
                float dist = (((uint16_t)pkt[4] << 8) | pkt[3]) / 4.0f;          // [cite: 414]
                if (dist > 0) printf("Angle: %.2f deg, Dist: %.2f mm\n", angle, dist);
            }
        }
    }
    return 0;
}


// With the checksum

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#define CMD_STOP           0x25
#define CMD_RESET          0x40
#define CMD_SCAN           0x20
#define CMD_GET_INFO       0x50
#define CMD_GET_HEALTH     0x52

int fd;

bool init_serial(const char* port) {
    fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) return false;
    struct termios tty;
    tcgetattr(fd, &tty);
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
    tty.c_lflag = 0; tty.c_oflag = 0;
    tty.c_cc[VMIN] = 0; tty.c_cc[VTIME] = 10;
    return tcsetattr(fd, TCSANOW, &tty) == 0;
}

void send_cmd(uint8_t cmd) {
    uint8_t pkt[2] = {0xA5, cmd};
    write(fd, pkt, 2);
}

// Validates the integrity of a 5-byte standard scan packet
bool validate_scan_packet(uint8_t* pkt) {
    // Check 1: Inversed start flag bit (S_INV == !S) 
    uint8_t s_bit = pkt[0] & 0x01;
    uint8_t s_inv_bit = (pkt[0] >> 1) & 0x01;
    if (s_inv_bit == s_bit) return false;

    // Check 2: Check bit (C) must be 1 [cite: 405]
    if (!(pkt[1] & 0x01)) return false;

    return true;
}

void start_validated_scan() {
    printf("Starting Validated Scan...\n");
    send_cmd(CMD_SCAN);
    
    uint8_t desc[7];
    if (read(fd, desc, 7) != 7 || desc[0] != 0xA5 || desc[1] != 0x5A) {
        printf("Failed to receive valid response descriptor[cite: 183].\n");
        return;
    }

    while (1) {
        uint8_t pkt[5];
        if (read(fd, pkt, 5) == 5) {
            if (validate_scan_packet(pkt)) {
                // angle_q6 is 15 bits: Byte 1[7:1] and Byte 2[7:0] [cite: 371, 376]
                float angle = (((uint16_t)pkt[2] << 7) | (pkt[1] >> 1)) / 64.0f;
                // distance_q2 is 16 bits: Byte 3 and Byte 4 [cite: 380, 384]
                float dist = (((uint16_t)pkt[4] << 8) | pkt[3]) / 4.0f;
                uint8_t quality = pkt[0] >> 2; // [cite: 366]

                if (dist > 0) {
                    printf("Angle: %6.2f | Dist: %8.2f mm | Quality: %3d\n", angle, dist, quality);
                }
            } else {
                // If validation fails, we may be out of sync; 
                // in a production app, you would flush the buffer here.
                printf("Invalid packet detected, skipping...\n");
            }
        }
    }
}

int main() {
    if (!init_serial("/dev/ttyAMA0")) {
        perror("UART Init Failed");
        return 1;
    }

    send_cmd(CMD_STOP);
    usleep(10000); 
    send_cmd(CMD_RESET);
    usleep(500000); // Wait 500ms for reboot [cite: 333]

    start_validated_scan();

    return 0;
}