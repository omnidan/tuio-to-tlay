/*
 * =====================================================================================
 *
 *       Filename:  tuio-to-tlay.cpp
 *
 *    Description:  Convert TUIO events to TLay Touch events
 *
 *        Version:  1.0.0
 *        Created:  06/13/2013 06:44:52 PM
 *       Compiler:  g++
 *
 *         Author:  Daniel Bugl <daniel.bugl@touchlay.com>
 *   Organization:  TouchLay (http://touchlay.com/)
 *
 * =====================================================================================
 */

 /* Note:
  *  This is based off an old project called TuioRelay that relayed TUIO messages from
  *  one port to many others and to a specific IPC system. I just replaced the IPC system
  *  hook at the moment. This should be totally rewritten.
  */

#include "tuio-to-tlay.h"
#include <cstdio>
#include <sstream>
#include <stdarg.h>

#define LENGTH(X) (sizeof X / sizeof X[0])

bool TuioRelay::netSend(std::string type, unsigned int action, unsigned int id, float x, float y, float ms, float ma) {
	std::stringstream ss;
	ss << "tuio-to-tlay@127.0.0.1\%lo/tuio/" << type; // TODO: This probably shouldn't be hardcoded later
	TLayTouch *tt = new TLayTouch(
		ss.str(),
		action,
		id,
		x,
		y,
		0.00, // velocity x - not given
		0.00, // velocity y - not given
		0.00, // angle - not given
		ms,
		ma
	);
	this->uC->send("localhost", 4444, (char*)tt->jexport().c_str());
	return true;
}

TuioRelay::TuioRelay() {
 printf("%s%s\n", LOG_INFO, "Booting tuio-to-tlay (based off TuioRelay)...");
	this->uC = new UdpClient();
	const Relay *r;
	for(unsigned int i = 0; i < LENGTH(relays); i++) {
		r = &relays[i];
		this->tS.push_back(new TuioServer(r->host, r->port));
  this->cursorList.push_back(new std::list<TuioCursor*>);
  if (ENABLED_NOTICE) printf("%s%s%s:%d\n", LOG_NOTICE, "Created TUIO Relay driver server at: ", r->host, r->port);
	}
	this->tTime = TuioTime::getSessionTime();
	for(unsigned int i = 0; i < LENGTH(relays); i++) this->tS[i]->initFrame(this->tTime);
	//this->tS->stopUntouchedMovingCursors();
	//this->tS->commitFrame();
 printf("%s", LOG_NOTICE);
}

std::string TuioRelay::generateValues(int iCount, ...) {
	va_list vList;
	va_start(vList, iCount);
	std::stringstream sValues;
	for (int i=0; i<iCount; i++) {
		if (i > 0) sValues << COMMAND_SEPERATOR;
		double fArg = va_arg(vList, double);
		sValues << fArg;
	}
	va_end(vList);

	return sValues.str();
}

std::string TuioRelay::generateCommand(const char * cName, const char * cAction, int iCID, long int iSID, std::string sValues) {
	char cCmd[BUFFER_LENGTH];
	sprintf(cCmd, COMMAND_STRUCTURE, cName, cAction, iCID, iSID, sValues.c_str());
	return (std::string)cCmd;
}

void TuioRelay::addTuioCursor(TuioCursor *tCur) {
	// Generating the command
	/*std::string sCmd = this->generateCommand(
		NAME_CURSOR, ACTION_ADD,
		tCur->getCursorID(), tCur->getSessionID(),
		this->generateValues(2,
			tCur->getX(), tCur->getY()
		)
	);

	// Debug output
	if (ENABLED_DEBUG) printf("%s%s", LOG_DEBUG, sCmd.c_str());*/
	this->netSend("2Dcur", 0, tCur->getSessionID(), tCur->getX(), tCur->getY(), 0.00, 0.00); // 0 = ADD

	// Relay the traffic
 //cursorList.push_back(new std::list<TuioCursor*>);
	for(unsigned int i = 0; i < LENGTH(relays); i++) {
  cursorList[i]->push_back(tS[i]->addTuioCursor(tCur->getX(), tCur->getY()));
  //this->tS[i]->addTuioCursor(tCur->getX(), tCur->getY());
  this->tS[i]->commitFrame();
 }
}

