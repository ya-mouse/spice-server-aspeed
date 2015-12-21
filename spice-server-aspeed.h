#ifndef __AST_BASE_H__
#define __AST_BASE_H__

#include <spice.h>
#include <linux/types.h>
#include <sys/ioctl.h>

#include "basic_event_loop.h"

#define COUNT(x) ((sizeof(x)/sizeof(x[0])))

/* Character device IOCTL's */
#define ASTCAP_MAGIC  		'a'
#define ASTCAP_IOCCMD		_IOWR(ASTCAP_MAGIC, 0, ASTCap_Ioctl)

typedef enum FPGA_opcode {
    ASTCAP_IOCTL_RESET_VIDEOENGINE = 0,
    ASTCAP_IOCTL_START_CAPTURE,
    ASTCAP_IOCTL_STOP_CAPTURE,
    ASTCAP_IOCTL_GET_VIDEO,
    ASTCAP_IOCTL_GET_CURSOR,
    ASTCAP_IOCTL_CLEAR_BUFFERS,
    ASTCAP_IOCTL_SET_VIDEOENGINE_CONFIGS,
    ASTCAP_IOCTL_GET_VIDEOENGINE_CONFIGS,
    ASTCAP_IOCTL_SET_SCALAR_CONFIGS,
    ASTCAP_IOCTL_ENABLE_VIDEO_DAC,
} ASTCap_OpCode;

typedef enum {
	ASTCAP_IOCTL_SUCCESS = 0,
	ASTCAP_IOCTL_ERROR,
	ASTCAP_IOCTL_NO_VIDEO_CHANGE,
	ASTCAP_IOCTL_BLANK_SCREEN,
} ASTCap_ErrCode;

typedef struct {
	ASTCap_OpCode OpCode;
	ASTCap_ErrCode ErrCode;
	unsigned long Size;
	void *vPtr;
	unsigned char Reserved [2];
} ASTCap_Ioctl;

/* Note :All Length Fields used in IUSB are Little Endian */

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;

#ifdef __GNUC__
#define PACKED __attribute__ ((packed))
#else
#define PACKED
#pragma pack( 1 )
#endif


/**
 * @struct  IUSB_HEADER packet format
 * @brief
**/
typedef struct
{
  u8      Signature [8];                  // signature "IUSB    "
  u8      Major;
  u8      Minor;
  u8      HeaderLen;                      // Header length
  u8      HeaderCheckSum;
  u32     DataPktLen;
  u8      ServerCaps;
  u8      DeviceType;
  u8      Protocol;
  u8      Direction;
  u8      DeviceNo;
  u8      InterfaceNo;
  u8      ClientData;
  u8      Instance;
  u32     SeqNo;
  u32     Key;
} PACKED IUSB_HEADER;


/*************************iUSB Device Information **************************/
typedef struct
{
	uint8 DeviceType;
	uint8 DevNo;
	uint8 IfNum;
	uint8 LockType;
	uint8 Instance;
} PACKED  IUSB_DEVICE_INFO;

typedef struct
{
	IUSB_HEADER 		Header;
	uint8			DevCount;
	IUSB_DEVICE_INFO	DevInfo;	/* for each device */
} PACKED IUSB_DEVICE_LIST;

#define LOCK_TYPE_SHARED	0x2
#define LOCK_TYPE_EXCLUSIVE	0x1

typedef struct
{
	uint8 DeviceType;
	uint8 LockType;
	IUSB_DEVICE_LIST	iUSBdevList;
} PACKED IUSB_FREE_DEVICE_INFO;

typedef struct
{
	IUSB_DEVICE_INFO	DevInfo;
	uint32	Key;
} PACKED  IUSB_REQ_REL_DEVICE_INFO;

typedef struct
{
	uint32 Key;
	IUSB_DEVICE_INFO	DevInfo;
	uint8 Data;
} PACKED IUSB_IOCTL_DATA;

/******* Values for iUsb Signature and Major and Minor Numbers ***********/
#define IUSB_SIG			"IUSB    "
#define IUSB_MAJOR			1
#define IUSB_MINOR			0

