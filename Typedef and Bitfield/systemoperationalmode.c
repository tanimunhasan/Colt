#include<stdio.h>
#include<stdint.h>

// ENUM: Define all system operation modes

typedef enum                        // Defines system operating modes(state machine friendly)
{
    OPM_HIBERNATE,
    OPM_CONFIG,
    OPM_CONFIG_HIBERNATE_PRERUN,
    OPM_NORMAL,
    OPM_TEST,                       // nominally runs for 60 seconds
    OPM_TEST_CONTINOUS,             // run continually
    OPM_BOOT_PASSTHRU = 0xFE,       // reboot into pass through mode
    OPM_RESET = 0xFF                // Full system / data reset to factory defualts

} OP;

// UNION: Represent status flags with bitfields and byte/word access

typedef union                       // Allow multiple views of system status: word, byte, and individual bits
{
    uint16_t allFlags;
    struct {
        uint8_t systemStatus;
        uint8_t errorFlags;
    };
    struct {
        unsigned powerOk            :1;
        unsigned initialized        :1;
        unsigned configLoaded       :1;
        unsigned opModeValid        :1;
        unsigned sensorOk           :1;
        unsigned reserev            :3;

        unsigned commError          :1;
        unsigned sensorFail         :1;
        unsigned memoryError        :1;
        unsigned watchdogReset      :1;
        unsigned reserved2          :4;
    };
} SystemFlags_t;

// STRUCT: Represent a control block for system operation
typedef struct          // Combines mode, flags, and metadata into a coherent system state block
{
    OP currentOpMode;           // Current operating mode
    SystemFlags_t flags;        // System status and error flags
    uint32_t uptimeSeconds;     // Time since boot
} SystemControl_t;

SystemControl_t systemControl;
// Example functions

void initializeSystem()
{
    systemControl.currentOpMode = OPM_CONFIG;
    systemControl.flags.allFlags = 0;
    systemControl.flags.initialized = 1;
    systemControl.flags.configLoaded = 1;

}

void checkSystemsStatus()
{
    if(systemControl.flags.commError){
        printf("Communication error detected!\n");

        systemControl.flags.commError = 0;
        printf("Communication error flag cleared.\n");
    }


    if(systemControl.currentOpMode == OPM_RESET){
        printf("Performing factory....\n");


        // Simulate reset: clear system status flags
        systemControl.flags.allFlags = 0;
        printf("System flags cleared for reset. \n");


    }

}

int main(void)
{
    initializeSystem();

    systemControl.currentOpMode = OPM_NORMAL;
    systemControl.flags.powerOk = 1;
    systemControl.flags.commError = 1;   // If we not set here  systemControl.flags.commError = 1; --> no print "Communication error detected!
    systemControl.uptimeSeconds = 123;

    checkSystemsStatus();

    // Check flags again after handling
    if(!systemControl.flags.commError){
        printf("commError is now cleared\n");
    }
    if(!systemControl.currentOpMode == OPM_CONFIG){
        printf("Factory reset not performing...\n");

    }

    return 0;
}

/*
    OP            :  tracks mode like boots, config, normal, test.
    SystemFlags_t :  holds runtime conditions, health, and error states.
    SystemControl_t: acts as global system status structure, ideal for RTOS task or bare-metal loops.

 */
