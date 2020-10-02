/* Headre file for adding MYS node */
#define SN "Dimmable Light"
#define SV "1.0"
#define CHILD_ID_LIGHT 1

#define EPROM_LIGHT_STATE 1
#define EPROM_DIMMER_LEVEL 2

#define LIGHT_OFF 0
#define LIGHT_ON 1

void Init_Node(void);
void Present_DimmableLight(void);
void receive(const MyMessage &message);
void SetCurrentState2Hardware();
void SendCurrentState2Controller();