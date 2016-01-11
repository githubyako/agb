#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <inttypes.h>
#include "XLoBorg.h"

///// Storage /////
uint8_t bufIn[BUFFER_MAX];
uint8_t bufOut[BUFFER_MAX];
char pathI2C[20];
int hI2C = HANDLE_UNINITALISED;

///// Public values /////
int busNumber = 1;
int addressAccelerometer = 0x1C;
int addressCompass = 0x0E;
int foundAccelerometer = FALSE;
int foundCompass = FALSE;
int diagnosticPrint = TRUE;
float gPerCount = 0.0f;
float tempOffest = 0.0f;

///// Macros /////
#define DIAG_PRINT(...)         do { if (diagnosticPrint) { printf(__VA_ARGS__); } } while (FALSE)


///// Private prototypes /////
void XLoInitAccelerometer(void);
void XLoInitCompass(void);
int XLoSetTargetI2C(int hI2C, int targetAddress);
int XLoSendI2C(int hI2C, int bytes, uint8_t *pData);
int XLoRecvI2C(int hI2C, int bytes, uint8_t *pData);

///// Public functions /////
int XLoInit(void) {
    return XLoInitialise(TRUE);
}

int XLoInitialise(int tryOtherBus) {
    int i;

    DIAG_PRINT("Loading XLoBorg on bus %d\n", busNumber);

    // Clean buffers
    for (i = 0; i < BUFFER_MAX; ++i) {
        bufIn[i] = 0xCC;
        bufOut[i] = 0xCC;
    }

    // Open I2C device
    snprintf(pathI2C, sizeof(pathI2C)-1, "/dev/i2c-%d", busNumber);
    hI2C = open(pathI2C, O_RDWR);
    if (hI2C < 0) {
        DIAG_PRINT("I2C ERROR: Failed to open bus %d, are we root?\n", busNumber);
        return I2C_ERROR_FAILED;
    }

    // Check for accelerometer
    XLoSetTargetI2C(hI2C, addressAccelerometer);
    if (XLoRecvI2C(hI2C, 1, bufIn) == I2C_ERROR_OK) {
        foundAccelerometer = TRUE;
    } else {
        foundAccelerometer = FALSE;
    }

    // Check for compass
    XLoSetTargetI2C(hI2C, addressCompass);
    if (XLoRecvI2C(hI2C, 1, bufIn) == I2C_ERROR_OK) {
        foundCompass = TRUE;
    } else {
        foundCompass = FALSE;
    }

    // See if we are missing chips
    if (!foundAccelerometer && !foundCompass) {
        DIAG_PRINT("Both the compass and accelerometer were not found\n");
        if (tryOtherBus) {
            if (busNumber == 1) {
                busNumber = 0;
            } else {
                busNumber = 1;
            }
            DIAG_PRINT("Trying bus %d instead\n", busNumber);
            return XLoInitialise(FALSE);
        } else {
            DIAG_PRINT("Are you sure your XLoBorg is properly attached, and the I2C drivers are running?\n");
            XLoShutdown();
            return I2C_ERROR_FAILED;
        }
    } else {
        DIAG_PRINT("XLoBorg loaded on bus %d\n", busNumber);
        if (foundAccelerometer) {
            XLoInitAccelerometer();
        }
        if (foundCompass) {
            XLoInitCompass();
        }
        if (foundAccelerometer && foundCompass) {
            return I2C_ERROR_OK;
        } else {
            return I2C_ERROR_PARTIAL;
        }
    }
}

void XLoReadAccelerometer(float* pdata) {
    int8_t x = 0;
    int8_t y = 0;
    int8_t z = 0;

    // Read the data from the accelerometer chip
    XLoSetTargetI2C(hI2C, addressAccelerometer);
    bufOut[0] = 0x00;
    if (XLoSendI2C(hI2C, 1, bufOut) == I2C_ERROR_OK) {
        if (XLoRecvI2C(hI2C, 4, bufIn) == I2C_ERROR_OK) {
            // Convert from unsigned to correctly signed values
            *((uint8_t*)&x) = bufIn[1];
            *((uint8_t*)&y) = bufIn[2];
            *((uint8_t*)&z) = bufIn[3];
        }
    }

    // Convert to Gs
    if (pdata != NULL) {
        pdata[0] = (float)x * gPerCount;
        pdata[1] = (float)y * gPerCount;
        pdata[2] = (float)z * gPerCount;
    }
}

void XLoReadCompassRaw(float* pdata) {
    int16_t x;
    int16_t y;
    int16_t z;

    // Read the data from the compass chip
    XLoSetTargetI2C(hI2C, addressCompass);
    bufOut[0] = 0x00;
    if (XLoSendI2C(hI2C, 1, bufOut) == I2C_ERROR_OK) {
        if (XLoRecvI2C(hI2C, 18, bufIn) == I2C_ERROR_OK) {
            // Convert from unsigned to correctly signed values
            *((uint16_t*)&x) = ((uint16_t)bufIn[1] << 8) | (uint16_t)bufIn[2];
            *((uint16_t*)&y) = ((uint16_t)bufIn[3] << 8) | (uint16_t)bufIn[4];
            *((uint16_t*)&z) = ((uint16_t)bufIn[5] << 8) | (uint16_t)bufIn[6];
        }
    }

    // Package data to caller
    if (pdata != NULL) {
        pdata[0] = (float)x;
        pdata[1] = (float)y;
        pdata[2] = (float)z;
    }
}

