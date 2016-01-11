//// Constants /////
#define BUFFER_MAX              (32)
#define I2C_ERROR_OK            (0)
#define I2C_ERROR_FAILED        (1)
#define I2C_ERROR_PARTIAL       (2)
#define HANDLE_UNINITALISED     (-1)

#ifndef FALSE
#define FALSE                   (0)
#endif
#ifndef TRUE
#define TRUE                    (!FALSE)
#endif

///// Storage /////
extern int busNumber;
extern int addressAccelerometer;
extern int addressCompass;
extern int foundAccelerometer;
extern int foundCompass;
extern int diagnosticPrint;
extern float gPerCount;
extern float tempOffest;

///// Prototypes /////
int XLoInit(void);
int XLoInitialise(int tryOtherBus);
void XLoShutdown(void);
void XLoReadAccelerometer(float *pdata);
void XLoReadCompassRaw(float *pdata);
float XLoReadTemperature(void);
