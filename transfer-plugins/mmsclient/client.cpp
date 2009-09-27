/*
 * mms://netshow.msn.com/msnbc8 
 * mms://216.106.172.144/bbc1099/ads/ibeam/0_ibeamEarth_aaab00020_15_350k.asf
 * mms://195.124.124.82/56/081001_angriffe_1200.wmv
 * mms://193.159.244.12/n24_wmt_mid
 */

#include <unistd.h>
#include <stdio.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "bswap.h"

#include <QThread>
#include <QString>

#ifdef __CYGWIN__
typedef unsigned char uint8_t;
typedef unsigned int uint32_t;
typedef unsigned long long int uint64_t;
#else
#include <inttypes.h>
#endif

#define BUF_SIZE 102400


class MMSClient
{
    public:
    MMSClient() {
        seq_num = 0;
        num_stream_ids;
        output_fh = 0;
    }
        
        
    typedef struct {

    uint8_t buf[BUF_SIZE];
    int     num_bytes;

    } command_t;

    void put_32 (command_t *cmd, uint32_t value) {

    cmd->buf[cmd->num_bytes  ] = value % 256;
    value = value >> 8;
    cmd->buf[cmd->num_bytes+1] = value % 256 ;
    value = value >> 8;
    cmd->buf[cmd->num_bytes+2] = value % 256 ;
    value = value >> 8;
    cmd->buf[cmd->num_bytes+3] = value % 256 ;

    cmd->num_bytes += 4;
    }

    uint32_t get_32 (unsigned char *cmd, int offset) {

    uint32_t ret;

    ret = cmd[offset] ;
    ret |= cmd[offset+1]<<8 ;
    ret |= cmd[offset+2]<<16 ;
    ret |= cmd[offset+3]<<24 ;

    return ret;
    }

    void send_command (int s, int command, uint32_t switches, 
                uint32_t extra, int length,
                char *data) {
    
    command_t  cmd;
    int        len8;
    int        i;

    len8 = (length + (length%8)) / 8;

    cmd.num_bytes = 0;

    put_32 (&cmd, 0x00000001); /* start sequence */
    put_32 (&cmd, 0xB00BFACE); /* #-)) */
    put_32 (&cmd, length + 32);
    put_32 (&cmd, 0x20534d4d); /* protocol type "MMS " */
    put_32 (&cmd, len8 + 4);
    put_32 (&cmd, seq_num);
    seq_num++;
    put_32 (&cmd, 0x0);        /* unknown */
    put_32 (&cmd, 0x0);
    put_32 (&cmd, len8+2);
    put_32 (&cmd, 0x00030000 | command); /* dir | command */
    put_32 (&cmd, switches);
    put_32 (&cmd, extra);

    memcpy (&cmd.buf[48], data, length);

    if (write (s, cmd.buf, length+48) != (length+48)) {
        printf ("write error\n");
    }

    printf ("\n***************************************************\ncommand sent, %d bytes\n", length+48);

    printf ("start sequence %08x\n", get_32 (cmd.buf,  0));
    printf ("command id     %08x\n", get_32 (cmd.buf,  4));
    printf ("length         %8x \n", get_32 (cmd.buf,  8));
    printf ("len8           %8x \n", get_32 (cmd.buf, 16));
    printf ("sequence #     %08x\n", get_32 (cmd.buf, 20));
    printf ("len8  (II)     %8x \n", get_32 (cmd.buf, 32));
    printf ("dir | comm     %08x\n", get_32 (cmd.buf, 36));
    printf ("switches       %08x\n", get_32 (cmd.buf, 40));

    printf ("ascii contents>");
    for (i=48; i<(length+48); i+=2) {
        unsigned char c = cmd.buf[i];

        if ((c>=32) && (c<=128))
        printf ("%c", c);
        else
        printf (".");
    }
    printf ("\n");

    printf ("complete hexdump of package follows:\n");
    for (i=0; i<(length+48); i++) {
        unsigned char c = cmd.buf[i];

        printf ("%02x", c);

        if ((i % 16) == 15)
        printf ("\n");

        if ((i % 2) == 1)
        printf (" ");

    }
    printf ("\n");

    }

    void string_utf16(char *dest, char *src, int len) {
    int i;

    memset (dest, 0, 1000);

    for (i=0; i<len; i++) {
        dest[i*2] = src[i];
        dest[i*2+1] = 0;
    }

    dest[i*2] = 0;
    dest[i*2+1] = 0;
    }

