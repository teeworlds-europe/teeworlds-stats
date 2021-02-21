#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <jansson.h>

#define _unused_ __attribute__((unused))
#define _cleanup_(x) __attribute__((__cleanup__(x)))
#define LOG_DEBUG(...) \
  do { \
    fprintf(stdout, "DEBUG:%s:%d: ", __FILE__, __LINE__); \
    fprintf(stdout, __VA_ARGS__); \
    fprintf(stdout, "\n"); \
  } while(0)
#define LOG_WARN(...) \
  do { \
    fprintf(stdout, "WARNING:%s:%d: ", __FILE__, __LINE__); \
    fprintf(stdout, __VA_ARGS__); \
    fprintf(stdout, "\n"); \
  } while(0)
#define LOG_ERROR(...) \
  do { \
    fprintf(stderr, "ERROR:%s:%d: ", __FILE__, __LINE__); \
    fprintf(stderr, __VA_ARGS__); \
    fprintf(stderr, "\n"); \
    exit(EXIT_FAILURE); \
  } while(0)

#define NET_PACKETFLAG_CONTROL 1
#define NET_CTRLMSG_TOKEN 5
#define NET_TOKENREQUEST_DATASIZE 512
#define NET_PACKETFLAG_CONNLESS 8
#define NET_PACKETVERSION 1
#define NETMSG_INFO 1
#define BUFFER_SIZE 8192
#define PACKET_GETINFO "\xff\xff\xff\xffgie3"
#define PACKET_INFO "\xff\xff\xff\xffinf3"
#define MAX_CLIENTS 128

struct packet_header {
  unsigned char flags_ack;    // 6bit flags, 2bit ack
  unsigned char ack;          // 8bit ack
  unsigned char numchunks;    // 8bit chunks
  unsigned char stoken[4];    // 32bit token
};

struct packet_control_data {
  unsigned char flags;
  unsigned char ctoken[4];
};

struct packet_connless_header {
  unsigned char flag_version;     // 6bit flags, 2bits version
  unsigned char token[4];         // 32bit token
  unsigned char rtoken[4]; // 32bit response token
};

struct client {
  unsigned char *name;
  unsigned char *clan;
  int country;
  int score;
  int team;
};

struct server_info {
  unsigned char *version;
  unsigned char *name;
  unsigned char *hostname;
  unsigned char *map;
  unsigned char *gametype;
  int num_players;
  int max_players;
  int num_clients;
  int max_clients;
  unsigned char flags;
  unsigned char skill;
  struct client clients[MAX_CLIENTS];
};

void pack_control_message(
  struct packet_header *pheader,
  struct packet_control_data *ctrldata,
  int32_t stoken,
  uint32_t ctoken
) {
  pheader->flags_ack = NET_PACKETFLAG_CONTROL << 2;
  pheader->ack = 0;
  pheader->numchunks = 0;
  pheader->stoken[0] = stoken >> 24;
  pheader->stoken[1] = stoken >> 16;
  pheader->stoken[2] = stoken >> 8;
  pheader->stoken[3] = stoken;

  ctrldata->flags = NET_CTRLMSG_TOKEN;
  ctrldata->ctoken[0] = ctoken >> 24;
  ctrldata->ctoken[1] = ctoken >> 16;
  ctrldata->ctoken[2] = ctoken >> 8;
  ctrldata->ctoken[3] = ctoken;
}

uint32_t unpack_server_token(struct packet_header *pheader) {
  uint32_t token = 0;
  token |= pheader->stoken[0] << 24;
  token |= pheader->stoken[1] << 16;
  token |= pheader->stoken[2] << 8;
  token |= pheader->stoken[3];
  return token;
}

uint32_t unpack_client_token(struct packet_control_data *ctrldata) {
  uint32_t token = 0;
  token |= ctrldata->ctoken[0] << 24;
  token |= ctrldata->ctoken[1] << 16;
  token |= ctrldata->ctoken[2] << 8;
  token |= ctrldata->ctoken[3];
  return token;
}

void pack_connless_message(
  struct packet_connless_header *pheader,
  uint32_t stoken,
  uint32_t ctoken
) {
  pheader->flag_version = (NET_PACKETFLAG_CONNLESS <<2 ) | NET_PACKETVERSION;
  pheader->token[0] = stoken >> 24;
  pheader->token[1] = stoken >> 16;
  pheader->token[2] = stoken >> 8;
  pheader->token[3] = stoken;

  pheader->rtoken[0] = ctoken >> 24;
  pheader->rtoken[1] = ctoken >> 16;
  pheader->rtoken[2] = ctoken >> 8;
  pheader->rtoken[3] = ctoken;
}

