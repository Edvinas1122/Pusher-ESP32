# Pusher-ESP32

An improvised solution for connecting an ESP32 to a [https://pusher.com](https://pusher.com) cloud service.
More about pusher.com web sockets api [here](https://pusher.com/docs/channels/using_channels/client-api-overview/).

## About

Pusher is an online web-socket message broker providing 200,000 messages per day free of charge, proving as a viable solution for small less frequent updated real-time applications.

### Event driven

Due to nature of duplex communication, its best to follow *event driven design*.
It increase amount of function pointers as we often pass callBacks methods.

### Important notes

1. For **device to send a ws message, it must be authorized by your server**. You can start  to use, but for function you need to set your authorization endpoint.
2. Cloud server allow https only, so consider auth strategies.

## Start Use

### Initiate

Initiate a class, provide your pusher app credentials.
You can get pusher app credentials [here](https://dashboard.pusher.com/accounts/sign_up).

```cpp
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
```

Currently only ws supported. wss would come for a demand.

### Register event handlers

```cpp
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
```

### Register auto subscribe

You can subscribe to public channels like this

```cpp
const char mainChannel[] = "a_channel_name"
void autoSubscribeToChannel(
	PusherService &socket,
) {
		socket.registerEventHandler(
		connectionEvent,
		[](const String& message)
			socket.Subscribe(mainChannel);
	);
};
```

### Ensure network

You must ensure that ESP32 is connected on wifi or a network.

```cpp
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
```

### Poll

It is a poll based solution, set a poll inside a thread for handlers to function. Of course do not block poll. 200ms block handles hooks function.

```cpp
void setup()
{
	socket.poll();
	delay(200);
}
```

## Expand use

Pusher does only allow for clients to send messages in private and presence channels.
Read more about authorization [here](https://pusher.com/docs/channels/server_api/authorizing-users/).

### Authorization

Channels must start with prefix *“presence-”* or *“private-”*

```cpp
const char *mainChannel = "presence-channel";
```

Once you subscribe to a channel you get a message with which you have to parse and send to your authorization endpoint

```cpp
void autoSubscribeToChannel(
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
```

Here is a snippet of pseudo code for sending a POST request to your server

```cpp
/*
	Pseudo code for requestAuthorize
*/
const String requestAuthorize(const char* data[]) {
	HTTPClient http;
	// http.setInsecure(true); // must solve HTTPS issue. either use http either contain certificate either use insecure
	http.begin("http://yourserver:3000/pusher/auth");
	http.addHeader("Content-Type", "application/json");
	int httpCode = http.POST("{\"socket_id\":\"" + String(data[0]) + "\",\"channel_name\":\"" + String(data[1]) + "\"}");
	String payload = http.getString();
	http.end();
	return payload;
}
```

Pseudo code for an authorization endpoint.

```ts
async function routeHandler(params: any) {
	console.log("API, pusher authorization request received:");
	const user = getUserFromHeaders(); // middleware passed body to headers
	if (!user.id || !user.name) return Respond({message: 'not authorized'}, 403);
	const presenceData = {
		user_id: user.id,
		user_info: {
			name: user.name,
		},
	};
	const socketId = params.socket_id;
	const channelName = params.channel_name;
	const pusher = new PusherServer.default(getPusherConfig());
	const authResponse = pusher.authorizeChannel(socketId, channelName, presenceData);
	console.log("API, pusher authorization response:");
	console.log(authResponse);
	return new Response(JSON.stringify(authResponse), {
		status: 200,
		headers: {
			'Content-Type': 'application/json',
		}
	});
}

export const POST = BodyParameters<ValidationBody>([
	"socket_id", "channel_name"
], routeHandler);
```

[Pusher Channels Docs | How to authorize users](https://pusher.com/docs/channels/server_api/authorizing-users/#implementing-the-authorization-endpoint-for-a-presence-channel)

### Send Messages

If you managed to connect, you can send messages on the subscribed channel, just, use *“clinet-”* prefix.

```cpp
socket.sendMessage("client-pong",
				mainChannel,
				"{\"message\":\"online\"}"
			);
```

Use sendMessage in event handler hooks, to follow *event driven design*.

## Collaborate

Reach out on [https://twitter.com/EdvinasMomkus](https://twitter.com/EdvinasMomkus) for any inquries, comments, or notes. I am mostly free and can communicate on discord.

Further we can support private channels, and improve error handling.