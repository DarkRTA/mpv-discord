#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#include <mpv/client.h>
#include "discord_rpc.h"

int mpv_command_ret(mpv_handle *ctx, char *args[], mpv_node *result) ;

//yeah
static void nullfunc() {}

static void initrpc() {
	DiscordEventHandlers handlers = {
		.ready		= nullfunc,
		.disconnected	= nullfunc,
		.errored        = nullfunc,
		.joinGame       = nullfunc,
		.spectateGame   = nullfunc,
		.joinRequest    = nullfunc,
	};
	Discord_Initialize("599691476057849878", &handlers, 1, NULL);
}

static void update(mpv_handle *handle) {
	//idle by default
	char details[64] = "(idle)";
	char state[64] = "(no file)";
	DiscordRichPresence presence = {
		.details	= details,
		.state		= state,
		.endTimestamp	= 0,
	};

	int idle;
	mpv_get_property(handle, "idle-active", MPV_FORMAT_FLAG, &idle);
	if (!idle) {
		//check if the player is paused
		int paused;
		mpv_get_property(handle, "pause", MPV_FORMAT_FLAG, &paused);
		
		mpv_node result;
		char *args[] = {
			"expand-text",
			NULL,
			NULL
		};


		//current file
		args[1] = "${?pause==yes:(paused) }${media-title}";
		int rc = mpv_command_ret(handle, args, &result);
		if (rc >= 0 && result.u.string != NULL) {
			strncpy(details, result.u.string, sizeof(details));
			mpv_free_node_contents(&result);
		}

		//state
		args[1] = "${metadata/by-key/artist}";

		rc = mpv_command_ret(handle, args, &result);
		if (rc >= 0 && result.u.string != NULL) {
			strncpy(state, result.u.string, sizeof(state));
			mpv_free_node_contents(&result);
		}

		//show the timestamp if applicable
		if (!paused) {
			long int timer = time(NULL);
			long int time_remaining;
			mpv_get_property(handle, "time-remaining", 
					 MPV_FORMAT_INT64, &time_remaining);
			presence.endTimestamp = timer + time_remaining;
		}
	}

	Discord_UpdatePresence(&presence);
}

int mpv_open_cplugin(mpv_handle *handle)
{
	initrpc();

	//mpv_observe_property(handle, 0, "pause", MPV_FORMAT_NONE);
	mpv_event *event = NULL;
	while(1) {
		event = mpv_wait_event(handle, -1);

		switch(event->event_id) {
		case MPV_EVENT_SHUTDOWN:
			return 0;
		case MPV_EVENT_NONE:
			break;
		case MPV_EVENT_PAUSE:
		case MPV_EVENT_UNPAUSE:
		case MPV_EVENT_FILE_LOADED:
		case MPV_EVENT_SEEK:
		case MPV_EVENT_IDLE:
			update(handle);
			break;
		default:
			break;
		}
	}
}