unsigned char *unpack_integer(unsigned char *intsrc, int *out, unsigned char *end)
{
  const char *errmsg = "Detected out of bound read while parsing integer";
  int sign = (*intsrc>>6)&1;
  *out = *intsrc&0x3F;
  do
  {
    if(!(*intsrc & 0x80)) break;
    if (++intsrc > end) LOG_ERROR(errmsg);
    *out |= (*intsrc & 0x7F) << 6;

    if(!(*intsrc & 0x80)) break;
    if (++intsrc > end) LOG_ERROR(errmsg);
    *out |= (*intsrc & 0x7F) << (6 + 7);

    if(!(*intsrc & 0x80)) break;
    if (++intsrc > end) LOG_ERROR(errmsg);
    *out |= (*intsrc & 0x7F) << (6 + 7 + 7);

    if(!(*intsrc & 0x80)) break;
    if (++intsrc > end) LOG_ERROR(errmsg);
    *out |= (*intsrc & 0x7F) << (6 + 7 + 7 + 7);
  } while(0);

  intsrc++;
  *out ^= -sign; // if(sign) *i = ~(*i)
  return intsrc;
}

void print_json(struct server_info *srvinfo) {
  json_t *jsonroot = json_pack(
    "{ss ss ss ss ss si si si si si si s[]}",
    "name", srvinfo->name,
    "version", srvinfo->version,
    "hostname", srvinfo->hostname,
    "map", srvinfo->map,
    "gametype", srvinfo->gametype,
    "num_players", srvinfo->num_players,
    "max_players", srvinfo->max_players,
    "num_clients", srvinfo->num_clients,
    "max_clients", srvinfo->max_clients,
    "skill", srvinfo->skill,
    "flags", srvinfo->flags,
    "clients"
  );
  json_t *jsonclients = json_object_get(jsonroot, "clients");
  for (int i = 0; i < srvinfo->num_clients; i++) {
    json_t *jsonclient = json_pack(
      "{ss ss si si si}",
      "name", srvinfo->clients[i].name,
      "clan", srvinfo->clients[i].clan,
      "score", srvinfo->clients[i].score,
      "country", srvinfo->clients[i].country,
      "team", srvinfo->clients[i].team
    );
    json_array_append(jsonclients, jsonclient);
    json_decref(jsonclient);
  }
  char *jsonout = json_dumps(jsonroot, JSON_COMPACT | JSON_ESCAPE_SLASH);
  if (jsonout == NULL) {
    LOG_ERROR("Failed to construct output JSON");
  }
  printf("%s\n", jsonout);
  free(jsonout);
  json_decref(jsonroot);
}