    void print_answer (char *data, int len) {

    int i;

    printf ("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\nanswer received, %d bytes\n", len);

    printf ("start sequence %08x\n", get_32 ((unsigned char *)data, 0));
    printf ("command id     %08x\n", get_32 ((unsigned char *)data, 4));
    printf ("length         %8x \n", get_32 ((unsigned char *)data, 8));
    printf ("len8           %8x \n", get_32 ((unsigned char *)data, 16));
    printf ("sequence #     %08x\n", get_32 ((unsigned char *)data, 20));
    printf ("len8  (II)     %8x \n", get_32 ((unsigned char *)data, 32));
    printf ("dir | comm     %08x\n", get_32 ((unsigned char *)data, 36));
    printf ("switches       %08x\n", get_32 ((unsigned char *)data, 40));

    for (i=48; i<len; i+=2) {
        unsigned char c = data[i];
        
        if ((c>=32) && (c<128))
        printf ("%c", c);
        else
        printf (" %02x ", c);
        
    }
    printf ("\n");
    }  

    void get_answer (int s) {

    char  data[BUF_SIZE];
    int   command = 0x1b;

    while (command == 0x1b) {
        int len;

        len = read (s, data, BUF_SIZE) ;
        if (!len) {
        printf ("\nalert! eof\n");
        return;
        }

        print_answer (data, len);

        command = get_32 ((unsigned char *)data, 36) & 0xFFFF;

        if (command == 0x1b) 
        send_command (s, 0x1b, 0, 0, 0, data);
    }
    }

    int get_data (int s, char *buf, size_t count) {

    ssize_t  len, total;

    total = 0;

    while (total < count) {

        len = read (s, &buf[total], count-total);

        if (len<0) {
        perror ("read error:");
        return 0;
        }

        total += len;

        if (len != 0) {
        printf ("[%d/%d]", total, count);
        fflush (stdout);
        }

    }

    return 1;

    }

    int get_header (int s, uint8_t *header) {

    unsigned char  pre_header[8];
    int            i, header_len;

    header_len = 0;

    while (1) {

        if (!get_data (s, (char *)pre_header, 8)) {
        printf ("pre-header read failed\n");
        return 0;
        }
        
        for (i=0; i<8; i++)
        printf ("pre_header[%d] = %02x (%d)\n",
            i, pre_header[i], pre_header[i]);
        
        if (pre_header[4] == 0x02) {
        
        int packet_len;
        
        packet_len = (pre_header[7] << 8 | pre_header[6]) - 8;

        printf ("asf header packet detected, len=%d\n",
            packet_len);

        if (!get_data (s, (char*) &header[header_len], packet_len)) {
        printf ("header data read failed\n");
        return 0;
        }

        header_len += packet_len;

        if ( (header[header_len-1] == 1) && (header[header_len-2]==1)) {

        write (output_fh, header, header_len);

        printf ("get header packet finished\n");

        return (header_len);

        } 

        } else {

        int packet_len, command;
        char data[BUF_SIZE];

        if (!get_data (s, (char *) &packet_len, 4)) {
        printf ("packet_len read failed\n");
        return 0;
        }
        
        packet_len = get_32 ((unsigned char *) &packet_len, 0) + 4;
        
        printf ("command packet detected, len=%d\n",
            packet_len);
        
        if (!get_data (s, data, packet_len)) {
        printf ("command data read failed\n");
        return 0;
        }
        
        command = get_32 ((unsigned char *) data, 24) & 0xFFFF;
        
        printf ("command: %02x\n", command);
        
        if (command == 0x1b) 
        send_command (s, 0x1b, 0, 0, 0, data);
        
        }

        printf ("get header packet succ\n");
    }
    }

