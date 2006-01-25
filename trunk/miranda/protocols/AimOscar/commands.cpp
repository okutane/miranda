#include "commands.h"
int aim_send_connection_packet(char *buf)
{
	if(aim_sendflap(0x01,4,buf)==0)
		return 0;
	else
		return -1;
}
int aim_authkey_request()
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x17,0x06,3,buf);
	aim_writetlv(0x01,strlen(conn.username),conn.username,buf);
	aim_writetlv(0x4B,0,0,buf);
	aim_writetlv(0x5A,0,0,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_auth_request(char* key,char* language,char* country)
{
	char buf[MSG_LEN*2];
	md5_byte_t pass_hash[16];
	md5_byte_t auth_hash[16];
	md5_state_t state;
	md5_init(&state);
	md5_append(&state,(const md5_byte_t *)conn.password, strlen(conn.password));
	md5_finish(&state,pass_hash);
	md5_init(&state);
	md5_append(&state,(md5_byte_t*)key, strlen(key));
	md5_append(&state,(md5_byte_t*)pass_hash,MD5_HASH_LENGTH);
	md5_append(&state,(md5_byte_t*)AIM_MD5_STRING, strlen(AIM_MD5_STRING));
	md5_finish(&state,auth_hash);
	aim_writesnac(0x17,0x02,4,buf);
	aim_writetlv(0x01,strlen(conn.username),conn.username,buf);
	aim_writetlv(0x25,MD5_HASH_LENGTH,(char*)auth_hash,buf);
	aim_writetlv(0x4C,0,0,buf);//signifies new password hash instead of old method
	aim_writetlv(0x03,strlen(AIM_CLIENT_ID_STRING),AIM_CLIENT_ID_STRING,buf);
	aim_writetlv(0x16,sizeof(AIM_CLIENT_ID_NUMBER)-1,AIM_CLIENT_ID_NUMBER,buf);
	aim_writetlv(0x17,sizeof(AIM_CLIENT_MAJOR_VERSION)-1,AIM_CLIENT_MAJOR_VERSION,buf);
	aim_writetlv(0x18,sizeof(AIM_CLIENT_MINOR_VERSION)-1,AIM_CLIENT_MINOR_VERSION,buf);
	aim_writetlv(0x19,sizeof(AIM_CLIENT_LESSER_VERSION)-1,AIM_CLIENT_LESSER_VERSION,buf);
	aim_writetlv(0x1A,sizeof(AIM_CLIENT_BUILD_NUMBER)-1,AIM_CLIENT_BUILD_NUMBER,buf);
	aim_writetlv(0x14,sizeof(AIM_CLIENT_DISTRIBUTION_NUMBER)-1,AIM_CLIENT_DISTRIBUTION_NUMBER,buf);
	aim_writetlv(0x0F,strlen(language),language,buf);
	aim_writetlv(0x0E,strlen(country),country,buf);
	aim_writetlv(0x4A,1,"\x01",buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_send_cookie(int cookie_size,char * cookie)
{
	char buf[MSG_LEN*2]={0x00,0x00,0x00,0x01};//Protocol version number
	conn.packet_offset=4;
	aim_writetlv(0x06,cookie_size,cookie,buf);
	if(aim_sendflap(0x01,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_send_service_request()
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x01,0x17,6,buf);
	aim_writefamily(AIM_SERVICE_GENERIC,buf);
	aim_writefamily(AIM_SERVICE_SSI,buf);
	aim_writefamily(AIM_SERVICE_LOCATION,buf);
	aim_writefamily(AIM_SERVICE_BUDDYLIST,buf);
	aim_writefamily(AIM_SERVICE_MESSAGING,buf);
	aim_writefamily(AIM_SERVICE_INVITATION,buf);
	aim_writefamily(AIM_SERVICE_GENERIC,buf);
	aim_writefamily(AIM_SERVICE_POPUP,buf);
	aim_writefamily(AIM_SERVICE_BOS,buf);
	aim_writefamily(AIM_SERVICE_USERLOOKUP,buf);
	aim_writefamily(AIM_SERVICE_STATS,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_request_rates()
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x01,0x06,6,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_accept_rates()
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x01,0x08,6,buf);
	aim_writegeneric(10,AIM_SERVICE_RATES,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_request_icbm()
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x04,0x04,6,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_set_icbm()
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x04,0x02,6,buf);
	aim_writegeneric(2,"\0\0",buf);//channel
	aim_writegeneric(4,"\0\0\0\x0b",buf);//flags
	aim_writegeneric(2,"\x1f\x40",buf);//max snac size
	aim_writegeneric(2,"\x03\xe7",buf);//max sender warning level
	aim_writegeneric(2,"\x03\xe7",buf);//max receiver warning level
	aim_writegeneric(2,"\0\0",buf);//min msg interval
	aim_writegeneric(2,"\0\0",buf);//unknown
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_request_list()
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x13,0x04,6,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_activate_list()
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x13,0x07,6,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_set_caps()
{
	char buf[MSG_LEN*2];
	char temp[MSG_LEN*2];
	memcpy(temp,AIM_CAP_ICQ_SUPPORT,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH],AIM_CAP_CHAT,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*2],AIM_CAP_RECEIVE_FILES,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*3],AIM_CAP_SEND_FILES,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*4],AIM_CAP_ICHAT,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*5],AIM_CAP_UNKNOWN,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*6],AIM_CAP_UNKNOWN3,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*7],AIM_CAP_LIST_TRANSFER,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*8],AIM_CAP_DIRECT_PLAY,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*9],AIM_CAP_UTF8,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*10],AIM_CAP_GAMES,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*11],AIM_CAP_STOCKS,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*12],AIM_CAP_AVATARS,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*13],AIM_CAP_DIRECT_IM,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*14],AIM_CAP_DIRECT_PLAY,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*15],AIM_CAP_VOICE_CHAT,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*16],AIM_CAP_SEND_FILES,AIM_CAPS_LENGTH);
	memcpy(&temp[AIM_CAPS_LENGTH*17],AIM_CAP_RECEIVE_FILES,AIM_CAPS_LENGTH);
	//AIM_CAP_CHAT
	aim_writesnac(0x02,0x04,6,buf);
	aim_writetlv(0x05,AIM_CAPS_LENGTH*17,temp,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_set_away(char *msg)//user info
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x02,0x04,6,buf);
	aim_writetlv(0x03,strlen(AIM_MSG_TYPE),AIM_MSG_TYPE,buf);
	if(msg!=NULL)
		aim_writetlv(0x04,strlen(msg),msg,buf);
	else
		aim_writetlv(0x04,0,0,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_set_invis(char* status,char* status_flag)
{
	char buf[MSG_LEN*2];
	char temp[MSG_LEN*2];
	memcpy(temp,status_flag,sizeof(status_flag)-2);
	memcpy(&temp[sizeof(status_flag)-2],status,sizeof(status)-2);
	aim_writesnac(0x01,0x1E,6,buf);
	aim_writetlv(0x06,sizeof(status)+sizeof(status_flag)-4,temp,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_client_ready()
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x01,0x02,6,buf);
	aim_writefamily(AIM_SERVICE_GENERIC,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,buf);
	aim_writefamily(AIM_SERVICE_SSI,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,buf);
	aim_writefamily(AIM_SERVICE_LOCATION,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,buf);
	aim_writefamily(AIM_SERVICE_BUDDYLIST,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,buf);
	aim_writefamily(AIM_SERVICE_MESSAGING,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,buf);
	aim_writefamily(AIM_SERVICE_INVITATION,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,buf);
	aim_writefamily(AIM_SERVICE_GENERIC,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,buf);
	aim_writefamily(AIM_SERVICE_POPUP,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,buf);
	aim_writefamily(AIM_SERVICE_BOS,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,buf);
	aim_writefamily(AIM_SERVICE_USERLOOKUP,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,buf);
	aim_writefamily(AIM_SERVICE_STATS,buf);
	aim_writegeneric(4,AIM_TOOL_VERSION,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_send_plaintext_message(char* sn,char* msg)
{	
	//see http://iserverd.khstu.ru/oscar/snac_04_06_ch1.html
	char caps_frag[]={0x05,0x01,0x00,0x01,0x01};
	char msg_frag[MSG_LEN*2];
	memcpy(msg_frag,"\x01\x01\0\0\0\0\0\0",8);//last two bytes are charset if 0xFFFF then triton doesn't accept message
	char tlv_frag[MSG_LEN];
	unsigned short sn_length=strlen(sn);
	unsigned short msg_length=htons(strlen(msg)+4);
	char buf[MSG_LEN*2];
	aim_writesnac(0x04,0x06,6,buf);
	aim_writegeneric(8,"\0\0\0\0\0\0\0\0",buf);
	aim_writegeneric(2,"\0\x01",buf);//channel
	aim_writegeneric(1,(char*)&sn_length,buf);
	aim_writegeneric(sn_length,sn,buf);
	memcpy(&msg_frag[2],(char*)&msg_length,2);
	memcpy(&msg_frag[8],msg,strlen(msg));
	memcpy(tlv_frag,caps_frag,sizeof(caps_frag));
	memcpy(&tlv_frag[sizeof(caps_frag)],msg_frag,strlen(msg)+8);
	aim_writetlv(0x02,sizeof(caps_frag)+strlen(msg)+8,tlv_frag,buf);
	aim_writetlv(0x03,0,0,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_query_away_message(char* sn)
{
	char buf[MSG_LEN*2];
	unsigned short sn_length=strlen(sn);
	aim_writesnac(0x02,0x15,0x06,buf);
	aim_writegeneric(4,"\0\0\0\x02",buf);
	aim_writegeneric(1,(char*)&sn_length,buf);
	aim_writegeneric(sn_length,sn,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_query_profile(char* sn)
{
	char buf[MSG_LEN*2];
	unsigned short sn_length=strlen(sn);
	aim_writesnac(0x02,0x15,0x06,buf);
	aim_writegeneric(4,"\0\0\0\x01",buf);
	aim_writegeneric(1,(char*)&sn_length,buf);
	aim_writegeneric(sn_length,sn,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
//function below not used
int aim_edit_contacts_start()
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x13,0x11,6,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
//function below not used
int aim_edit_contacts_end()
{
	char buf[MSG_LEN*2];
	aim_writesnac(0x13,0x12,6,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 0;
	else
		return -1;
}
int aim_delete_contact(char* sn,unsigned short item_id,unsigned short group_id)
{
	char buf[MSG_LEN*2];
	unsigned short sn_length_flipped=htons(strlen(sn));
	unsigned short sn_length=strlen(sn);
	item_id=htons(item_id);
	group_id=htons(group_id);
	aim_writesnac(0x13,0x0a,0x06,buf);
	aim_writegeneric(2,(char*)&sn_length_flipped,buf);
	aim_writegeneric(sn_length,sn,buf);
	aim_writegeneric(2,(char*)&group_id,buf);
	aim_writegeneric(2,(char*)&item_id,buf);
	aim_writegeneric(4,"\0\0\0\0",buf);//item type then the last two bytes are for additional data length
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_add_contact(char* sn,unsigned short item_id,unsigned short group_id)
{
	char buf[MSG_LEN*2];
	unsigned short sn_length_flipped=htons(strlen(sn));
	unsigned short sn_length=strlen(sn);
	item_id=htons(item_id);
	group_id=htons(group_id);
	aim_writesnac(0x13,0x08,0x06,buf);
	aim_writegeneric(2,(char*)&sn_length_flipped,buf);
	aim_writegeneric(sn_length,sn,buf);
	aim_writegeneric(2,(char*)&group_id,buf);
	aim_writegeneric(2,(char*)&item_id,buf);
	aim_writegeneric(4,"\0\0\0\0",buf);//item type then the last two bytes are for additional data length
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_add_group(char* name,unsigned short group_id)
{
	char buf[MSG_LEN*2];
	unsigned short name_length_flipped=htons(strlen(name));
	unsigned short name_length=strlen(name);
	group_id=htons(group_id);
	aim_writesnac(0x13,0x08,0x06,buf);
	aim_writegeneric(2,(char*)&name_length_flipped,buf);
	aim_writegeneric(name_length,name,buf);
	aim_writegeneric(2,(char*)&group_id,buf);
	aim_writegeneric(6,"\0\0\0\x01\0\0",buf);//item id[2] item type[2] addition data[2]
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_delete_group(char* name,unsigned short group_id)
{
	char buf[MSG_LEN*2];
	unsigned short name_length_flipped=htons(strlen(name));
	unsigned short name_length=strlen(name);
	group_id=htons(group_id);
	aim_writesnac(0x13,0x0a,0x06,buf);
	aim_writegeneric(2,(char*)&name_length_flipped,buf);
	aim_writegeneric(name_length,name,buf);
	aim_writegeneric(2,(char*)&group_id,buf);
	aim_writegeneric(6,"\0\0\0\x01\0\0",buf);//item id[2] item type[2] addition data[2]
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_mod_group(char* name,unsigned short group_id,char* members,unsigned short members_length)
{
	char buf[MSG_LEN*2];
	unsigned short name_length_flipped=htons(strlen(name));
	unsigned short name_length=strlen(name);
	group_id=htons(group_id);
	aim_writesnac(0x13,0x09,0x06,buf);
	aim_writegeneric(2,(char*)&name_length_flipped,buf);
	aim_writegeneric(name_length,name,buf);
	aim_writegeneric(2,(char*)&group_id,buf);
	aim_writegeneric(4,"\0\0\0\x01",buf);//item id[2] item type[2
	unsigned short tlv_length=htons(TLV_HEADER_SIZE+members_length);
	aim_writegeneric(2,(char*)&tlv_length,buf);
	aim_writetlv(0xc8,members_length,members,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
//below function Not used
int aim_add_clientside_contact(char* name)//adds a contact to the client side list so that aim will send status messages
{
	char buf[MSG_LEN*2];
	unsigned short name_length=strlen(name);
	aim_writesnac(0x03,0x04,0x06,buf);
	aim_writegeneric(1,(char*)&name_length,buf);//one byte this time no need to htons
	aim_writegeneric(name_length,name,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
//below function not used
int aim_remove_clientside_contact(char* name)//removes a contact to the client side list so that aim will send status messages
{
	char buf[MSG_LEN*2];
	unsigned short name_length=strlen(name);
	aim_writesnac(0x03,0x05,0x06,buf);
	aim_writegeneric(1,(char*)&name_length,buf);//one byte this time no need to htons
	aim_writegeneric(name_length,name,buf);
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_keepalive()
{
	if(aim_sendflap(0x05,0,0)==0)
		return 0;
	else
		return -1;
}
int aim_send_file(char* sn,char* icbm_cookie, char* file_name,unsigned long total_bytes,char* descr)//used when requesting a regular file transfer
{	
	//see http://iserverd.khstu.ru/oscar/snac_04_06_ch2.html
	char msg_frag[MSG_LEN*2];
	unsigned short sn_length=strlen(sn);
	char buf[MSG_LEN*2];
	aim_writesnac(0x04,0x06,6,buf);
	aim_writegeneric(8,icbm_cookie,buf);
	aim_writegeneric(2,"\0\x02",buf);//channel
	aim_writegeneric(1,(char*)&sn_length,buf);
	aim_writegeneric(sn_length,sn,buf);
	{
	//0x05 tlv data begin
	memcpy(&msg_frag[0],"\0\0",2);
	memcpy(&msg_frag[2],icbm_cookie,8);
	memcpy(&msg_frag[10],AIM_CAP_SEND_FILES,sizeof(AIM_CAP_SEND_FILES));
	char temp[]="\0\x0a\0\x02\0\x01\0\x0f\0\0\0\x0e\0\x02\x65\x6e\0\x0d\0\x08us-ascii\0\x0c";
	memcpy(&msg_frag[9+sizeof(AIM_CAP_SEND_FILES)],temp,sizeof(temp));	
	unsigned short descr_size =htons(strlen(descr));
	memcpy(&msg_frag[8+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)],(char*)&descr_size,2);
	descr_size =htons(descr_size);
	memcpy(&msg_frag[10+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)],descr,descr_size);
	memcpy(&msg_frag[10+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x02\0\x04",4);
	unsigned long lip=htonl(conn.InternalIP);
	char *ip=(char*)&lip;
	memcpy(&msg_frag[14+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],ip,4);
	memcpy(&msg_frag[18+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x16\0\x04",4);
	unsigned long bw_lip=~lip;
	char* bw_ip=(char*)&bw_lip;
	memcpy(&msg_frag[22+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],bw_ip,4);

	memcpy(&msg_frag[26+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x03\0\x04",4);
	memcpy(&msg_frag[30+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],ip,4);


	unsigned short lport=htons(conn.LocalPort);
	memcpy(&msg_frag[34+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x05\0\x02",4);
	char *port=(char*)&lport;
	memcpy(&msg_frag[38+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],port,2);
	unsigned short bw_lport=~lport;
	memcpy(&msg_frag[40+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x17\0\x02",4);
	char *bw_port=(char*)&bw_lport;
	memcpy(&msg_frag[44+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],bw_port,2);
	/*
	from: On Sending Files via OSCAR
	for tlv 0x2711
	* The first 2 bytes are the Multiple Files Flag. A value of 0x0001 indicates
	that only one file is being transferred while a value of 0x0002 indicates that more
	than one file is being transferred.
	* The next 2 bytes is the File Count, the total number of files that will be
	transmitted during this file transfer.
	* The next 4 bytes is the Total Bytes, the sum of the size in bytes of all files
	to be transferred.
	* The remaining bytes is a null-terminated string that is the name of the
	file.
	*/
	memcpy(&msg_frag[46+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\x27\x11",2);
	unsigned short packet_size=htons(9+strlen(file_name));
	memcpy(&msg_frag[48+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],(char*)&packet_size,2);
	memcpy(&msg_frag[50+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x01\0\x01",4);
	total_bytes=htonl(total_bytes);
	memcpy(&msg_frag[54+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],(char*)&total_bytes,4);
	memcpy(&msg_frag[58+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],file_name,strlen(file_name));
	memcpy(&msg_frag[58+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size+strlen(file_name)],"\0",1);
	memcpy(&msg_frag[59+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size+strlen(file_name)],"\x27\x12\0\x08us-ascii",12);
	aim_writetlv(0x05,(71+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size+strlen(file_name)),msg_frag,buf);
	//end huge ass tlv hell
	}
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_send_file_proxy(char* sn,char* icbm_cookie, char* file_name,unsigned long total_bytes,char* descr,unsigned long proxy_ip, unsigned short port)//used when requesting a file transfer through a proxy-or rather forcing proxy use
{	
	//see http://iserverd.khstu.ru/oscar/snac_04_06_ch2.html
	char msg_frag[MSG_LEN*2];
	unsigned short sn_length=strlen(sn);
	char buf[MSG_LEN*2];
	aim_writesnac(0x04,0x06,6,buf);
	aim_writegeneric(8,icbm_cookie,buf);
	aim_writegeneric(2,"\0\x02",buf);//channel
	aim_writegeneric(1,(char*)&sn_length,buf);
	aim_writegeneric(sn_length,sn,buf);
	{
	//0x05 tlv data begin
	memcpy(&msg_frag[0],"\0\0",2);
	memcpy(&msg_frag[2],icbm_cookie,8);
	memcpy(&msg_frag[10],AIM_CAP_SEND_FILES,sizeof(AIM_CAP_SEND_FILES));
	char temp[]="\0\x0a\0\x02\0\x01\0\x0f\0\0\0\x0e\0\x02\x65\x6e\0\x0d\0\x08us-ascii\0\x0c";
	memcpy(&msg_frag[9+sizeof(AIM_CAP_SEND_FILES)],temp,sizeof(temp));	
	unsigned short descr_size =htons(strlen(descr));
	memcpy(&msg_frag[8+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)],(char*)&descr_size,2);
	descr_size =htons(descr_size);
	memcpy(&msg_frag[10+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)],descr,descr_size);
	memcpy(&msg_frag[10+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x02\0\x04",4);

	unsigned long lip=htonl(proxy_ip);
	char *ip=(char*)&lip;
	memcpy(&msg_frag[14+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],ip,4);
	memcpy(&msg_frag[18+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x16\0\x04",4);
	unsigned long bw_lip=~lip;
	char* bw_ip=(char*)&bw_lip;
	memcpy(&msg_frag[22+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],bw_ip,4);

	memcpy(&msg_frag[26+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x03\0\x04",4);
	memcpy(&msg_frag[30+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],ip,4);


	unsigned short lport=htons(port);
	memcpy(&msg_frag[34+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x05\0\x02",4);
	char *port=(char*)&lport;
	memcpy(&msg_frag[38+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],port,2);
	unsigned short bw_lport=~lport;
	memcpy(&msg_frag[40+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x17\0\x02",4);
	char *bw_port=(char*)&bw_lport;
	memcpy(&msg_frag[44+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],bw_port,2);

	memcpy(&msg_frag[46+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x10\0\0\x27\x11",6);
	unsigned short packet_size=htons(9+strlen(file_name));
	memcpy(&msg_frag[52+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],(char*)&packet_size,2);
	memcpy(&msg_frag[54+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],"\0\x01\0\x01",4);
	total_bytes=htonl(total_bytes);
	memcpy(&msg_frag[58+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],(char*)&total_bytes,4);
	memcpy(&msg_frag[62+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size],file_name,strlen(file_name));
	memcpy(&msg_frag[62+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size+strlen(file_name)],"\0",1);
	memcpy(&msg_frag[63+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size+strlen(file_name)],"\x27\x12\0\x08us-ascii",12);
	aim_writetlv(0x05,(75+sizeof(AIM_CAP_SEND_FILES)+sizeof(temp)+descr_size+strlen(file_name)),msg_frag,buf);
	//end huge ass tlv hell
	}
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_file_redirected_request(char* sn,char* icbm_cookie)//used when a direct connection failed so we request a redirected connection
{	
	char msg_frag[MSG_LEN*2];
	unsigned short sn_length=strlen(sn);
	char buf[MSG_LEN*2];
	aim_writesnac(0x04,0x06,6,buf);
	aim_writegeneric(8,icbm_cookie,buf);
	aim_writegeneric(2,"\0\x02",buf);//channel
	aim_writegeneric(1,(char*)&sn_length,buf);
	aim_writegeneric(sn_length,sn,buf);
	{
	//0x05 tlv data begin
	memcpy(&msg_frag[0],"\0\0",2);
	memcpy(&msg_frag[2],icbm_cookie,8);
	memcpy(&msg_frag[10],AIM_CAP_SEND_FILES,sizeof(AIM_CAP_SEND_FILES));
	char temp[]="\0\x0a\0\x02\0\x02\0\x02\0\x04";
	memcpy(&msg_frag[9+sizeof(AIM_CAP_SEND_FILES)],temp,sizeof(temp));
	unsigned long proxy_ip=htonl(conn.InternalIP);
	char *ip=(char*)&proxy_ip;
	memcpy(&msg_frag[19+sizeof(AIM_CAP_SEND_FILES)],ip,4);
	memcpy(&msg_frag[23+sizeof(AIM_CAP_SEND_FILES)],"\0\x16\0\x04",4);
	unsigned long bw_lip=~proxy_ip;
	char* bw_ip=(char*)&bw_lip;
	memcpy(&msg_frag[27+sizeof(AIM_CAP_SEND_FILES)],bw_ip,4);
	memcpy(&msg_frag[31+sizeof(AIM_CAP_SEND_FILES)],"\0\x03\0\x04",4);
	memcpy(&msg_frag[35+sizeof(AIM_CAP_SEND_FILES)],ip,4);
	unsigned short lport=htons(conn.LocalPort);
	memcpy(&msg_frag[39+sizeof(AIM_CAP_SEND_FILES)],"\0\x05\0\x02",4);
	char *port=(char*)&lport;
	memcpy(&msg_frag[43+sizeof(AIM_CAP_SEND_FILES)],port,2);
	unsigned short bw_lport=~lport;
	memcpy(&msg_frag[45+sizeof(AIM_CAP_SEND_FILES)],"\0\x17\0\x02",4);
	char *bw_port=(char*)&bw_lport;
	memcpy(&msg_frag[49+sizeof(AIM_CAP_SEND_FILES)],bw_port,2);
	aim_writetlv(0x05,(51+sizeof(AIM_CAP_SEND_FILES)),msg_frag,buf);
	}
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_file_proxy_request(char* sn,char* icbm_cookie,char request_num,unsigned long proxy_ip, unsigned short port)//used when a direct & redirected connection failed so we request a proxy connection
{	
	char msg_frag[MSG_LEN*2];
	unsigned short sn_length=strlen(sn);
	char buf[MSG_LEN*2];
	aim_writesnac(0x04,0x06,6,buf);
	aim_writegeneric(8,icbm_cookie,buf);
	aim_writegeneric(2,"\0\x02",buf);//channel
	aim_writegeneric(1,(char*)&sn_length,buf);
	aim_writegeneric(sn_length,sn,buf);
	{
	//0x05 tlv data begin
	memcpy(&msg_frag[0],"\0\0",2);
	memcpy(&msg_frag[2],icbm_cookie,8);
	memcpy(&msg_frag[10],AIM_CAP_SEND_FILES,sizeof(AIM_CAP_SEND_FILES));
	char temp[10];
	memcpy(temp,"\0\x0a\0\x02\0",5);
	memcpy(&temp[5],&request_num,1);
	memcpy(&temp[6],"\0\x02\0\x04",4);
	memcpy(&msg_frag[9+sizeof(AIM_CAP_SEND_FILES)],temp,sizeof(temp));
	proxy_ip=htonl(proxy_ip);
	char *ip=(char*)&proxy_ip;
	memcpy(&msg_frag[19+sizeof(AIM_CAP_SEND_FILES)],ip,4);
	memcpy(&msg_frag[23+sizeof(AIM_CAP_SEND_FILES)],"\0\x16\0\x04",4);
	unsigned long bw_lip=~proxy_ip;
	char* bw_ip=(char*)&bw_lip;
	memcpy(&msg_frag[27+sizeof(AIM_CAP_SEND_FILES)],bw_ip,4);
	unsigned short lport=htons(port);
	memcpy(&msg_frag[31+sizeof(AIM_CAP_SEND_FILES)],"\0\x05\0\x02",4);
	char *port=(char*)&lport;
	memcpy(&msg_frag[35+sizeof(AIM_CAP_SEND_FILES)],port,2);
	unsigned short bw_lport=~lport;
	memcpy(&msg_frag[37+sizeof(AIM_CAP_SEND_FILES)],"\0\x17\0\x02",4);
	char *bw_port=(char*)&bw_lport;
	memcpy(&msg_frag[41+sizeof(AIM_CAP_SEND_FILES)],bw_port,2);
	memcpy(&msg_frag[43+sizeof(AIM_CAP_SEND_FILES)],"\0\x10\0\0",4);
	aim_writetlv(0x05,(47+sizeof(AIM_CAP_SEND_FILES)),msg_frag,buf);
	}
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_accept_file(char* sn,char* icbm_cookie)
{	
	//see http://iserverd.khstu.ru/oscar/snac_04_06_ch2.html
	char msg_frag[MSG_LEN*2];
	unsigned short sn_length=strlen(sn);
	char buf[MSG_LEN*2];
	aim_writesnac(0x04,0x06,6,buf);
	aim_writegeneric(8,icbm_cookie,buf);
	aim_writegeneric(2,"\0\x02",buf);//channel
	aim_writegeneric(1,(char*)&sn_length,buf);
	aim_writegeneric(sn_length,sn,buf);
	{
	//0x05 tlv data begin
	memcpy(&msg_frag[0],"\0\x02",2);//accept
	memcpy(&msg_frag[2],icbm_cookie,8);
	memcpy(&msg_frag[10],AIM_CAP_SEND_FILES,sizeof(AIM_CAP_SEND_FILES));
	aim_writetlv(0x05,(9+sizeof(AIM_CAP_SEND_FILES)),msg_frag,buf);
	//end tlv
	}
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
int aim_deny_file(char* sn,char* icbm_cookie)
{	
	//see http://iserverd.khstu.ru/oscar/snac_04_06_ch2.html
	char msg_frag[MSG_LEN*2];
	unsigned short sn_length=strlen(sn);
	char buf[MSG_LEN*2];
	aim_writesnac(0x04,0x06,6,buf);
	aim_writegeneric(8,icbm_cookie,buf);
	aim_writegeneric(2,"\0\x02",buf);//channel
	aim_writegeneric(1,(char*)&sn_length,buf);
	aim_writegeneric(sn_length,sn,buf);
	{
	//0x05 tlv data begin
	memcpy(&msg_frag[0],"\0\x01",2);//deny or cancel
	memcpy(&msg_frag[2],icbm_cookie,8);
	memcpy(&msg_frag[10],AIM_CAP_SEND_FILES,sizeof(AIM_CAP_SEND_FILES));
	aim_writetlv(0x05,(9+sizeof(AIM_CAP_SEND_FILES)),msg_frag,buf);
	//end tlv
	}
	if(aim_sendflap(0x02,conn.packet_offset,buf)==0)
		return 1;
	else
		return 0;
}
