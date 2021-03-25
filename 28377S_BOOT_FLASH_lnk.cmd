
MEMORY
{
PAGE 0 :  /* Program Memory */
          /* Memory (RAM/FLASH) blocks can be moved to PAGE1 for data allocation */
          /* BEGIN is used for the "boot to Flash" bootloader mode   */

   PREBOOT_START    : origin = 0x080000, length = 0x000002
   RAMM0           	: origin = 0x000122, length = 0x0002DE
   RAMD0           	: origin = 0x00B000, length = 0x000800
   UPDATE_FLAG      : origin = 0x008000, length = 0x000002  /* Boot flag, updated by app when jump from app to boot */
   RAMLS0          	: origin = 0x008002, length = 0x0007FE
   RAMLS1          	: origin = 0x008800, length = 0x000800
   RAMLS2      		: origin = 0x009000, length = 0x000800
   RAMLS3      		: origin = 0x009800, length = 0x000800
   RAMLS4      		: origin = 0x00A000, length = 0x000800
   RAMGS2           : origin = 0x00E000, length = 0x001000
   RAMGS14     		: origin = 0x01A000, length = 0x001000
   RAMGS15     		: origin = 0x01B000, length = 0x001000
   RESET           	: origin = 0x3FFFC0, length = 0x000002
   
   /* Flash BANK0 sectors */
   FLASHA           : origin = 0x080002, length = 0x000FFE	/* on-chip Flash, PreBootloader program start */
   EXIT             : origin = 0x081000, length = 0x001000	/* on-chip Flash, PreBootloader program start */
   BOOT0_START      : origin = 0x082000, length = 0x000002	/* on-chip Flash, Bootloader0 program start */
   FLASHB           : origin = 0x082002, length = 0x001FFE	/* on-chip Flash */
   FLASHC           : origin = 0x084000, length = 0x002000	/* on-chip Flash */
   FLASHD           : origin = 0x086000, length = 0x001FF4	/* on-chip Flash */
   BOOT0_PN         : origin = 0x087FF4, length = 0x000004  /* bootloader0 software version, 5 valid bytes */
   BOOT0_CRC        : origin = 0x087FF8, length = 0x000004  /* bootloader0 CRC, Bootloader0 program end */
   BOOT0_VALID		: origin = 0x087FFC, length = 0x000004  /* bootloader0 Valid flag */
   FLASHE           : origin = 0x088000, length = 0x008000	/* on-chip Flash, Application0 program start */
   FLASHF           : origin = 0x090000, length = 0x008000	/* on-chip Flash */
   FLASHG           : origin = 0x098000, length = 0x008000	/* on-chip Flash */
   FLASHH           : origin = 0x0A0000, length = 0x008000	/* on-chip Flash */
   FLASHI           : origin = 0x0A8000, length = 0x008000	/* on-chip Flash */
   FLASHJ           : origin = 0x0B0000, length = 0x007FF8	/* on-chip Flash */
   APP_PN           : origin = 0x0B7FF8, length = 0x000004	/* Application software version, 5 valid bytes */
   APP_CRC          : origin = 0x0B7FFC, length = 0x000004  /* Application CRC */
   /*reserved*/
   FLASHK           : origin = 0x0B8000, length = 0x001FFE	/* on-chip Flash */
   FLASHL           : origin = 0x0BA000, length = 0x002000	/* on-chip Flash */
   FLASHM           : origin = 0x0BC000, length = 0x002000	/* on-chip Flash */
   FLASHN           : origin = 0x0BE000, length = 0x002000	/* on-chip Flash */
   /* Flash BANK1 sectors */
   BOOT1_START      : origin = 0x0C0000, length = 0x000002	/* on-chip Flash, Bootloader1 program start  */
   FLASHO           : origin = 0x0C0002, length = 0x001FFE	/* on-chip Flash */
   FLASHP           : origin = 0x0C2000, length = 0x002000	/* on-chip Flash */
   FLASHQ           : origin = 0x0C4000, length = 0x001FF4	/* on-chip Flash */
   BOOT1_PN         : origin = 0x0C5FF4, length = 0x000004  /* bootloader0 software version, 5 valid bytes */
   BOOT1_CRC        : origin = 0x0C5FF8, length = 0x000004  /* bootloader0 CRC, Bootloader0 program end */
   BOOT1_VALID		: origin = 0x0C5FFC, length = 0x000004  /* bootloader0 Valid flag */
   /* Flag area */
   APP_VAILID       : origin = 0x0C6000, length = 0x000004	/* Application software vaild flag(2 x 16bit) */
   HW_VERSION       : origin = 0x0C6004, length = 0x000004	/* OBC hardware version(1 x 16bit) */
   HW_VER_CRC       : origin = 0x0C6008, length = 0x000004	/* OBC hardware version CRC(1 x 16bit) */
   HW_SERIAL        : origin = 0x0C600C, length = 0x000004	/* OBC hardware serial number(7 x 8bit) */
   HW_SER_CRC       : origin = 0x0C6010, length = 0x000004	/* OBC hardware serial number CRC(2 x 8bit) */
   /* Flag area */
   FLASHR           : origin = 0x0C6014, length = 0x001FEC	/* on-chip Flash */
   FLASHS           : origin = 0x0C8000, length = 0x008000	/* on-chip Flash, Reserved for Application1 program start*/
   FLASHT           : origin = 0x0D0000, length = 0x008000	/* on-chip Flash */   
   FLASHU           : origin = 0x0D8000, length = 0x008000	/* on-chip Flash */   
   FLASHV           : origin = 0x0E0000, length = 0x008000	/* on-chip Flash */   
   FLASHW           : origin = 0x0E8000, length = 0x008000	/* on-chip Flash */   
   FLASHX           : origin = 0x0F0000, length = 0x008000	/* on-chip Flash */
   FLASHY           : origin = 0x0F8000, length = 0x002000	/* on-chip Flash */
   FLASHZ           : origin = 0x0FA000, length = 0x002000	/* on-chip Flash */
   FLASHAA          : origin = 0x0FC000, length = 0x002000	/* on-chip Flash */   
   FLASHAB          : origin = 0x0FE000, length = 0x002000	/* on-chip Flash */   

PAGE 1 : /* Data Memory */
         /* Memory (RAM/FLASH) blocks can be moved to PAGE0 for program allocation */

   BOOT_RSVD       : origin = 0x000002, length = 0x000120     /* Part of M0, BOOT rom will use this for stack */
   RAMM1           : origin = 0x000400, length = 0x000400     /* on-chip RAM block M1 */
   RAMD1           : origin = 0x00B800, length = 0x000800

   RAMLS5      : origin = 0x00A800, length = 0x000800

   RAMGS0      : origin = 0x00C000, length = 0x001000
   RAMGS1      : origin = 0x00D000, length = 0x001000

   RAMGS3      : origin = 0x00F000, length = 0x001000
   RAMGS4      : origin = 0x010000, length = 0x001000
   RAMGS5      : origin = 0x011000, length = 0x001000
   RAMGS6      : origin = 0x012000, length = 0x001000
   RAMGS7      : origin = 0x013000, length = 0x001000
   RAMGS8      : origin = 0x014000, length = 0x001000
   RAMGS9      : origin = 0x015000, length = 0x001000
   RAMGS10     : origin = 0x016000, length = 0x001000
   RAMGS11     : origin = 0x017000, length = 0x001000
   RAMGS12     : origin = 0x018000, length = 0x001000
   RAMGS13     : origin = 0x019000, length = 0x001000
   
}

