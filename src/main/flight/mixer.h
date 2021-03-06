/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#define MAX_SUPPORTED_MOTORS 12
#define MAX_SUPPORTED_SERVOS 8
#define YAW_JUMP_PREVENTION_LIMIT_LOW 80
#define YAW_JUMP_PREVENTION_LIMIT_HIGH 500


// Note: this is called MultiType/MULTITYPE_* in baseflight.
typedef enum mixerMode
{
    MIXER_QUADX = 1,
    MIXER_GIMBAL = 2,
    MIXER_PPM_TO_SERVO = 3,    // PPM -> servo relay
    MIXER_COAXIAL = 4
} mixerMode_e;

// Custom mixer data per motor
typedef struct motorMixer_t {
    float throttle;
    float roll;
    float pitch;
    float yaw;
} motorMixer_t;

// Custom mixer configuration
typedef struct mixer_t {
    uint8_t motorCount;
    uint8_t useServo;
    const motorMixer_t *motor;
} mixer_t;

typedef struct mixerConfig_s {
    uint8_t pid_at_min_throttle;            // when enabled pids are used at minimum throttle
    int8_t yaw_motor_direction;
    uint16_t yaw_jump_prevention_limit;      // make limit configurable (original fixed value was 100)
#ifdef USE_SERVOS
    uint8_t tri_unarmed_servo;              // send tail servo correction pulses even when unarmed
    int16_t servo_lowpass_freq;             // lowpass servo filter frequency selection; 1/1000ths of loop freq
    int8_t servo_lowpass_enable;            // enable/disable lowpass filter
#endif
} mixerConfig_t;

typedef struct flight3DConfig_s {
    uint16_t deadband3d_low;                // min 3d value
    uint16_t deadband3d_high;               // max 3d value
    uint16_t neutral3d;                     // center 3d value
    uint16_t deadband3d_throttle;           // default throttle deadband from MIDRC
} flight3DConfig_t;

typedef struct airplaneConfig_t {
    int8_t fixedwing_althold_dir;           // +1 or -1 for pitch/althold gain. later check if need more than just sign
} airplaneConfig_t;

#define CHANNEL_FORWARDING_DISABLED (uint8_t)0xFF

#ifdef USE_SERVOS

// These must be consecutive, see 'reversedSources'
enum {
    INPUT_STABILIZED_ROLL = 0,
    INPUT_STABILIZED_PITCH,
    INPUT_STABILIZED_YAW,
    INPUT_STABILIZED_THROTTLE,
    INPUT_RC_ROLL,
    INPUT_RC_PITCH,
    INPUT_RC_YAW,
    INPUT_RC_THROTTLE,
    INPUT_RC_AUX1,
    INPUT_RC_AUX2,
    INPUT_RC_AUX3,
    INPUT_RC_AUX4,
    INPUT_GIMBAL_PITCH,
    INPUT_GIMBAL_ROLL,

    INPUT_SOURCE_COUNT
} inputSource_e;

// target servo channels
typedef enum {
    SERVO_GIMBAL_PITCH = 0,
    SERVO_GIMBAL_ROLL = 1,
    SERVO_SWASH_1 = 2,
    SERVO_SWASH_2 = 3,
    SERVO_SWASH_3 = 4,
    SERVO_SWASH_4 = 5
} servoIndex_e; // FIXME rename to servoChannel_e

#define SERVO_COAXILDUAL_INDEX_MIN SERVO_SWASH_1
#define SERVO_COAXILDUAL_INDEX_MAX SERVO_SWASH_4

typedef struct servoMixer_t {
    uint8_t targetChannel;                  // servo that receives the output of the rule
    uint8_t inputSource;                    // input channel for this rule
    int8_t rate;                            // range [-125;+125] ; can be used to adjust a rate 0-125% and a direction
    uint8_t speed;                          // reduces the speed of the rule, 0=unlimited speed
    int8_t min;                             // lower bound of rule range [0;100]% of servo max-min
    int8_t max;                             // lower bound of rule range [0;100]% of servo max-min
    uint8_t box;                            // active rule if box is enabled, range [0;3], 0=no box, 1=BOXSERVO1, 2=BOXSERVO2, 3=BOXSERVO3
} servoMixer_t;

#define MAX_SERVO_RULES (2 * MAX_SUPPORTED_SERVOS)
#define MAX_SERVO_SPEED UINT8_MAX
#define MAX_SERVO_BOXES 3

// Custom mixer configuration
typedef struct servoMixerRules_s {
    uint8_t servoRuleCount;
    const servoMixer_t *rule;
} servoMixerRules_t;

typedef struct servoParam_t {
    int16_t min;                            // servo min
    int16_t max;                            // servo max
    int16_t middle;                         // servo middle
    int8_t rate;                            // range [-125;+125] ; can be used to adjust a rate 0-125% and a direction
    uint8_t angleAtMin;                     // range [0;180] the measured angle in degrees from the middle when the servo is at the 'min' value.
    uint8_t angleAtMax;                     // range [0;180] the measured angle in degrees from the middle when the servo is at the 'max' value.
    int8_t forwardFromChannel;              // RX channel index, 0 based.  See CHANNEL_FORWARDING_DISABLED
    uint32_t reversedSources;               // the direction of servo movement for each input source of the servo mixer, bit set=inverted
} __attribute__ ((__packed__)) servoParam_t;

struct gimbalConfig_s;
struct escAndServoConfig_s;
struct rxConfig_s;

extern int16_t servo[MAX_SUPPORTED_SERVOS];
bool isMixerUsingServos(void);
void writeServos(void);
void filterServos(void);
#endif

extern int16_t motor[MAX_SUPPORTED_MOTORS];
extern int16_t motor_disarmed[MAX_SUPPORTED_MOTORS];

void mixerUseConfigs(
#ifdef USE_SERVOS
        servoParam_t *servoConfToUse,
        struct gimbalConfig_s *gimbalConfigToUse,
#endif
        flight3DConfig_t *flight3DConfigToUse,
		struct escAndServoConfig_s *escAndServoConfigToUse,
        mixerConfig_t *mixerConfigToUse,
        airplaneConfig_t *airplaneConfigToUse,
		struct rxConfig_s *rxConfigToUse);

void writeAllMotors(int16_t mc);
void mixerLoadMix(int index, motorMixer_t *customMixers);
#ifdef USE_SERVOS
void servoMixerLoadMix(int index, servoMixer_t *customServoMixers);
void loadCustomServoMixer(void);
int servoDirection(int servoIndex, int fromChannel);
#endif
void mixerResetDisarmedMotors(void);
void mixTable(void);
void writeMotors(void);
void stopMotors(void);
void StopPwmAllMotors(void);