int main(int argc, char **argv) {
  struct iovec outvecs[3];
  struct iovec invec;
  struct packet_header pheader;
  struct packet_control_data ctrldata;
  struct packet_connless_header cpheader;
  struct msghdr msghdr;
  unsigned char rbuf[BUFFER_SIZE];
  unsigned char tkreqdata[NET_TOKENREQUEST_DATASIZE];

  if (argc < 2) {
    LOG_ERROR("Provide server address and port as arguments");
  }
  struct addrinfo *srvaddress;
  if (getaddrinfo(argv[1], argv[2], NULL, &srvaddress) != 0) {
    LOG_ERROR("Failed to resolve name: %s", argv[1]);
  }

  int srvsock = socket(AF_INET, SOCK_DGRAM, 0);
  if (srvsock == -1) LOG_ERROR("Failed to create UDP socket: %s", strerror(errno));
  msghdr.msg_name = srvaddress->ai_addr;
  msghdr.msg_namelen = srvaddress->ai_addrlen;
  msghdr.msg_iov = outvecs;
  msghdr.msg_control = NULL;
  msghdr.msg_controllen = 0;
  msghdr.msg_flags = 0;

  msghdr.msg_iovlen = 3;
  uint32_t ctoken = rand();
  pack_control_message(&pheader, &ctrldata, -1, ctoken);
  outvecs[0].iov_base = &pheader;
  outvecs[0].iov_len = sizeof(pheader);
  outvecs[1].iov_base = &ctrldata;
  outvecs[1].iov_len = sizeof(ctrldata);
  outvecs[2].iov_base = tkreqdata;
  outvecs[2].iov_len = NET_TOKENREQUEST_DATASIZE;
  memset(tkreqdata, 0, NET_TOKENREQUEST_DATASIZE);
  if (sendmsg(srvsock, &msghdr, 0) == -1) {
    LOG_ERROR("Failed to send message to server: %s", strerror(errno));
  }

  invec.iov_base = rbuf;
  invec.iov_len = BUFFER_SIZE;
  msghdr.msg_iov = &invec;
  msghdr.msg_iovlen = 1;
  ssize_t rlen;
  if ((rlen = recvmsg(srvsock, &msghdr, 0)) == -1) {
    LOG_ERROR("Failed to receive message from server: %s", strerror(errno));
  }
  memcpy(&pheader, rbuf, sizeof(pheader));
  memcpy(&ctrldata, rbuf + sizeof(pheader), sizeof(ctrldata));
  uint32_t stoken = unpack_client_token(&ctrldata);
  uint32_t rctoken = unpack_server_token(&pheader);
  if (ctoken != rctoken) {
    LOG_ERROR("Server responded with wrong server token");
  }
  ctoken = rctoken;

  pack_connless_message(&cpheader, stoken, ctoken);
  msghdr.msg_iov = outvecs;
  msghdr.msg_iovlen = 2;
  outvecs[0].iov_base = &cpheader;
  outvecs[0].iov_len = sizeof(cpheader);
  outvecs[1].iov_base = PACKET_GETINFO;
  outvecs[1].iov_len = strlen(PACKET_GETINFO) + 1;
  if (sendmsg(srvsock, &msghdr, 0) == -1) {
    LOG_ERROR("Failed to send message to server: %s", strerror(errno));
  }

  invec.iov_base = rbuf;
  invec.iov_len = BUFFER_SIZE;
  msghdr.msg_iov = &invec;
  msghdr.msg_iovlen = 1;
  if ((rlen = recvmsg(srvsock, &msghdr, 0)) == -1) {
    LOG_ERROR("Failed to receive message from server: %s", strerror(errno));
  }
  struct packet_connless_header ecpheader;
  pack_connless_message(&ecpheader, ctoken, stoken);
  if (memcmp(&ecpheader, rbuf, sizeof(ecpheader)) != 0) {
    LOG_ERROR("Received wrong header from server");
  }
  if (memcmp(PACKET_INFO, rbuf + sizeof(ecpheader), strlen(PACKET_INFO) + 1) != 0) {
    LOG_ERROR("Didn't receive status info from server");
  }
  unsigned char *cur = rbuf + sizeof(ecpheader) + sizeof(PACKET_INFO);
  unsigned char *bufend = rbuf + rlen;
  int proplen = 0;
  struct server_info srvinfo;
  for (int i = 0; i < 5; i++) {
    if (cur > bufend) break;
    while (cur <= bufend && *cur != '\0') {
      cur++;
      proplen++;
    }
    switch (i) {
      case 0:
        srvinfo.version = cur - proplen;
        break;
      case 1:
        srvinfo.name = cur - proplen;
        break;
      case 2:
        srvinfo.hostname = cur - proplen;
        break;
      case 3:
        srvinfo.map = cur - proplen;
        break;
      case 4:
        srvinfo.gametype = cur - proplen;
        break;
    }
    cur++;
    proplen = 0;
  }
  const char *oobmsg = "Detected out-of-bounds read, while parsing server status";
  if (++cur > bufend) LOG_ERROR(oobmsg);
  srvinfo.flags = *cur;
  if (++cur > bufend) LOG_ERROR(oobmsg);
  srvinfo.skill = *cur;
  cur = unpack_integer(cur, &srvinfo.num_players, bufend);
  cur = unpack_integer(cur, &srvinfo.max_players, bufend);
  cur = unpack_integer(cur, &srvinfo.num_clients, bufend);
  cur = unpack_integer(cur, &srvinfo.max_clients, bufend);
  for (int nclient = 0; nclient < srvinfo.num_clients; nclient++) {
    proplen = 0;
    for (int i = 0; i < 2; i++) {
      if (cur > bufend) break;
      while (cur <= bufend && *cur != '\0') {
        cur++;
        proplen++;
      }
      switch (i) {
        case 0:
          srvinfo.clients[nclient].name = cur - proplen;
          break;
        case 1:
          srvinfo.clients[nclient].clan = cur - proplen;
          break;
      }
      cur++;
      proplen = 0;
    }
    cur = unpack_integer(cur, &srvinfo.clients[nclient].country, bufend);
    cur = unpack_integer(cur, &srvinfo.clients[nclient].score, bufend);
    cur = unpack_integer(cur, &srvinfo.clients[nclient].team, bufend);
  }
  print_json(&srvinfo);

  close(srvsock);
  freeaddrinfo(srvaddress);
  return 0;
}