float XLoReadTemperature(void) {
    int8_t temp;

    // Read the data from the compass chip
    XLoSetTargetI2C(hI2C, addressCompass);
    bufOut[0] = 0x00;
    if (XLoSendI2C(hI2C, 1, bufOut) == I2C_ERROR_OK) {
        if (XLoRecvI2C(hI2C, 18, bufIn) == I2C_ERROR_OK) {
            // Convert from unsigned to correctly signed values
            *((uint8_t*)&temp) = bufIn[15];
        }
    }

    return ((float)temp + tempOffest);
}

void XLoShutdown(void) {
    if (hI2C > -1) {
        close(hI2C);
    }
    hI2C = HANDLE_UNINITALISED;
}

///// Private functions /////
void XLoInitAccelerometer(void) {
    XLoSetTargetI2C(hI2C, addressAccelerometer);

    // Setup mode configuration
    bufOut[0] = 0x2A; // CTRL_REG1 
    bufOut[1] = (0 << 6) | (0 << 4) | (0 << 2) | (1 << 1) | (1 << 0);
                // Sleep rate 50 Hz
                // Data rate 800 Hz
                // No reduced noise mode
                // Normal read mode
                // Active
    if (XLoSendI2C(hI2C, 2, bufOut)) {
        DIAG_PRINT("Failed sending CTRL_REG1!\n");
    }

    // Setup range
    bufOut[0] = 0x0E; // XYZ_DATA_CFG
    bufOut[1] = 0x00; // Range 2G, no high pass filtering
    if (XLoSendI2C(hI2C, 2, bufOut)) {
        DIAG_PRINT("Failed sending XYZ_DATA_CFG!\n");
    }
    gPerCount = 2.0f / 128.0f; // 2G over 128 counts

    // System state
    bufOut[0] = 0x0B; // SYSMOD
    bufOut[1] = 0x01; // Wake mode
    if (XLoSendI2C(hI2C, 2, bufOut)) {
        DIAG_PRINT("Failed sending SYSMOD!\n");
    }

    // Reset ready for reading
    bufOut[0] = 0x00;
    if (XLoSendI2C(hI2C, 1, bufOut)) {
        DIAG_PRINT("Failed sending final write!\n");
    }
}

void XLoInitCompass(void) {
    XLoSetTargetI2C(hI2C, addressCompass);

    // Acquisition mode
    bufOut[0] = 0x11; // CTRL_REG2
    bufOut[1] = (1 << 7) | (1 << 5) | (0 << 5);
                // Reset before each acquisition
                // Raw mode, do not apply user offsets
                // Disable reset cycle
    if (XLoSendI2C(hI2C, 2, bufOut)) {
        DIAG_PRINT("Failed sending CTRL_REG2!\n");
    }

    // System operation
    bufOut[0] = 0x10; // CTRL_REG1
    bufOut[1] = (0 << 5) | (3 << 3) | (0 << 2) | (0 << 1) | (1 << 0);
                // Output data rate (10 Hz when paired with 128 oversample)
                // Oversample of 128
                // Disable fast read
                // Continuous measurement
                // Active mode
    if (XLoSendI2C(hI2C, 2, bufOut)) {
        DIAG_PRINT("Failed sending CTRL_REG1!\n");
    }
}

///// I2C Functions /////
int XLoSetTargetI2C(int hI2C, int targetAddress) {
    if (ioctl(hI2C, I2C_SLAVE, targetAddress) < 0) {
        printf("I2C ERROR: Failed to set target address to %d!\n", targetAddress);
        return 1;
    }
    return 0;
}

int XLoSendI2C(int hI2C, int bytes, uint8_t *pData) {
    int rc = write(hI2C, pData, bytes);
    if (rc == bytes) {
        return I2C_ERROR_OK;
    } else if (rc < 0) {
        printf("I2C ERROR: Failed to send %d bytes!\n", bytes);
        return I2C_ERROR_FAILED;
    } else {
        printf("I2C ERROR: Only sent %d of %d bytes!\n", rc, bytes);
        return I2C_ERROR_PARTIAL;
    }
}

int XLoRecvI2C(int hI2C, int bytes, uint8_t *pData) {
    int rc = read(hI2C, pData, bytes);
    if (rc == bytes) {
        return I2C_ERROR_OK;
    } else if (rc < 0) {
        printf("I2C ERROR: Failed to read %d bytes!\n", bytes);
        return I2C_ERROR_FAILED;
    } else {
        printf("I2C ERROR: Only read %d of %d bytes!\n", rc, bytes);
        return I2C_ERROR_PARTIAL;
    }
}

