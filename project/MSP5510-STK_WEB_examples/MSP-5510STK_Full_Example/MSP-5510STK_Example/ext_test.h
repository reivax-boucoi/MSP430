void Set_EXT_MASKs(void);
void Set_UEXT_MASKs(void);
void PIN_Init();
void TEST_EXT_Init();
void Locate(unsigned char p, unsigned char port);
void Locate_PIN(unsigned char port1, unsigned char pin1, unsigned port2, unsigned char pin2);
unsigned char TEST_Extension();

//#define P1MASK    0xFF		//bit 0:7 (14-21)
//#define P2MASK    0x01		//bit 0   (22)
//#define P4MASK    0x7F		//bit 0:6 (29-36) //test withou pull pin
//#define P5MASK    0x03		//bit 0:1 (5-6)
//#define P6MASK    0x0F		//bit 0:3 (1-4)	
#define STR_SIZE  64   
#define SET_TIME  200 

extern unsigned char g_f, f, count, flag, e_0, e_1, e_2, e_3;
extern unsigned char aray[12][8];
extern unsigned int d;
extern char str[STR_SIZE];
extern char str_temp[8];
extern unsigned char state;
extern unsigned char P1MASK;
extern unsigned char P2MASK;
extern unsigned char P4MASK;
extern unsigned char P5MASK;
extern unsigned char P6MASK;
extern unsigned char PULL_PIN_DIR;
extern unsigned char PULL_PIN_OUT;
extern unsigned char EXT_flag;