/************************ Values for Protocol ****************************/
#define IUSB_PROTO_RESERVED		0x00
#define IUSB_PROTO_SCSI			0x01
#define IUSB_PROTO_FLOPPY		0x02
#define IUSB_PROTO_ATA			0x03
#define IUSB_PROTO_KEYBD_DATA	0x10	/* Keybd Data */
#define IUSB_PROTO_KEYBD_STATUS	0x11	/* Keybd Led Status */
#define IUSB_PROTO_MOUSE_DATA	0x20
#define IUSB_PROTO_MOUSE_ALIVE	0x21	/* Host Mouse Alive */


/************************ Values for DeviceType **************************/

/*
   Note: We use the SCSI-2 values for SCSI devices defined in the SCSI-2
         Specs. For non SCSI devices and those not defined in SCSI-2 Specs
         we follow our own values.
		 Each definition has the equivalent SCSI definition at the end of
		 line in a comment section.
*/

/* Defines for Lower 7 bits of Device Type */
#define IUSB_DEVICE_HARDDISK		0x00	/* PERIPHERAL_DIRECT		*/
#define IUSB_DEVICE_TAPE			0x01	/* PERIPHERAL_SEQUENCTIAL	*/
#define IUSB_DEVICE_02				0x02
#define IUSB_DEVICE_03				0x03
#define IUSB_DEVICE_WORM			0x04 	/* PERIPHERAL_WRITE_ONCE	*/
#define IUSB_DEVICE_CDROM			0x05	/* PERIPHERAL_CDROM			*/
#define IUSB_DEVICE_OPTICAL			0x06	/* PERIPHERAL_OPTICAL		*/
#define IUSB_DEVICE_COMM			0x09    /* Communications Device    */
#define IUSB_DEVICE_UNKNOWN			0x1F	/* PERIPHERAL_UNKNOWN		*/
#define IUSB_DEVICE_FLOPPY			0x20
#define IUSB_DEVICE_CDRW			0x21
#define IUSB_CDROM_FLOPPY_COMBO		0x22
#define IUSB_DEVICE_FLASH			0x23
#define IUSB_DEVICE_KEYBD			0x30
#define IUSB_DEVICE_MOUSE			0x31
#define IUSB_DEVICE_HID				0x32	/* Keybd and Mouse Composite*/
#define IUSB_DEVICE_HUB				0x33
#define IUSB_DEVICE_RESERVED		0x7F

/* Defines for High Bit of Device Type  */
# define IUSB_DEVICE_FIXED			0x00	/*	MEDIUM_FIXED			*/
# define IUSB_DEVICE_REMOVABLE		0x80	/*	MEDIUM_REMOVABLE		*/

/**************** Values for Direction Field of IUSB Header ****************/
#define TO_REMOTE	0x00
#define FROM_REMOTE	0x80



typedef struct
{
  IUSB_HEADER   Header;
  u8            DataLen;
  u8            Data;
} PACKED IUSB_HID_PACKET;



typedef struct
{
	uint8 		Event;
	uint8		dx;
	uint8		dy;
	uint16		AbsX;
	uint16		AbsY;
	uint16		ResX;
	uint16		ResY;
	uint8		IsValidData;
} PACKED HOST_MOUSE_PKT;

/*used by hidserver for playing back 
  absolute mouse directly on USB
  in this new mode absolute scaled coords
  are always sent by the remote console and this
  is played back driectly by the USB mouse itself.
*/
typedef struct 
{
    uint8 Event;
    uint16 ScaledX;
    uint16 ScaledY;
}
PACKED DIRECT_ABSOLUTE_USB_MOUSE_PKT;




#define FLEXIBLE_DISK_PAGE_CODE	(0x05)
typedef struct
{
	uint16 ModeDataLen;
#define MEDIUM_TYPE_DEFAULT	(0x00)
#define MEDIUM_TYPE_720KB	(0x1e)
#define MEDIUM_TYPE_125_MB	(0x93)
#define MEDIUM_TYPE_144_MB	(0x94)
	uint8 MediumTypeCode;
	uint8 WriteProtect; // bit(7):write protect, bit(6-5):reserved, bit(4):DPOFUA which should be zero, bit(3-0) reserved
	uint8 Reserved[4];
} PACKED MODE_SENSE_RESP_HDR;

typedef struct
{
	MODE_SENSE_RESP_HDR	ModeSenseRespHdr;
#define MAX_MODE_SENSE_RESP_DATA_LEN	(72)
#define FLEXIBLE_DISK_PAGE_LEN	(32)
	uint8	PageData[MAX_MODE_SENSE_RESP_DATA_LEN];
} PACKED MODE_SENSE_RESPONSE;

