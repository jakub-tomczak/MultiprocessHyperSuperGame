#define true 1
#define false 0

#define maxClients 20
#define maxRooms 10

int initialMessageKey = 9899;
int initialMessageSize = 14;
typedef struct
{
	long mtype;
	char mtext[10];
	int mClientsPID;
} InitialMessage;

int privateMessageSize = 10;
typedef struct 
{
	long mtype;
	char mtext[10];
} PrivateMessage;


 typedef struct  {
               struct ipc_perm msg_perm;     /* Ownership and permissions */
               time_t          msg_stime;    /* Time of last msgsnd(2) */
               time_t          msg_rtime;    /* Time of last msgrcv(2) */
               time_t          msg_ctime;    /* Time of last change */
               unsigned long   __msg_cbytes; /* Current number of bytes in
                                                queue (nonstandard) */
               msgqnum_t       msg_qnum;     /* Current number of messages
                                                in queue */
               msglen_t        msg_qbytes;   /* Maximum number of bytes
                                                allowed in queue */
               pid_t           msg_lspid;    /* PID of last msgsnd(2) */
               pid_t           msg_lrpid;    /* PID of last msgrcv(2) */
           } msqid_ds;

