/* Header with keys for CC_INFO Command */

#define INFO_EOB  0x0000
#define INFO_SKIP 0x2a2a
/* numeric values */

/* users values */
#define INFO_USERS_ONLINE 0x0001
#define INFO_USERS_ONLINE_DESCR "Users online"
#define INFO_USERS_SEEN   0x0002
#define INFO_USERS_SEEN_DESCR "Users seen"

#define INFO_PKT_IN  0x0010
#define INFO_PKT_IN_DESCR "Packets in"
#define INFO_PKT_IN_RESENDS 0x0011
#define INFO_PKT_IN_RESENDS_DESCR "Accepted resends"
#define INFO_PKT_IN_DROPRESENDS 0x0012
#define INFO_PKT_IN_DROPRESENDS_DESCR "Ignored too early resends"
#define INFO_PKT_IN_CORRUPTED 0x0013
#define INFO_PKT_IN_CORRUPTED_DESCR "Corrupted packets"
#define INFO_PKT_IN_BADKEY 0x0014
#define INFO_PKT_IN_BADKEY_DESCR "Droped because of bad key"
#define INFO_PKT_IN_IGNORED 0x0015
#define INFO_PKT_IN_IGNORED_DESCR "Droped because host is ignored"
#define INFO_PKT_IN_REJECTED 0x0016
#define INFO_PKT_IN_REJECTED_DESCR "Dropped because host is rejected"

#define INFO_PKT_OUT       0x0020
#define INFO_PKT_OUT_DESCR "Packets out"

#define INFO_TRAFFIC_IN    0x0030
#define INFO_TRAFFIC_IN_DESCR "Inbound traffic (MB)"
#define INFO_TRAFFIC_OUT   0x0031
#define INFO_TRAFFIC_OUT_DESCR "Outbound traffic (MB)"

#define INFO_NUMERIC_MAX 0x1fff

/* time values */
#define INFO_STARTUP 0x2000
#define INFO_STARTUP_DESCR "Server started"
#define INFO_STATSUP 0x2001
#define INFO_STATSUP_DESCR "Statistics last cleared"

#define INFO_TIME_MAX 0x3fff
/* string values */

/* server info */
#define INFO_SERVER_SOFTWARE 0x4000
#define INFO_SERVER_SOFTWARE_DESCR "Server software"
#define INFO_SERVER_NAME     0x4001
#define INFO_SERVER_NAME_DESCR "Server name"
#define INFO_SERVER_LOCATION 0x4002
#define INFO_SERVER_LOCATION_DESCR "Server location"
#define INFO_SERVER_OPERATOR 0x4003
#define INFO_SERVER_OPERATOR_DESCR "Server operator"

#define INFO_STRING_MAX 0x5fff
