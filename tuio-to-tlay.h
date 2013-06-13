// tlay-touch headers
#include "tlay-touch/TLayTouch.h"
#include "tlay-touch/udp/UdpClient.h"

// tuio headers
#include "TuioListener.h"
#include "TuioClient.h"
#include "TuioServer.h"
#include "TuioTime.h"
#include "TuioCursor.h"

// other headers
#include <vector>
#include <math.h>

using namespace TUIO;

class TuioRelay : public TuioListener {
	private:
		// Functions
		std::string generateValues(int iCount, ...);
		std::string generateCommand(const char * cName, const char * cAction, int iCID, long int iSID, std::string sValues);
		bool netSend(std::string type, unsigned int action, unsigned int id, float x, float y, float ms, float ma);

		// Pointers
		UdpClient *uC;
		std::vector<TuioServer*> tS;
		std::vector<std::list<TuioCursor*>* > cursorList;
		TuioTime tTime;

	public:
		TuioRelay();
		// Cursors
		void addTuioCursor(TuioCursor *tCur);
		void updateTuioCursor(TuioCursor *tCur);
		void removeTuioCursor(TuioCursor *tCur);

		// Objects
		void addTuioObject(TuioObject *tObj);
		void updateTuioObject(TuioObject *tObj);
		void removeTuioObject(TuioObject *tObj);

		// Frames
		void refresh(TuioTime tFrameTime);
};

// TODO: This shouldn't be here
#define BUFFER_LENGTH 256 // This is the max-size of a single command

#define COMMAND_STRUCTURE "{\"message\": {\"command\": {\"type\": \"tuio\", \"attr\": {\"name\": \"%s\", \"action\": \"%s\", \"cid\": %d, \"sid\": %ld, \"values\": \"%s\"}}}}\n\n" //"%s|%s(%d,%ld):%s\n" // NAME|ACTION(ID):VALUES
#define COMMAND_SEPERATOR ","

// Command sample: C|A(0,2):0.2625,0.458333

#define NAME_CURSOR "CURSOR" // "C"
#define NAME_OBJECT "OBJECT" // "O"
#define NAME_FRAME  "FRAME" // "F"

#define ACTION_ADD     "ADD" // "A"
#define ACTION_UPDATE  "UPDATE" // "U"
#define ACTION_REMOVE  "REMOVE" // "R"
#define ACTION_REFRESH "REFRESH" // "S"

#define RELAY_REFRESH false
typedef struct {
	const char *host;
	int port;
} Relay;
static const Relay relays[] = {
	{ "localhost", 3663 },
 { "localhost", 3664 },
};

#define IPC_HOST "localhost"
#define IPC_PORT 2503

#define LOG_DEBUG  "[DEBUG ] "
#define LOG_INFO   "[INFO  ] "
#define LOG_NOTICE "[NOTICE] "
#define ENABLED_DEBUG true
#define ENABLED_NOTICE true