typedef struct
{
	uint8	PageCode;
	uint8	PageLength; //1Eh
	uint16	TransferRate;
	uint8	NumberofHeads;
	uint8	SectorsPerTrack;
	uint16	DataBytesPerSector;
	uint16	NumberofCylinders;
	uint8	Reserved1[9];
	uint8	MotorONDelay;
	uint8	MotorOFFDelay;
	uint8	Reserved2[7];
	uint16	MediumRotationRate;
	uint8	Reserved3;
	uint8	Reserved4;
} PACKED FLEXIBLE_DISK_PAGE;

/********* Scsi Status Packet Structure used in IUSB_SCSI_PACKET ***********/
typedef struct
{
	uint8 	OverallStatus;
	uint8	SenseKey;
	uint8	SenseCode;
	uint8	SenseCodeQ;
} PACKED SCSI_STATUS_PACKET;




#ifndef __GNUC__
#pragma pack()
#endif

/*Ioctls */
#define USB_GET_IUSB_DEVICES	_IOC(_IOC_READ, 'U',0x00,0x3FFF)
#define USB_GET_CONNECT_STATE	_IOC(_IOC_READ, 'U',0x01,0x3FFF)
#define USB_KEYBD_DATA		_IOC(_IOC_WRITE,'U',0x11,0x3FFF)
#define USB_KEYBD_LED		_IOC(_IOC_READ ,'U',0x12,0x3FFF)
#define USB_KEYBD_EXIT		_IOC(_IOC_WRITE,'U',0x13,0x3FFF)
#define USB_KEYBD_LED_NO_WAIT	_IOC(_IOC_READ,'U',0x14,0x3FFF)

#define USB_MOUSE_DATA		_IOC(_IOC_WRITE,'U',0x21,0x3FFF)
#define MOUSE_ABS_TO_REL        _IOC(_IOC_WRITE,'U',0x22,0x3FFF)
#define MOUSE_REL_TO_ABS        _IOC(_IOC_WRITE,'U',0x23,0x3FFF)
#define MOUSE_GET_CURRENT_MODE  _IOC(_IOC_WRITE,'U',0x24,0x3FFF)
#define USB_GET_ADDRESS  	_IOC(_IOC_WRITE,'U',0x25,0x3FFF) 

#define USB_CDROM_REQ		_IOC(_IOC_READ, 'U',0x31,0x3FFF)
#define USB_CDROM_RES		_IOC(_IOC_WRITE,'U',0x32,0x3FFF)
#define USB_CDROM_EXIT		_IOC(_IOC_WRITE,'U',0x33,0x3FFF)
#define USB_CDROM_ACTIVATE	_IOC(_IOC_WRITE,'U',0x34,0x3FFF)


#define USB_HDISK_REQ		_IOC(_IOC_READ, 'U',0x41,0x3FFF)
#define USB_HDISK_RES		_IOC(_IOC_WRITE,'U',0x42,0x3FFF)
#define USB_HDISK_EXIT		_IOC(_IOC_WRITE,'U',0x43,0x3FFF)
#define USB_HDISK_ACTIVATE	_IOC(_IOC_WRITE,'U',0x44,0x3FFF)
#define USB_HDISK_SET_TYPE	_IOC(_IOC_WRITE,'U',0x45,0x3FFF)
#define USB_HDISK_GET_TYPE	_IOC(_IOC_READ, 'U',0x46,0x3FFF)

#define USB_FLOPPY_REQ		_IOC(_IOC_READ, 'U',0x51,0x3FFF)
#define USB_FLOPPY_RES		_IOC(_IOC_WRITE,'U',0x52,0x3FFF)
#define USB_FLOPPY_EXIT		_IOC(_IOC_WRITE,'U',0x53,0x3FFF)
#define USB_FLOPPY_ACTIVATE	_IOC(_IOC_WRITE,'U',0x54,0x3FFF)

#define USB_VENDOR_REQ		_IOC(_IOC_READ, 'U',0x61,0x3FFF)
#define USB_VENDOR_RES		_IOC(_IOC_WRITE,'U',0x62,0x3FFF)
#define USB_VENDOR_EXIT		_IOC(_IOC_WRITE,'U',0x63,0x3FFF)

