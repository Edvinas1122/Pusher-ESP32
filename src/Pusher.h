#ifndef PUSHER_H
# define PUSHER_H

#include <Arduino.h>
#include <functional>
#include <unordered_map>
#include <functional>
#include <WebSocketService.h>

constexpr char connectionEvent[] = "pusher:connection_established";
constexpr char subscriptionSucceededEvent[] = "pusher_internal:subscription_succeeded";
constexpr char errorEvent[] = "pusher:error";
constexpr char memberAddedEvent[] = "pusher_internal:member_added";
constexpr char memberRemovedEvent[] = "pusher_internal:member_removed";


/*
	https://pusher.com/docs/channels/library_auth_reference/pusher-websockets-protocol/
*/
class PusherService: 
	public WebSocketService
{
	public:
		PusherService(
			const char *key,
			const char *cluster,
			void (*log)(const char *) = defaultLog
		);
		virtual ~PusherService();


		void sendMessage(
			const char *event,
			const char *channel,
			const char *data
		);

		void Subscribe(
			const char *,
			const String &signature = ""
		);
		String	socket_id;
};

#endif