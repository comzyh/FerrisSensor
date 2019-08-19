#ifndef MPU_REG
#define MPU_REG

typedef enum {
  SMPLRT_DIV = 0x19, // Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV)
  CONFIG,
  GYRO_CONFIG,
  ACCEL_CONFIG,
  ACCEL_XOUT_H = 0x3B,
  ACCEL_XOUT_L,
  ACCEL_YOUT_H,
  ACCEL_YOUT_L,
  ACCEL_ZOUT_H,
  ACCEL_ZOUT_L,

  TEMP_OUT_H = 0x41,
  TEMP_OUT_L,

  GYRO_XOUT_H = 0x43,
  GYRO_XOUT_L,
  GYRO_YOUT_H,
  GYRO_YOUT_L,
  GYRO_ZOUT_H,
  GYRO_ZOUT_L,

  PWR_MGMT_1 = 0x6B,
  PWR_MGMT_2,

} mpu_reg_address;

//  Digital Low Pass Filter
#define DLPF_5HZ (6)
#define DLPF_10HZ (5)
#define DLPF_21HZ (4)
#define DLPF_44HZ (3)
#define DLPF_94HZ (2)
#define DLPF_184HZ (1)
#define DLPF_260HZ (0)
#define DLPF_RESERVE (7)

#define SAMPLE_50HZ (19)
#define SAMPLE_125HZ (7)

#define ACCEL_FS_2g (0)
#define ACCEL_FS_4g (8)
#define ACCEL_FS_8g (0x10)
#define ACCEL_FS_16g (0x18)

#define GYRO_FS_250 (0)
#define GYRO_FS_500 (8)
#define GYRO_FS_1000 (10)
#define GYRO_FS_2000 (18)

// PWR_MGMT_1
//  it is highly recommended that the device be configured to use one of the gyroscopes (or an external clock source)
//  as the clock reference for improved stability.
#define CLKSEL_INTER8M (0)
#define CLKSEL_PllGyroX (1)
#define CLKSEL_PllGyroY (2)
#define CLKSEL_PllGyroZ (3)
#define TEMP_DIS (8)
#define CYCLE (0x20)
#define SLEEP (0x40)

#define WAKEUP (0x00)

// PWR_MGMT2
#define gyroscope_STBY (0x07)
#define LP_WAKE_CTRL_1_25 (0x00)
#define LP_WAKE_CTRL_5 (0x40)
#define LP_WAKE_CTRL_20 (0x80)
#define LP_WAKE_CTRL_40 (0xC0)


#endif