#define USB_LUNCOMBO_ADD_CD	_IOC(_IOC_WRITE,'U',0x91,0x3FFF) 
#define USB_LUNCOMBO_ADD_FD	_IOC(_IOC_WRITE,'U',0x92,0x3FFF) 
#define USB_LUNCOMBO_ADD_HD	_IOC(_IOC_WRITE,'U',0x93,0x3FFF) 
#define USB_LUNCOMBO_ADD_VF	_IOC(_IOC_WRITE,'U',0x94,0x3FFF) 
#define USB_LUNCOMBO_RST_IF	_IOC(_IOC_WRITE,'U',0x95,0x3FFF) 

#define USB_CDROM_ADD_CD	_IOC(_IOC_WRITE,'U',0xA1,0x3FFF) 
#define USB_CDROM_RST_IF	_IOC(_IOC_WRITE,'U',0xA2,0x3FFF) 

#define USB_FLOPPY_ADD_FD	_IOC(_IOC_WRITE,'U',0xA6,0x3FFF) 
#define USB_FLOPPY_RST_IF	_IOC(_IOC_WRITE,'U',0xA7,0x3FFF) 

#define USB_HDISK_ADD_HD	_IOC(_IOC_WRITE,'U',0xAB,0x3FFF) 
#define USB_HDISK_RST_IF	_IOC(_IOC_WRITE,'U',0xAC,0x3FFF) 

#define USB_DEVICE_DISCONNECT	_IOC(_IOC_WRITE,'U',0xE3,0x3FFF)
#define USB_DEVICE_RECONNECT	_IOC(_IOC_WRITE,'U',0xE4,0x3FFF)

#define USB_GET_INTERFACES	_IOC(_IOC_READ,'U',0xF1,0x3FFF)
#define USB_REQ_INTERFACE	_IOC(_IOC_READ,'U',0xF2,0x3FFF)
#define USB_REL_INTERFACE	_IOC(_IOC_WRITE,'U',0xF3,0x3FFF)


/*
 * simple queue for commands.
 * each command can have up to two parameters (grow as needed)
 *
 * TODO: switch to gtk main loop. Then add gobject-introspection. then
 * write tests in python/guile/whatever.
 */
typedef enum {
    PATH_PROGRESS,
    SIMPLE_CREATE_SURFACE,
    SIMPLE_DRAW,
    SIMPLE_DRAW_BITMAP,
    SIMPLE_DRAW_SOLID,
    SIMPLE_COPY_BITS,
    SIMPLE_DESTROY_SURFACE,
    SIMPLE_UPDATE,
    DESTROY_PRIMARY,
    CREATE_PRIMARY,
    SLEEP
} CommandType;

typedef struct CommandCreatePrimary {
    uint32_t width;
    uint32_t height;
} CommandCreatePrimary;

typedef struct CommandCreateSurface {
    uint32_t surface_id;
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint8_t *data;
} CommandCreateSurface;

typedef struct CommandDrawBitmap {
    QXLRect bbox;
    uint8_t *bitmap;
    uint32_t surface_id;
    uint32_t num_clip_rects;
    QXLRect *clip_rects;
} CommandDrawBitmap;

typedef struct CommandDrawSolid {
    QXLRect bbox;
    uint32_t color;
    uint32_t surface_id;
} CommandDrawSolid;

typedef struct CommandSleep {
    uint32_t secs;
} CommandSleep;

typedef struct Command Command;
typedef struct Test Test;

struct Command {
    CommandType command;
    void (*cb)(Test *test, Command *command);
    void *cb_opaque;
    union {
        CommandCreatePrimary create_primary;
        CommandDrawBitmap bitmap;
        CommandDrawSolid solid;
        CommandSleep sleep;
        CommandCreateSurface create_surface;
    };
};

#define MAX_HEIGHT 2048
#define MAX_WIDTH 2048

#define SURF_WIDTH 320
#define SURF_HEIGHT 240

/* format of cursor pattern is ARGB444, each pixel is presented by 16 bits(2 bytes) */
#define AST_VIDEOCAP_CURSOR_BITMAP			(64 * 64)
#define AST_VIDEOCAP_CURSOR_BITMAP_DATA		(AST_VIDEOCAP_CURSOR_BITMAP * 2)

struct ast_videocap_cursor_info_t {
	uint8_t type; /* 0: monochrome, 1: color */
	uint32_t checksum;
	uint16_t pos_x;
	uint16_t pos_y;
	uint16_t offset_x;
	uint16_t offset_y;
	uint16_t pattern[AST_VIDEOCAP_CURSOR_BITMAP];
} __attribute__((packed));