void TuioRelay::updateTuioCursor(TuioCursor *tCur) {
	// Generating the command
	/*std::string sCmd = this->generateCommand(
		NAME_CURSOR, ACTION_UPDATE,
		tCur->getCursorID(), tCur->getSessionID(),
		this->generateValues(4,
			tCur->getX(), tCur->getY(),
			tCur->getMotionSpeed(), tCur->getMotionAccel()
		)
	);

	// Debug output
	if (ENABLED_DEBUG) printf("%s%s", LOG_DEBUG, sCmd.c_str());*/
	this->netSend("2Dcur", 1, tCur->getSessionID(), tCur->getX(), tCur->getY(), tCur->getMotionSpeed(), tCur->getMotionAccel()); // 1 = UPDATE

	// Relay the traffic
        long s_id = tCur->getSessionID();
        std::list<TuioCursor*>::iterator it1;
        TuioCursor *it;
        for(unsigned int i = 0; i < LENGTH(relays); i++) {
        for (it1=cursorList[i]->begin(); it1!=cursorList[i]->end(); ++it1) {
                it = (*it1);
                if ((*it1)->getSessionID()==s_id) break;
        }
        it->update(TuioTime::getSessionTime(), tCur->getX(), tCur->getY());
        this->tS[i]->updateTuioCursor(it, it->getX(), it->getY());
        this->tS[i]->commitFrame();
        }
}

void TuioRelay::removeTuioCursor(TuioCursor *tCur) {
	// Generating the command
	/*std::string sCmd = this->generateCommand(
		NAME_CURSOR, ACTION_REMOVE,
		tCur->getCursorID(), tCur->getSessionID(),
		this->generateValues(0)
	);

	// Debug output
	if (ENABLED_DEBUG) printf("%s%s", LOG_DEBUG, sCmd.c_str());*/
	this->netSend("2Dcur", 2, tCur->getSessionID(), 0.00, 0.00, 0.00, 0.00); // 2 = REMOVE

	// Relay the traffic
	long s_id = tCur->getSessionID();
	std::list<TuioCursor*>::iterator it1;
	for(unsigned int i = 0; i < LENGTH(relays); i++) {
 TuioCursor *it;
	for (it1=cursorList[i]->begin(); it1!=cursorList[i]->end(); ++it1) {
		it = (*it1);
		if ((*it1)->getSessionID()==s_id) break;
	}
	//cursorList.remove(it);
	this->tS[i]->removeTuioCursor(it);
 //for(unsigned int i = 0; i < LENGTH(relays); i++) { printf("WORKS %d\n", i); this->tS[i]->removeTuioCursor(it); printf("REM %d\n", i);}
 cursorList[i]->remove(it);
 this->tS[i]->commitFrame();
 }
}

// TODO: Relay objects too
void TuioRelay::addTuioObject(TuioObject *tObj) {
	// Generating the command
	std::string sCmd = this->generateCommand(
		NAME_OBJECT, ACTION_ADD,
		tObj->getSymbolID(), tObj->getSessionID(),
		this->generateValues(3,
			tObj->getX(), tObj->getY(), tObj->getAngle()
		)
	);

	// Debug output
	if (ENABLED_DEBUG) printf("%s%s", LOG_DEBUG, sCmd.c_str());
	//this->netSend(sCmd);
}

void TuioRelay::updateTuioObject(TuioObject *tObj) {
	// Generating the command
	std::string sCmd = this->generateCommand(
		NAME_OBJECT, ACTION_UPDATE,
		tObj->getSymbolID(), tObj->getSessionID(),
		this->generateValues(7,
			tObj->getX(), tObj->getY(), tObj->getAngle(),
			tObj->getMotionSpeed(), tObj->getMotionAccel(),
			tObj->getRotationSpeed(), tObj->getRotationAccel()
		)
	);

	// Debug output
	if (ENABLED_DEBUG) printf("%s%s", LOG_DEBUG, sCmd.c_str());
	//this->netSend(sCmd);
}

void TuioRelay::removeTuioObject(TuioObject *tObj) {
	// Generating the command
	std::string sCmd = this->generateCommand(
		NAME_OBJECT, ACTION_REMOVE,
		tObj->getSymbolID(), tObj->getSessionID(),
		this->generateValues(0)
	);

	// Debug output
	if (ENABLED_DEBUG) printf("%s%s", LOG_DEBUG, sCmd.c_str());
	//this->netSend(sCmd);
}

void TuioRelay::refresh(TuioTime tFrameTime) {
	//this->tTime = TuioTime::getSessionTime();
	//this->tS->stopUntouchedMovingCursors();
	//this->tS->commitFrame();
	if (RELAY_REFRESH) {
		// Generating the command
		std::string sCmd = this->generateCommand(
			NAME_FRAME, ACTION_REFRESH,
			0, 0,
			this->generateValues(1, tFrameTime.getSeconds()) // TODO: Is this really useful?
		);

		// Debug output
		if (ENABLED_DEBUG) printf("%s%s", LOG_DEBUG, sCmd.c_str());
		//this->netSend(sCmd);

		// Relay the traffic
		this->tTime = tFrameTime;
	} else this->tTime = TuioTime::getSessionTime();
	//this->tS->initFrame(this->tTime);
}

int main(int argc, char* argv[]) {
	TuioRelay relay;
	TuioClient client(3333);
	client.addTuioListener(&relay);
	client.connect(true);
	return 0;
}