    int interp_header (uint8_t *header, int header_len) {

    int i;
    int packet_length;

    /*
    * parse header
    */

    i = 30;
    while (i<header_len) {
        
        uint64_t  guid_1, guid_2, length;

        guid_2 = (uint64_t)header[i] | ((uint64_t)header[i+1]<<8) 
        | ((uint64_t)header[i+2]<<16) | ((uint64_t)header[i+3]<<24)
        | ((uint64_t)header[i+4]<<32) | ((uint64_t)header[i+5]<<40)
        | ((uint64_t)header[i+6]<<48) | ((uint64_t)header[i+7]<<56);
        i += 8;

        guid_1 = (uint64_t)header[i] | ((uint64_t)header[i+1]<<8) 
        | ((uint64_t)header[i+2]<<16) | ((uint64_t)header[i+3]<<24)
        | ((uint64_t)header[i+4]<<32) | ((uint64_t)header[i+5]<<40)
        | ((uint64_t)header[i+6]<<48) | ((uint64_t)header[i+7]<<56);
        i += 8;
        
        printf ("guid found: %016llx%016llx\n", guid_1, guid_2);

        length = (uint64_t)header[i] | ((uint64_t)header[i+1]<<8) 
        | ((uint64_t)header[i+2]<<16) | ((uint64_t)header[i+3]<<24)
        | ((uint64_t)header[i+4]<<32) | ((uint64_t)header[i+5]<<40)
        | ((uint64_t)header[i+6]<<48) | ((uint64_t)header[i+7]<<56);

        i += 8;

        if ( (guid_1 == 0x6cce6200aa00d9a6LL) && (guid_2 == (uint64_t) 0x11cf668e75b22630LL) ) {
        printf ("header object\n");
        } else if ((guid_1 == 0x6cce6200aa00d9a6LL) && (guid_2 == 0x11cf668e75b22636LL)) {
        printf ("data object\n");
        } else if ((guid_1 == 0x6553200cc000e48eLL) && (guid_2 == 0x11cfa9478cabdca1LL)) {

        packet_length = get_32(header, i+92-24);

        printf ("file object, packet length = %d (%d)\n",
            packet_length, get_32(header, i+96-24));


        } else if ((guid_1 == 0x6553200cc000e68eLL) && (guid_2 == 0x11cfa9b7b7dc0791LL)) {

        int stream_id = header[i+48] | header[i+49] << 8;

        printf ("stream object, stream id: %d\n", stream_id);

        stream_ids[num_stream_ids] = stream_id;
        num_stream_ids++;
        

        /*
        } else if ((guid_1 == 0x) && (guid_2 == 0x)) {
        printf ("??? object\n");
        */
        } else {
        printf ("unknown object\n");
        }

        printf ("length    : %lld\n", length);

        i += length-24;

    }

    return packet_length;

    }


    int get_media_packet (int s, int padding) {

    unsigned char  pre_header[8];
    int            i;
    char           data[BUF_SIZE];

    if (!get_data (s, (char*) pre_header, 8)) {
        printf ("pre-header read failed\n");
        return 0;
    }

    for (i=0; i<8; i++)
        printf ("pre_header[%d] = %02x (%d)\n",
            i, pre_header[i], pre_header[i]);

    if (pre_header[4] == 0x04) {

        int packet_len;

        packet_len = (pre_header[7] << 8 | pre_header[6]) - 8;

        printf ("asf media packet detected, len=%d\n",
            packet_len);

        if (!get_data (s, data, packet_len)) {
        printf ("media data read failed\n");
        return 0;
        }

        write (output_fh, data, padding);

    } else {

        int packet_len, command;

        if (!get_data (s, (char *) &packet_len, 4)) {
        printf ("packet_len read failed\n");
        return 0;
        }

        packet_len = get_32 ((unsigned char *) &packet_len, 0) + 4;

        printf ("command packet detected, len=%d\n",
            packet_len);

        if (!get_data (s, data, packet_len)) {
        printf ("command data read failed\n");
        return 0;
        }

        if ( (pre_header[7] != 0xb0) || (pre_header[6] != 0x0b)
        || (pre_header[5] != 0xfa) || (pre_header[4] != 0xce) ) {

        printf ("missing signature\n");
        return 1;

        }

        command = get_32 ((unsigned char *) data, 24) & 0xFFFF;

        printf ("command: %02x\n", command);

        if (command == 0x1b) 
        send_command (s, 0x1b, 0, 0, 0, data);
        else if (command == 0x1e) {

        printf ("everything done. Thank you for downloading a media file containing proprietary and patentend technology.\n");

        return 0;
        } else if (command != 0x05) {
        printf ("unknown command %02x\n", command);
        return 1;
        }
    }

    printf ("get media packet succ\n");

    return 1;
    }