typedef struct iUSBSpice {
    int fd;
    uint32_t key;
    uint32_t seq_no;
    uint16_t addr;
} iUSBSpice;

typedef struct iUSBSpiceKbd {
    iUSBSpice iusb;
    SpiceKbdInstance sin;
    int ledstate;
    int emul0;
    int modifiers;
} iUSBSpiceKbd;

typedef struct iUSBSpicePointer {
    iUSBSpice iusb;
    SpiceMouseInstance sin;
    int width, height;
    uint32_t last_bmask;
    int absolute;
    int last_x, last_y;
} iUSBSpicePointer;


struct Test {
    SpiceCoreInterface *core;
    SpiceServer *server;

    QXLInstance qxl_instance;
    QXLWorker *qxl_worker;

    uint8_t primary_surface[1];
    int primary_height;
    int primary_width;

    SpiceTimer *wakeup_timer;
    int wakeup_ms;

    int cursor_notify;

    // qxl scripted rendering commands and io
    Command *commands;
    int num_commands;
    int cmd_index;

    int target_surface;

    // callbacks
    void (*on_client_connected)(Test *test);
    void (*on_client_disconnected)(Test *test);

    int started;

    iUSBSpicePointer pointer;
    struct ast_videocap_cursor_info_t curinfo;

    /* ---------- Aspeed private ---------- */
    int videocap_fd;
    void *mmap;
    ASTCap_Ioctl ioc;
};

struct ASTHeader
{
    short version;
    short headlen;

    short src_mode_x;
    short src_mode_y;
    short src_mode_depth;
    short src_mode_rate;
    char src_mode_index;

    short dst_mode_x;
    short dst_mode_y;
    short dst_mode_depth;
    short dst_mode_rate;
    char dst_mode_index;

    int frame_start;
    int frame_num;
    short frame_vsize;
    short frame_hsize;

    int rsvd[2];

    char compression;
    char jpeg_scale;
    char jpeg_table;
    char jpeg_yuv;
    char sharp_mode;
    char adv_table;
    char adv_scale;
    int num_of_MB;
    char rc4_en;
    char rc4_reset;

    char mode420;

    char inf_downscale;
    char inf_diff;
    short inf_analog_thr;
    short inf_dig_thr;
    char inf_ext_sig;
    char inf_auto_mode;
    char inf_vqmode;

    int comp_frame_size;
    int comp_size;
    int comp_hdebug;
    int comp_vdebug;

    char input_signal;
    short cur_xpos;
    short cur_ypos;
} __attribute__((packed));

void test_set_simple_command_list(Test *test, int *command, int num_commands);
void test_set_command_list(Test *test, Command *command, int num_commands);
void test_add_display_interface(Test *test);
void test_add_agent_interface(SpiceServer *server); // TODO - Test *test
Test* ast_new(SpiceCoreInterface* core);

uint32_t test_get_width(void);
uint32_t test_get_height(void);

void spice_test_config_parse_args(int argc, char **argv);

static uint8_t keycode2usbcode[] = {
    0, 41, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 45, 46, 42, 43, 20, 26, 8, 21, 23, 28, 24, 12, 18, 19, 47, 48, 40, 224, 4, 22, 7, 9, 10, 11, 13, 14, 15, 51, 52, 53, 225, 50, 29, 27, 6, 25, 5, 17, 16, 54, 55, 56, 229, 85, 226, 44, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 83, 71, 95, 96, 97, 86, 92, 93, 94, 87, 89, 90, 91, 98, 99, 0, 148, 100, 68, 69, 135, 146, 147, 138, 136, 139, 140, 88, 228, 84, 70, 230, 0, 74, 82, 75, 80, 79, 77, 81, 78, 73, 76, 0, 239, 238, 237, 102, 103, 0, 72, 0, 133, 144, 145, 137, 227, 231, 101, 243, 121, 118, 122, 119, 124, 116, 125, 244, 123, 117, 0, 251, 0, 248, 0, 0, 0, 0, 0, 0, 0, 240, 0, 249, 0, 0, 0, 0, 0, 241, 242, 0, 236, 0, 235, 232, 234, 233, 0, 0, 0, 0, 0, 0, 250, 0, 0, 247, 245, 246, 182, 183, 0, 0, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#endif /* __TEST_DISPLAY_BASE_H__ */
