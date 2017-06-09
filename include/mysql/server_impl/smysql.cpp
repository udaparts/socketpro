

#include "mysqlimpl.h"
#include "../../mysql/mysql/client_plugin.h"


static int async_sql_plugin_init(void *p)
{
	my_plugin_log_message(&p, MY_INFORMATION_LEVEL, "Installation");
	return 0;
}

static int async_sql_plugin_deinit(void *p)
{
  my_plugin_log_message(&p, MY_INFORMATION_LEVEL, "Uninstallation");
  return 0;
}

st_mysql_daemon async_sql_plugin = {
	MYSQL_DAEMON_INTERFACE_VERSION 
};

mysql_declare_plugin(async_sql)
{
  MYSQL_DAEMON_PLUGIN,
  &async_sql_plugin,
  "UDAParts_Async_SQL",
  "UDAParts",
  "Async SQL Real-time Streaming Processing",
  PLUGIN_LICENSE_PROPRIETARY,
  async_sql_plugin_init,		/* Plugin Init */
  async_sql_plugin_deinit,		/* Plugin Deinit */
  0x0100						/* 1.0 */,
  NULL,							/* status variables                */
  NULL,							/* system variables                */
  NULL,							/* config options                  */
  0,							/* flags                           */
}
mysql_declare_plugin_end;


/********************* CLIENT SIDE ***************************************/

#define ORDINARY_QUESTION       "\2"
#define LAST_QUESTION           "\3"
#define LAST_PASSWORD           "\4"
#define PASSWORD_QUESTION       "\5"

/**
  The main function of the test plugin.

  Reads the prompt, check if the handshake is done and if the prompt is a
  password request and returns the password. Otherwise return error.

  @note
   1. this plugin shows how a client authentication plugin
      may read a MySQL protocol OK packet internally - which is important
      where a number of packets is not known in advance.
   2. the first byte of the prompt is special. it is not
      shown to the user, but signals whether it is the last question
      (prompt[0] & 1 == 1) or not last (prompt[0] & 1 == 0),
      and whether the input is a password (not echoed).
   3. the prompt is expected to be sent zero-terminated
*/
static int async_auth_plugin_client(MYSQL_PLUGIN_VIO *vio, MYSQL *mysql)
{
  unsigned char *pkt, cmd= 0;
  int pkt_len, res;
  char *reply;

  do
  {
    /* read the prompt */
    pkt_len= vio->read_packet(vio, &pkt);
    if (pkt_len < 0)
      return CR_ERROR;

    if (pkt == 0)
    {
      /*
        in mysql_change_user() the client sends the first packet, so
        the first vio->read_packet() does nothing (pkt == 0).

        We send the "password", assuming the client knows what it's doing.
        (in other words, the dialog plugin should be only set as a default
        authentication plugin on the client if the first question
        asks for a password - which will be sent in clear text, by the way)
      */
      reply= mysql->passwd;
    }
    else
    {
      cmd= *pkt++;

      /* is it MySQL protocol (0=OK or 254=need old password) packet ? */
      if (cmd == 0 || cmd == 254)
        return CR_OK_HANDSHAKE_COMPLETE; /* yes. we're done */

      /*
        asking for a password with an empty prompt means mysql->password
        otherwise return an error
      */
      if ((cmd == LAST_PASSWORD[0] || cmd == PASSWORD_QUESTION[0]) && *pkt == 0)
        reply= mysql->passwd;
      else
        return CR_ERROR;
    }
    if (!reply)
      return CR_ERROR;
    /* send the reply to the server */
    res= vio->write_packet(vio, (const unsigned char *) reply, 
                           (int)strlen(reply) + 1);

    if (res)
      return CR_ERROR;

    /* repeat unless it was the last question */
  } while (cmd != LAST_QUESTION[0] && cmd != PASSWORD_QUESTION[0]);

  /* the job of reading the ok/error packet is left to the server */
  return CR_OK;
}

/*
mysql_declare_client_plugin(AUTHENTICATION)
  "UDAParts_MySQL_Auth_Plugin",
  "UDParts",
  "Dialog Client Authentication Plugin",
  {0,1,0},
  "UDAParts License",
  NULL,
  NULL,
  NULL,
  NULL,
  async_auth_plugin_client
mysql_end_client_plugin;
*/