SECTIONS
{
   /* Allocate program areas: */
   /* ******************************************************
      ##define Macro used in .cmd file
	*Open CCS project propeties->Build -> C2000 Linker -> Advanced Options ->
 	command file Preprocessing;
	*Add macro in Pre-define preprocessor macro name;
	*********************************************************/
#if defined (__IS_STANDALONE) && defined (__IS_ODD)
#define   text_sector (FLASHO | FLASHP | FLASHQ)
#define   init_sector FLASHO
#define   BOOT_PN     BOOT1_PN
#define   BEGIN       BOOT1_START
#define   VALID_FLAG  BOOT1_VALID
#elif defined (__IS_STANDALONE) && defined (__IS_EVEN)
#define   text_sector (FLASHB | FLASHC | FLASHD)
#define   init_sector FLASHB
#define   BOOT_PN     BOOT0_PN
#define   BEGIN       BOOT0_START
#define   VALID_FLAG  BOOT0_VALID
#endif

#ifndef __IS_STANDALONE
#define   BEGIN       PREBOOT_START
#define   text_sector (FLASHB | FLASHC | FLASHD)
#define   init_sector FLASHB
#define   BOOT_PN     BOOT0_PN
#define   VALID_FLAG  BOOT0_VALID
#endif

   codestart           : > BEGIN       		PAGE = 0, ALIGN(4)
   .text               : >> text_sector     PAGE = 0, ALIGN(4)
   .cinit              : > init_sector      PAGE = 0, ALIGN(4)
   .pinit              : > init_sector      PAGE = 0, ALIGN(4)
      /* Initalized sections go in Flash */
   .econst             : > init_sector      PAGE = 0, ALIGN(4)
   .switch             : > init_sector      PAGE = 0, ALIGN(4)

	/* user defined sections */
#ifndef __IS_STANDALONE
   .preboot            : > FLASHA       	PAGE = 0, ALIGN(4)
   exitboot            : > EXIT       		PAGE = 0, ALIGN(4)
#endif

#ifdef __TI_COMPILER_VERSION__
   #if __TI_COMPILER_VERSION__ >= 15009000
   		GROUP
        {
            .TI.ramfunc
            { -l F021_API_F2837xS_FPU32.lib}
        }                LOAD = text_sector,
                         RUN = RAMGS2,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         PAGE = 0, ALIGN(4)
   #else
        GROUP
        {
            ramfuncs
            { -l F021_API_F2837xS_FPU32.lib}
        }                LOAD = text_sector,
                         RUN = RAMGS2,
                         LOAD_START(_RamfuncsLoadStart),
                         LOAD_SIZE(_RamfuncsLoadSize),
                         LOAD_END(_RamfuncsLoadEnd),
                         RUN_START(_RamfuncsRunStart),
                         RUN_SIZE(_RamfuncsRunSize),
                         RUN_END(_RamfuncsRunEnd),
                         PAGE = 0, ALIGN(4)
   #endif
#endif

						 
   /* Allocate uninitalized data sections: */
   .stack              : > RAMM1        PAGE = 1
   .ebss               : >> RAMLS5 | RAMGS0 | RAMGS1       PAGE = 1
   .esysmem            : > RAMLS5       PAGE = 1
   
   /* user defined sections */
   .boot_ver           : > BOOT_PN      PAGE = 0
   .boot_valid         : > VALID_FLAG   PAGE = 0
   .updataflag         : > UPDATE_FLAG  PAGE = 0, TYPE = NOLOAD
   .reset              : > RESET,     PAGE = 0, TYPE = DSECT /* not used, */
   
}

/*
//===========================================================================
// End of file.
//===========================================================================
*/
