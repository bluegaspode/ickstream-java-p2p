//
//  ickDiscoveryInternal.h
//  ickStreamProto
//
//  Created by Jörg Schwieder on 13.02.12.
//  Copyright (c) 2012 Du!Business GmbH. All rights reserved.
//

#ifndef ickStreamProto_ickDiscoveryInternal_h
#define ickStreamProto_ickDiscoveryInternal_h

#include <stdio.h>
//#include <ifaddrs.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/queue.h>
#include <ctype.h>


#ifdef DEBUG
static inline
void debug(const char *format, ...)
{
	va_list ap;
	va_start(ap, format); vfprintf(stderr, format, ap); va_end(ap);
}
#else
static inline
void debug(const char *format, ...)
{
}
#endif


/*
 * Mac OSX as well as iOS do not define the MSG_NOSIGNAL flag,
 * but happily have something equivalent in the SO_NOSIGPIPE flag.
 */
#ifdef __APPLE__
#define MSG_NOSIGNAL SO_NOSIGPIPE 
#endif


// Some UPNP Standards

#define XSTR(s) STR(s)
#define STR(s) #s

#define UPNP_PORT       1900
#define UPNP_MCAST_ADDR "239.255.255.250"
#define LOCALHOST_ADDR  "127.0.0.1"

struct _upnp_device;

// callback functions for received discovery messages have this type
typedef void (* receive_callback_t)(const struct _upnp_device * device, enum ickDiscovery_command);


struct _ick_callback_list {
    struct _ick_callback_list * next;
    receive_callback_t callback;
};

struct _reference_list {
    struct _reference_list * next;
    
    enum ickDevice_servicetype type;
    char * UUID;
    char * URL;
    
    void *  element;
};


// strcut defining the discovery handler.
struct _ick_discovery_struct {
    int         lock;
    pthread_t   thread;
    int         socket;
    
    char *      UUID;
    char *      interface;
    char *      location;
    char *      osname;
    enum ickDevice_servicetype services;
    
    struct _ick_callback_list * receive_callbacks;
};


//
//  Functions
//


// receiver callsbacks for discovery messages
// in ickDiscoveryRegistry.c

extern void _ick_receive_notify(const struct _upnp_device * device, enum ickDiscovery_command cmd);



// minissdp modified functions and internal maintenance

extern int ParseSSDPPacket(const struct _ick_discovery_struct * discovery, const char * p, ssize_t n, const struct sockaddr * addr);

/* 
 from minisspd
 device data structures */
struct _header_string {
	const char * p; /* string pointer */
	int l;          /* string length */
};

#define HEADER_NT	0
#define HEADER_USN	1
#define HEADER_LOCATION	2

// UPnP devices found

struct _upnp_device {
	struct _upnp_device * next;
	time_t t;                 /* validity time */
	struct _header_string headers[3]; /* NT, USN and LOCATION headers */
	char data[];
};


/* Services stored for answering to M-SEARCH
   For ickStream, devices are registered like services, but if the service is "server" more than one service per device may have to be registered */
struct _upnp_service {
	char * st;	/* Service type */
	char * usn;	/* Unique identifier */
	char * server;	/* Server string */
	char * location;	/* URL */
	LIST_ENTRY(_upnp_service) entries;
};


#define LIST_VALIDATE_PRESENT(head, iterator, object, field) do { \
iterator = LIST_FIRST(head); \
while (iterator) { \
if ((iterator) == (object)) \
break; \
(iterator) = LIST_NEXT((iterator), field); \
} \
(object) = (iterator); \
} while (0)

// device type strings

#define ICKDEVICE_TYPESTR_MISC          "urn:schemas-ickstream-com:device:"
#define ICKDEVICE_TYPESTR_ROOT          "urn:schemas-ickstream-com:device:Root:1"
#define ICKDEVICE_TYPESTR_PLAYER        "urn:schemas-ickstream-com:device:Player:1"
//#define ICKDEVICE_TYPESTR_PLAYER        "urn:schemas-upnp-org:device:MediaRenderer:1"
#define ICKDEVICE_TYPESTR_CONTROLLER    "urn:schemas-ickstream-com:device:Controller:1"
#define ICKDEVICE_STRING_PLAYER         "Player"
#define ICKDEVICE_STRING_CONTROLLER     "Controller"
#define ICKDEVICE_STRING_ROOT           "Root"

#define ICKDEVICE_TYPESTR_USN           "uuid:%s::%s"       // 1st string: UUID, 2nd string: device URN
#define ICKDEVICE_TYPESTR_LOCATION      "http://%s:9/%s.xml"       // Port 9 is "discard". We need to replace this with something sensible once we enable description XML downloads

#define ICKDEVICE_TYPESTR_SERVERSTRING  "SERVER: %s UPnP/1.1 ickStream/1.0"


/* Commands to be used to feed the notification and search message queue */

enum _ick_send_cmd {
    ICK_SEND_CMD_NONE,
    ICK_SEND_QUIT,
    ICK_SEND_CMD_SEARCH,
    ICK_SEND_CMD_NOTIFY_REMOVE,
    ICK_SEND_CMD_NOTIFY_PERIODICADD,
    ICK_SEND_CMD_NOTIFY_PERIODICSEARCH,
    ICK_SEND_CMD_NOTIFY_ADD
};



void _ick_init_discovery_registry (const char * UUID, const char * location, char * osname);
void _ick_close_discovery_registry (int wait);

int _ick_add_service (const char * st, const char * usn, const char * server, const char * location);
int _ick_remove_service(const char * st);
int _ick_notifications_send (enum _ick_send_cmd command, struct _upnp_service * service);



#endif
