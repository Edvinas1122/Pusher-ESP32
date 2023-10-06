#include <Pusher.h>
#include <Wifi.h>

// add key
const char pusher_key[] = "your-pusher-key-here";
// add cluster
const char pusher_cluster[] = "your-pusher-cluster-here";

PusherService socket(
	pusher_key,
	pusher_cluster,
	// enable logging
	[](const char* message) {Serial.print(message);}
);

/*
	Register event handlers
*/
void registerHandlers(
	// register dependencies
	PusherService &socket,
	const char *mainChannel
) {
	socket.registerEventHandler(
		subscriptionSucceededEvent,
		[](const String& message) {
		// pass behavuours here
		Serial.println("subscriptionSucceededEvent");
	});
}

/*
	Subscribe to a public channel
*/
void autoSubscribeToAPublicChannel(
	const char *mainChannel
) {
		socket.registerEventHandler(
		connectionEvent,
		[mainChannel](const String& message) {
			socket.Subscribe(
				mainChannel
			);
	});
};


/*
	Pseudo code for requestAuthorize
*/
const String requestAuthorize(const char* data[]) {
	HTTPClient http;
	// http.setInsecure(true); // must solve HTTPS issue. either use http either contain certificate either use insecure
	http.begin("http://yourdomain:3000/pusher/auth");
	http.addHeader("Content-Type", "application/json");
	int httpCode = http.POST("{\"socket_id\":\"" + String(data[0]) + "\",\"channel_name\":\"" + String(data[1]) + "\"}");
	String payload = http.getString();
	http.end();
	return payload;
}

/*
	https://pusher.com/docs/channels/server_api/authorizing-users/
	subs to presence channel as Pusher only allows sending messages on
	presence and private channels.
	HTTPInterface is a dependency for an action because,
	we need to make a request to our web service to get authourization.
*/
void autoSubscribeToPresenceChannel(
	// your requestAuthorize method
	const String (*requestAuthorize)(const char*[]),
	PusherService &socket,
	const char *mainChannel
) {
		socket.registerEventHandler(
		connectionEvent,
		[&socket, mainChannel, requestAuthorize](const String& message) {
			// set socket id like this
			socket.socket_id = PusherService::Message(message.c_str())
									.getItem("socket_id");
			// prepare data for requestAuthorize on your server
			const char* data[] = {socket.socket_id.c_str(),
									mainChannel, NULL };
			// get response from your server
			const String response = requestAuthorize(data);
			// subscribe to channel
			socket.Subscribe(
				mainChannel,
				response
			);
	});
};

/*
	Use your methods to ensure that esp is connected to a network
*/
static void ensureConnected() {
	const char* ssid = "ssid"; //Enter SSID
	const char* password = "password"; //Enter Password

	WiFi.begin(ssid, password);
    for(int i = 0; i < 10 && WiFi.status() != WL_CONNECTED; i++) {
        Serial.print(".");
        delay(1000);
    }
    if(WiFi.status() != WL_CONNECTED) {
        Serial.println("No Wifi!");
        return;
    }
	Serial.println("Connected to Wifi, Connecting to server.");
}


/*
	Currently suporing one channel and
	only auto subscribing on connection to a pusher server.
*/
const char *mainChannel = "presence-channel";

void setup()
{
	Serial.begin(115200);
	ensureConnected();
	autoSubscribeToAPublicChannel(mainChannel);
	registerHandlers(socket, mainChannel);
}

void setup()
{
	socket.poll();
	delay(200);
}