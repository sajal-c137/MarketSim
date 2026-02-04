// MarketSim.cpp : Defines the entry point for the application.
//

#include "MarketSim.h"

using namespace std;

int main()
{
	cout << "Hello CMake." << endl;
	
	// Verify ZeroMQ is working
	zmq::context_t context(1);
	cout << "ZeroMQ context created successfully!" << endl;
	
	// Verify Protobuf is working
	cout << "Protobuf version: " << GOOGLE_PROTOBUF_VERSION << endl;
	
	return 0;
}