    int download(const QString & urlString) {

    int                  s ;
    struct sockaddr_in   sa;
    struct hostent      *hp;
    char                 str[1024];
    char                 data[1024];
    uint8_t              asf_header[8192];
    int                  asf_header_len;
    int                  len, i, packet_length;
    char                 host[256];
    char                *path, *url, *file, *cp;

    /* parse url */
    url = qstrdup(urlString.toAscii());
    strncpy (host, &url[6], 255);
    cp = strchr(host,'/');
    *cp= 0;

    printf ("host : >%s<\n", host);

    path = strchr(&url[6], '/') +1;

    printf ("path : >%s<\n", path);

    file = strrchr (url, '/');

    printf ("file : >%s<\n", file);

    output_fh = open (&file[1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (output_fh<0) {
        printf ("cannot create output file '%s'.\n",
            &file[1]);
        return 1;
    }

    printf ("creating output file '%s'\n", &file[1]);

    /* DNS lookup */

    if ((hp = gethostbyname(host)) == NULL) {
        printf("Host name lookup failure.\n");
        return 1 ;
    }

    /* fill socket structure */

    bcopy ((char *) hp->h_addr, (char *) &sa.sin_addr, hp->h_length);
    sa.sin_family = hp->h_addrtype;
    /*sa.sin_port = 0x5000;*/ /* http port (80 => 50 Hex, switch Hi-/Lo-Word ) */

    sa.sin_port = htons(1755) ; /* be2me_16(1755);  mms port 1755 */

    printf ("port: %08x\n", sa.sin_port);

    /* open socket */

    if ((s = socket(hp->h_addrtype, SOCK_STREAM, 0))<0) {
        perror ("socket");
        return  1 ;
    }

    printf ("socket open\n");

    /* try to connect */

    if (connect (s, (struct sockaddr *)&sa, sizeof sa)<0) {
        perror ("request");
        return(1);
    }

    printf ("connected\n");

    /* cmd1 */
    
    sprintf (str, "\034\003NSPlayer/7.0.0.1956; {33715801-BAB3-9D85-24E9-03B90328270A}; Host: %s",
        host);
    string_utf16 (data, str, strlen(str)+2);

    send_command (s, 1, 0, 0x0004000b, strlen(str) * 2+8, data);

    len = read (s, data, BUF_SIZE) ;
    if (len)
        print_answer (data, len);
    
    /* cmd2 */

    string_utf16 (&data[8], "\002\000\\\\192.168.0.129\\TCP\\1037\0000", 
            28);
    memset (data, 0, 8);
    send_command (s, 2, 0, 0, 28*2+8, data);

    len = read (s, data, BUF_SIZE) ;
    if (len)
        print_answer (data, len);

    /* 0x5 */

    string_utf16 (&data[8], path, strlen(path));
    memset (data, 0, 8);
    send_command (s, 5, 0, 0, strlen(path)*2+12, data);

    get_answer (s);

    /* 0x15 */

    memset (data, 0, 40);
    data[32] = 2;

    send_command (s, 0x15, 1, 0, 40, data);

    num_stream_ids = 0;
    /* get_headers(s, asf_header);  */

    asf_header_len = get_header (s, asf_header);
    packet_length = interp_header (asf_header, asf_header_len);

    /* 0x33 */

    memset (data, 0, 40);

    for (i=1; i<num_stream_ids; i++) {
        data [ (i-1) * 6 + 2 ] = 0xFF;
        data [ (i-1) * 6 + 3 ] = 0xFF;
        data [ (i-1) * 6 + 4 ] = stream_ids[i];
        data [ (i-1) * 6 + 5 ] = 0x00;
    }

    send_command (s, 0x33, num_stream_ids, 
            0xFFFF | stream_ids[0] << 16, 
            (num_stream_ids-1)*6+2 , data);

    get_answer (s);

    /* 0x07 */

    memset (data, 0, 40);

    for (i=8; i<16; i++)
        data[i] = 0xFF;

    data[20] = 0x04;

    send_command (s, 0x07, 1, 
            0xFFFF | stream_ids[0] << 16, 
            24, data);



    while (get_media_packet(s, packet_length)) {

        /* nothing */

    }

    close (output_fh);
    close (s);

    return 0;
    }
    
private:
    int seq_num;
    int num_stream_ids;
    int stream_ids[20];
    int output_fh;
};

class MMSClientThread : public QThread
{
public:
    MMSClientThread(const QString & url)
        : QThread(),
          m_url(url)
    {
        
    }
    
    void run()
    {
        MMSClient client;
       
        client.download(m_url);
    }
    
private:
    QString m_url;
};
