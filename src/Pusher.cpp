#include "Pusher.h"

/*
	[scheme]://ws-[cluster_name].pusher.com:[port]/app/[key]
	establish a websocket connection to the cluster
*/
static const String getFullUrl(const char *key, const char *cluster) {
	String fullUrl = "ws://ws-";
	fullUrl += cluster;
	fullUrl += ".pusher.com:80/app/";
	fullUrl += key;
	fullUrl += "?client=js&version=7.0.3&protocol=5";
	return fullUrl.c_str();
}

PusherService::PusherService(
	const char *key,
	const char *cluster,
	void (*log)(const char *)
): WebSocketService(getFullUrl(key, cluster), log)
{
	this->socket_id.reserve(20);
}

PusherService::~PusherService() {}


static const String generateSubMessage(
	const char *channel
) {
    String message;
	message.reserve(100);
	message += "{\"event\":\"pusher:subscribe\",\"data\":{\"channel\":\"";
    message += channel;
    message += "\"}}";
    return message;
}

static const String generateSubMessagePresenceChannel(
	const char *channel,
	const String &data
) {
	String message;
	message.reserve(100);
	message += "{\"event\":\"pusher:subscribe\",\"data\":{\"channel\":\"";
	message += channel;
	message += "\",";
	message += data.substring(1);
	message += "}";
	return message;
}

void PusherService::Subscribe(
	const char *channel,
	const String &signature
) {
	if (signature.length() > 0) {
		const String mesage = generateSubMessagePresenceChannel(channel, signature).c_str();
		Serial.println("Sending message: " + mesage);
		this->send(mesage);
	} else {
    	this->send(generateSubMessage(channel).c_str());
	}
}

static const String formMessage(
	const char *event,
	const char *channel,
	const char *data
) {
	String message;
	message.reserve(100);
	message += "{\"event\":\"";
	message += event;
	message += "\",\"channel\":\"";
	message += channel;
	message += "\",\"data\":";
	message += data;
	message += "}";
	return message;
}

void PusherService::sendMessage(
	const char *event,
	const char *channel,
	const char *data
) {
	const String message = formMessage(event, channel, data);
	Serial.println("Sending message: " + message);
	this->send(message.c_str());
}
