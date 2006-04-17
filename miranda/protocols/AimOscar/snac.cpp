#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#include "snac.h"
void snac_md5_authkey(unsigned short subgroup,char* buf)//family 0x0017
{
	if(subgroup==0x0007)//md5 authkey string
	{
		unsigned short* length=(unsigned short*)&buf[SNAC_SIZE];
		*length=htons(*length);
		char* authkey=new char[*length+1];
		memcpy(authkey,&buf[SNAC_SIZE+sizeof(*length)],*length);
		authkey[*length]='\0';
		aim_auth_request(authkey,AIM_LANGUAGE,AIM_COUNTRY);
		delete[] authkey;
	}
}
char *COOKIE=NULL;
int COOKIE_LENGTH=0;
int snac_authorization_reply(unsigned short subgroup,char* buf,unsigned short flap_length)//family 0x0017
{
	if(subgroup==0x0003)
	{
		char* server;
		int length=(flap_length)-(SNAC_SIZE);
		int address=0;
		buf=&buf[SNAC_SIZE];
		while(address<length)
		{
			TLV tlv(&buf[address]);
			if(tlv.cmp(0x0005))
				server=tlv.dup();
			else if(tlv.cmp(0x0006))
			{
				Netlib_CloseHandle(conn.hServerConn);
				conn.hServerConn=aim_connect(server);
				delete[] server;
				if(conn.hServerConn)
				{
					delete[] COOKIE;
					COOKIE_LENGTH=tlv.len();
					COOKIE=tlv.dup();
					ForkThread((pThreadFunc)aim_protocol_negotiation,NULL);
					return 1;
				}
			}
			else if(tlv.cmp(0x0008))
			{
				unsigned short* error=new unsigned short; 
				*error=tlv.ushort();
				ForkThread((pThreadFunc)login_error,error);
				break;
			}
			address+=(tlv.len()+TLV_HEADER_SIZE);
		}
	}
	return 0;
}
void snac_supported_families(unsigned short subgroup)//family 0x0001
{
	if(subgroup==0x0003)//server supported service list
	{
		aim_send_service_request();
	}
}
void snac_supported_family_versions(unsigned short subgroup)//family 0x0001
{
	if(subgroup==0x0018)//service list okayed
	{
		aim_request_rates();//request some rate crap
		aim_request_list();
	}
}
void snac_rate_limitations(unsigned short subgroup)// family 0x0013
{
	if(subgroup==0x0007)
	{
		aim_accept_rates();
		aim_request_icbm();
	}
}
void snac_icbm_limitations(unsigned short subgroup)//family 0x0004
{
	for(int t=0;!conn.buddy_list_received;t++)
	{
		Sleep(1000);
		if(t==5)
			break;
	}
	if(subgroup==0x0005)
	{
		aim_set_icbm();
		aim_set_caps();
		broadcast_status(conn.initial_status);
		if(conn.initial_status==ID_STATUS_ONLINE)
		{
			aim_set_invis(AIM_STATUS_ONLINE,AIM_STATUS_NULL);
			aim_set_away(NULL);
		}
		else if(conn.initial_status==ID_STATUS_INVISIBLE)
		{
			aim_set_invis(AIM_STATUS_INVISIBLE,AIM_STATUS_NULL);
		}
		else if(conn.initial_status==ID_STATUS_AWAY)
		{
			if(!conn.szModeMsg)
			{
				DBVARIANT dbv;
				if(!DBGetContactSetting(NULL,MOD_KEY_SA,OTH_KEY_AD,&dbv)&&!DBGetContactSettingByte(NULL,MOD_KEY_SA,OTH_KEY_AI,0))
				{
					assign_modmsg(dbv.pszVal);
					DBFreeVariant(&dbv);
				}
				else if(!DBGetContactSetting(NULL,MOD_KEY_SA,OTH_KEY_AM,&dbv)&&!DBGetContactSettingByte(NULL,MOD_KEY_SA,OTH_KEY_AI,0))
				{
					assign_modmsg(dbv.pszVal);
					DBFreeVariant(&dbv);
				}
				else
					assign_modmsg(DEFAULT_AWAY_MSG);
			}
				aim_set_invis(AIM_STATUS_AWAY,AIM_STATUS_NULL);
				aim_set_away(conn.szModeMsg);

		}
		if(DBGetContactSettingByte(NULL, AIM_PROTOCOL_NAME, AIM_KEY_II,0))
		{
			unsigned long time = DBGetContactSettingDword(NULL, AIM_PROTOCOL_NAME, AIM_KEY_IIT, 0);
			aim_set_idle(time*60);
		}
		aim_client_ready();
		add_contacts_to_groups();//woo
		aim_activate_list();
		conn.state=1;
	}
}
void snac_user_online(unsigned short subgroup, char* buf)//family 0x0003
{
	if(subgroup==0x000b)
	{
		bool hiptop_user=0;
		bool wireless_user=0;
		bool bot_user=0;
		bool adv2_icon=0;
		bool adv1_icon=0;
		bool away_user=0;
		bool caps_included=0;
		unsigned char buddy_length=buf[SNAC_SIZE];
		int offset=SNAC_SIZE+TLV_PART_SIZE+buddy_length+1;
		int i=0;
		char buddy[32];
		ZeroMemory(buddy,sizeof(buddy));
		memcpy(buddy,&buf[SNAC_SIZE+1],buddy_length);
		unsigned short* tlv_count=(unsigned short*)&buf[offset];//tlv count
		*tlv_count=htons(*tlv_count);//flip
		offset+=TLV_PART_SIZE;
		HANDLE hContact=find_contact(buddy);
		int ESIconsDisabled=DBGetContactSettingByte(NULL, AIM_PROTOCOL_NAME, AIM_KEY_ES, 0);
		if(!hContact)
			hContact=add_contact(buddy);
		for(;i<*tlv_count;i++)
		{
			TLV tlv(&buf[offset]);
			offset+=TLV_HEADER_SIZE;
			if(tlv.cmp(0x0001))//user status
			{
				if(hContact)
				{
					unsigned short status = tlv.ushort();
					int unconfirmed = status&0x0001;
					int admin_aol = status&0x0002;
					int aol = status&0x0004;
					int nonfree = status&0x0008;
					int free = status&0x0010;
					int away = status&0x0020;
					int icq = status&0x0040;
					int wireless = status&0x0080;
					int bot = status&0x0400;
 					DBWriteContactSettingString(hContact, AIM_PROTOCOL_NAME, AIM_KEY_NK, buddy);
					if(ServiceExists(MS_CLIST_EXTRA_ADD_ICON))
					{
						adv2_icon=1;
						char* data=new char[sizeof(HANDLE)*2+sizeof(unsigned short)];
						if(admin_aol)
						{
							DBWriteContactSettingByte(hContact, AIM_PROTOCOL_NAME, AIM_KEY_AC, ACCOUNT_TYPE_ADMIN);
							memcpy(data,&conn.admin_icon,sizeof(HANDLE));
						}
						else if(aol)
						{
							DBWriteContactSettingByte(hContact, AIM_PROTOCOL_NAME, AIM_KEY_AC, ACCOUNT_TYPE_AOL);	
							memcpy(data,&conn.aol_icon,sizeof(HANDLE));						
						}
						else if(icq)
						{
							DBWriteContactSettingByte(hContact, AIM_PROTOCOL_NAME, AIM_KEY_AC, ACCOUNT_TYPE_ICQ);	
							memcpy(data,&conn.icq_icon,sizeof(HANDLE));
						}
						else if(unconfirmed)
						{
							DBWriteContactSettingByte(hContact, AIM_PROTOCOL_NAME, AIM_KEY_AC, ACCOUNT_TYPE_UNCONFIRMED);
							memcpy(data,&conn.unconfirmed_icon,sizeof(HANDLE));
						}
						else
						{
							DBWriteContactSettingByte(hContact, AIM_PROTOCOL_NAME, AIM_KEY_AC, ACCOUNT_TYPE_CONFIRMED);
							memcpy(data,&conn.confirmed_icon,sizeof(HANDLE));
						}
						memcpy(&data[sizeof(HANDLE)],&hContact,sizeof(HANDLE));
						unsigned short column_type=EXTRA_ICON_ADV2;
						memcpy(&data[sizeof(HANDLE)*2],(char*)&column_type,sizeof(unsigned short));
						ForkThread((pThreadFunc)set_extra_icon,data);
					}
					if(bot)
						bot_user=1;
					if(wireless)
					{
						wireless_user=1;
						DBDeleteContactSetting(hContact, MOD_KEY_CL, OTH_KEY_SM);
						DBWriteContactSettingWord(hContact, AIM_PROTOCOL_NAME, AIM_KEY_ST, ID_STATUS_ONTHEPHONE);	
					}
					else if(away==0)
					{
						DBDeleteContactSetting(hContact, MOD_KEY_CL, OTH_KEY_SM);
						DBWriteContactSettingWord(hContact, AIM_PROTOCOL_NAME, AIM_KEY_ST, ID_STATUS_ONLINE);
					}
					else 
					{
						away_user=1;
						DBWriteContactSettingWord(hContact, AIM_PROTOCOL_NAME, AIM_KEY_ST, ID_STATUS_AWAY);
					}
					//aim_query_away_message(buddy);
					DBWriteContactSettingDword(hContact, AIM_PROTOCOL_NAME, AIM_KEY_IT, 0);//erase idle time
					DBWriteContactSettingDword(hContact, AIM_PROTOCOL_NAME, AIM_KEY_OT, 0);//erase online time
				}
			}
			
			else if(tlv.cmp(0x000d))
			{
				for(int i=0;i<tlv.len();i=i+16)
				{
					if(is_oscarj_ver_cap(&buf[offset+i]))
					{
						char msg[MSG_LEN];
						strlcpy(msg,"Miranda IM ",sizeof(msg));
						char a =buf[offset+i+8];
						char b =buf[offset+i+9];
						char c =buf[offset+i+10];
						char d =buf[offset+i+11];
						//char e =buf[offset+i+12];
						char f =buf[offset+i+13];
						char g =buf[offset+i+14];
						char h =buf[offset+i+15];
						mir_snprintf(msg,sizeof(msg),"Miranda IM %d.%d.%d.%d(ICQ v0.%d.%d.%d)",a,b,c,d,f,g,h);
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,msg);
					}
					else if(is_aimoscar_ver_cap(&buf[offset+i]))
					{
						char msg[MSG_LEN];
						strlcpy(msg,"Miranda IM ",sizeof(msg));
						char a =buf[offset+i+8];
						char b =buf[offset+i+9];
						char c =buf[offset+i+10];
						char d =buf[offset+i+11];
						char e =buf[offset+i+12];
						char f =buf[offset+i+13];
						char g =buf[offset+i+14];
						char h =buf[offset+i+15];
						mir_snprintf(msg,sizeof(msg),"Miranda IM %d.%d.%d.%d(AimOSCAR v%d.%d.%d.%d)",a,b,c,d,e,f,g,h);
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,msg);
					}
					else if(is_kopete_ver_cap(&buf[offset+i]))
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"Kopete");
					else if(is_qip_ver_cap(&buf[offset+i]))
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"qip");
					else if(is_micq_ver_cap(&buf[offset+i]))
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"mICQ");
					else if(is_im2_ver_cap(&buf[offset+i]))
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"IM2");
					else if(is_sim_ver_cap(&buf[offset+i]))
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"SIM");
					else if(is_naim_ver_cap(&buf[offset+i]))
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"naim");
				}
			}
			else if(tlv.cmp(0x0019))//new caps
			{
				caps_included=1;
				bool f002=0, f003=0, f004=0, f005=0, f007=0, f008=0, 
					O101=0, O102=0, O103=0, O104=0, O105=0, O107=0, O1ff=0, 
					l341=0, l343=0, l345=0, l346=0, l347=0, l348=0, l34b=0, 
					utf8=0;//O actually means 0 in this case
				for(int i=0;i<tlv.len();i=i+2)
				{
					unsigned short* cap =(unsigned short*)&buf[offset+i];
					*cap=htons(*cap);
					if(*cap==0x134E)
						utf8=1;
					if(*cap==0xf002)
						f002=1;
					if(*cap==0xf003)
						f003=1;
					if(*cap==0xf004)
						f004=1;
					if(*cap==0xf005)
						f005=1;
					if(*cap==0xf007)
						f007=1;
					if(*cap==0xf008)
						f008=1;
					if(*cap==0x0101)
						O101=1;
					if(*cap==0x0102)
						O102=1;
					if(*cap==0x0103)
						O103=1;
					if(*cap==0x0104)
						O104=1;
					if(*cap==0x0105)
						O105=1;
					if(*cap==0x0107)
						O107=1;
					if(*cap==0x01ff)
						O1ff=1;
					if(*cap==0x1323)
					{
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"GPRS");
						hiptop_user=1;
					}
					if(*cap==0x1341)
						l341=1;
					if(*cap==0x1343)
						l343=1;
					if(*cap==0x1345)
						l345=1;
					if(*cap==0x1346)
						l346=1;
					if(*cap==0x1347)
						l347=1;
					if(*cap==0x1348)
						l348=1;
					if(*cap==0x134b)
						l34b=1;
				}
				if(f002&f003&f004&f005)
					DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"Trillian Pro");
				else if(f004&f005&f007&f008||f004&f005&O104&O105)
					DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"iChat");
				else if(f003&f004&f005)
					DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"Trillian");
				else if(l343&&tlv.len()==2)
					DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"AIM TOC");
				else if(l343&&l345&&l346&&tlv.len()==6)
					DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"Gaim");
				else if(tlv.len()==0&&DBGetContactSettingWord(hContact, AIM_PROTOCOL_NAME, AIM_KEY_ST,0)!=ID_STATUS_ONTHEPHONE)
					DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"AIM Express");
				else if(l34b&&l341&&l343&&O1ff&&l345&&l346&&l347)
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"AIM 5.x");
				else if(l34b&&l341&&l343&&l345&l346&&l347&&l348)
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"AIM 4.x");
				else if(O1ff&&l343&&O107&&l341&&O104&&O105&&O101&&l346)
						DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"AIM Triton");
				if(utf8)
					DBWriteContactSettingByte(hContact, AIM_PROTOCOL_NAME, AIM_KEY_US, 1);
				else
					DBWriteContactSettingByte(hContact, AIM_PROTOCOL_NAME, AIM_KEY_US, 0);
			}
			else if(tlv.cmp(0x0004))//idle tlv
			{
				if(hContact)
				{
					unsigned short *idle_time=(unsigned short*)&buf[offset];//getting idle length
					*idle_time=htons(*idle_time);
					time_t current_time;
					time(&current_time);
					DBWriteContactSettingDword(hContact, AIM_PROTOCOL_NAME, AIM_KEY_IT, ((DWORD)current_time) - *idle_time * 60);
				}
			}
			else if(tlv.cmp(0x0003))//online time tlv
			{
				if(hContact)
				{
					unsigned long* online_time=(unsigned long*)&buf[offset];
					*online_time=htonl(*online_time);
					DBWriteContactSettingDword(hContact, AIM_PROTOCOL_NAME, AIM_KEY_OT, *online_time);
				}
			}
			offset+=(tlv.len());
		}
		if(ServiceExists(MS_CLIST_EXTRA_ADD_ICON))
		{
			if(bot_user)
			{
				DBWriteContactSettingByte(hContact, AIM_PROTOCOL_NAME, AIM_KEY_ET, EXTENDED_STATUS_BOT);
				if(!ESIconsDisabled)
				{
					adv1_icon=1;
					char* data=new char[sizeof(HANDLE)*2+sizeof(unsigned short)];
					memcpy(data,&conn.bot_icon,sizeof(HANDLE));
					memcpy(&data[sizeof(HANDLE)],&hContact,sizeof(HANDLE));
					unsigned short column_type=EXTRA_ICON_ADV1;
					memcpy(&data[sizeof(HANDLE)*2],(char*)&column_type,sizeof(unsigned short));
					ForkThread((pThreadFunc)set_extra_icon,data);
				}
			}
			else if(hiptop_user)
			{
				DBWriteContactSettingByte(hContact, AIM_PROTOCOL_NAME, AIM_KEY_ET, EXTENDED_STATUS_HIPTOP);
				if(!ESIconsDisabled)
				{
					adv1_icon=1;
					char* data=new char[sizeof(HANDLE)*2+sizeof(unsigned short)];
					memcpy(data,&conn.hiptop_icon,sizeof(HANDLE));
					memcpy(&data[sizeof(HANDLE)],&hContact,sizeof(HANDLE));
					unsigned short column_type=EXTRA_ICON_ADV1;
					memcpy(&data[sizeof(HANDLE)*2],(char*)&column_type,sizeof(unsigned short));
					ForkThread((pThreadFunc)set_extra_icon,data);
				}
			}
			else if(wireless_user)
			{
				DBWriteContactSettingString(hContact,AIM_PROTOCOL_NAME,AIM_KEY_MV,"SMS");
			}
			if(caps_included)
			{
				if(!adv1_icon)
				{
					char* data=new char[sizeof(HANDLE)*2+sizeof(unsigned short)];
					HANDLE handle=(HANDLE)-1;
					memcpy(data,&handle,sizeof(HANDLE));
					memcpy(&data[sizeof(HANDLE)],&hContact,sizeof(HANDLE));
					unsigned short column_type=EXTRA_ICON_ADV1;
					memcpy(&data[sizeof(HANDLE)*2],(char*)&column_type,sizeof(unsigned short));
					ForkThread((pThreadFunc)set_extra_icon,data);
				}
				if(!adv2_icon)
				{
					char* data=new char[sizeof(HANDLE)*2+sizeof(unsigned short)];
					HANDLE handle=(HANDLE)-1;
					memcpy(data,&handle,sizeof(HANDLE));
					memcpy(&data[sizeof(HANDLE)],&hContact,sizeof(HANDLE));
					unsigned short column_type=EXTRA_ICON_ADV2;
					memcpy(&data[sizeof(HANDLE)*2],(char*)&column_type,sizeof(unsigned short));
					ForkThread((pThreadFunc)set_extra_icon,data);
				}
			}
		}
	}
}
void snac_user_offline(unsigned short subgroup, char* buf)//family 0x0003
{
	if(subgroup==0x000c)
	{
		int offset=0;
		unsigned char buddy_length=buf[SNAC_SIZE];
		char buddy[32];
		HANDLE hContact;
		ZeroMemory(buddy,sizeof(buddy));
		memcpy(buddy,&buf[SNAC_SIZE+1],buddy_length);//+5 for useless data
		hContact=find_contact(buddy);
		if(!hContact)
		{
			hContact=add_contact(buddy);
		}
		if(hContact)
			offline_contact(hContact);
	}
}
void snac_error(unsigned short subgroup, char* buf)//family 0x0003 or 0x0004
{
	if(subgroup==0x0001)
	{
		unsigned short* error=new unsigned short;
		*error=htons(*(unsigned short*)&buf[SNAC_SIZE]);
		ForkThread((pThreadFunc)get_error,error);
	}
}
void snac_contact_list(unsigned short subgroup, char* buf, int flap_length)//family 0x0013
{
	if(subgroup==0x0006)
	{
		conn.buddy_list_received=1;
		int item_length=(flap_length)-(SNAC_SIZE+4+3);// +4 for end of snac
		char* items=new char[item_length];
		memcpy(items,&buf[SNAC_SIZE+3],item_length);//+3 for useless data
		for(int offset=0;offset<item_length;)
		{	
			unsigned short* name_length=(unsigned short*)&items[offset];//length
			*name_length=htons(*name_length);//flip bytes
			char name[257];
			ZeroMemory(name,sizeof(name));
			memcpy(name,&items[offset+TLV_PART_SIZE],*name_length);
			unsigned short* group_id=(unsigned short*)&items[offset+(TLV_PART_SIZE)+*name_length];
			*group_id=htons(*group_id);//flip bytes
			unsigned short* item_id=(unsigned short*)&items[offset+(TLV_PART_SIZE*2)+*name_length];
			*item_id=htons(*item_id);//flip bytes
			unsigned short* type=(unsigned short*)&items[offset+(TLV_PART_SIZE*3)+*name_length];//item type
			*type=htons(*type);//flip bytes
			if(*type==0x0000)//buddy record
			{
				HANDLE hContact=find_contact(name);
				if(!hContact)
				{
					if(strcmp(name,SYSTEM_BUDDY))//nobody likes that stupid aol buddy anyway
						hContact=add_contact(name);
				}
				if(hContact)
				{
					DBWriteContactSettingWord(hContact, AIM_PROTOCOL_NAME, AIM_KEY_BI, *item_id);	
                	DBWriteContactSettingWord(hContact, AIM_PROTOCOL_NAME, AIM_KEY_GI, *group_id);	
					DBWriteContactSettingWord(hContact, AIM_PROTOCOL_NAME, AIM_KEY_ST, ID_STATUS_OFFLINE);
				}	
			}
			else if(*type==0x0001)//group record
			{
				if(group_id)
				{
					BOOL bUtfReadyDB = ServiceExists(MS_DB_CONTACT_GETSETTING_STR);
					char group_id_string[32];
					_itoa(*group_id,group_id_string,10);
					if(bUtfReadyDB==1)
 						DBWriteContactSettingStringUtf(NULL, ID_GROUP_KEY,group_id_string, name);
					else
						DBWriteContactSettingString(NULL, ID_GROUP_KEY,group_id_string, name);
					DBWriteContactSettingWord(NULL, GROUP_ID_KEY,name, *group_id);
				}
			}
			unsigned short* tlv_size=(unsigned short*)&items[offset+(TLV_PART_SIZE*4)+*name_length];//getting tlv size
			*tlv_size=htons(*tlv_size);//flip bytes
			offset+=(*name_length+(TLV_PART_SIZE*5)+*tlv_size);
		}
		delete[] items;
	}
}
void snac_message_accepted(unsigned short subgroup, char* buf)//family 0x004
{
	if(subgroup==0x000c)
	{
		char sn[33];
		int sn_length=buf[SNAC_SIZE*2];
		HANDLE hContact;
		ZeroMemory(sn,sizeof(sn));
		memcpy(sn,&buf[SNAC_SIZE*2+1],sn_length);
		hContact=find_contact(sn);
		if(hContact)
		{
			ForkThread(msg_ack_success,hContact);
		}	
	}
}
void snac_received_message(unsigned short subgroup, char* buf, int flap_length)//family 0x0004
{
	if(subgroup==0x0007)
	{   
		
		HANDLE hContact;
		//ZeroMemory(icbm_cookie,sizeof(icbm_cookie));
		//memcpy(icbm_cookie,buf,8);
		char sn[33];
		int sn_length=buf[SNAC_SIZE*2];
		int offset=0;
		ZeroMemory(sn,sizeof(sn));
		memcpy(sn,&buf[SNAC_SIZE*2+1],sn_length);
		offset=SNAC_SIZE*2+TLV_PART_SIZE*2+1+sn_length;
		CCSDATA ccs;
		PROTORECVEVENT pre;
		char* msg_buf;
		//file transfer stuff
		char icbm_cookie[8];
		ZeroMemory(icbm_cookie,sizeof(icbm_cookie));
		char* filename;
		unsigned long file_size=0;
		bool auto_response=0;
		bool force_proxy=0;
		bool port_tlv=0;
		bool descr_included=0;
		bool unicode_message=0;
		int recv_file_type=-1;
		unsigned short request_num;
		char local_ip[20],verified_ip[20],proxy_ip[20];
		ZeroMemory(local_ip,sizeof(local_ip));
		ZeroMemory(verified_ip,sizeof(verified_ip));
		ZeroMemory(proxy_ip,sizeof(proxy_ip));
		unsigned short port;
		//end file transfer stuff
		while(offset<flap_length)
		{
			TLV tlv(&buf[offset]);
			offset+=TLV_HEADER_SIZE;
			if(tlv.cmp(0x0002))//msg
			{
				unsigned short caps_length=tlv.ushort(2);
				unsigned short msg_length=tlv.ushort(6+caps_length)-4;
				unsigned short encoding=tlv.ushort(8+caps_length);
				char* msg=tlv.part(12+caps_length,msg_length);
				hContact=find_contact(sn);
				if(!hContact)
				{
					hContact=add_contact(sn);
					DBWriteContactSettingByte(hContact,MOD_KEY_CL,AIM_KEY_NL,1);
					DBWriteContactSettingWord(hContact, AIM_PROTOCOL_NAME, AIM_KEY_ST, ID_STATUS_ONLINE);
				}
				if(hContact)
				{
					ccs.hContact = hContact;
					if(encoding==0x0002)
					{
						unicode_message=1;
						wchar_t* wch=new wchar_t[msg_length+1];
						memcpy(wch,msg,msg_length);	
						wch[msg_length/2]=0x00;
						wcs_htons(wch);
						wchar_t* stripped_wch;
						if(DBGetContactSettingByte(NULL, AIM_PROTOCOL_NAME, AIM_KEY_FI, 0))
						{
							html_to_bbcodes(wch);
							stripped_wch=strip_html(wch);
						}
						else
							stripped_wch=strip_html(wch);
						delete[] wch;
						char* mbch=new char[msg_length/2+1];
						WideCharToMultiByte( CP_ACP, 0, stripped_wch, -1,mbch, msg_length/2+1, NULL, NULL );
						msg_buf=new char[msg_length/2+(msg_length)+2+1];
						char* p=msg_buf;
						memcpy( p, mbch, strlen(mbch)+1);
						p+=(strlen(msg_buf)+1);
						memcpy( p,stripped_wch,wcslen(stripped_wch)*2+2);
						delete[] stripped_wch;
						delete[] mbch;
					}
					else
					{
						if(DBGetContactSettingByte(NULL, AIM_PROTOCOL_NAME, AIM_KEY_FI, 0))
						{
							html_to_bbcodes(msg);
							msg_buf=strip_html(msg);
						}
						else
							msg_buf=strip_html(msg);
					}
				}
			}
			if(tlv.cmp(0x0004)&&!tlv.len())//auto response flag
			{
					auto_response=1;
			}
			if(tlv.cmp(0x0005))//recv rendervous packet
			{
				recv_file_type=*(unsigned short*)&buf[offset];
				recv_file_type=htons(recv_file_type);
				memcpy(icbm_cookie,&buf[offset+2],8);
				if(cap_cmp(&buf[offset+10],AIM_CAP_SEND_FILES))//is it a file transfer request?
					return;//not a file transfer
				hContact=find_contact(sn);
				if(!hContact)
				{
					hContact=add_contact(sn);
				}
				for(int i=26;i<tlv.len();)
				{
					TLV tlv(&buf[offset+i]);
					if(tlv.cmp(0x000A))
					{
						request_num=tlv.ushort();//for file transfer
					}
					else if(tlv.cmp(0x0002))//proxy ip
					{
						unsigned long ip=tlv.ulong();
						long_ip_to_char_ip(ip,proxy_ip);
					}
					else if(tlv.cmp(0x0003))//client ip
					{
						unsigned long ip=tlv.ulong();
						long_ip_to_char_ip(ip,local_ip);
					}
					else if(tlv.cmp(0x0004))//verified ip
					{
						unsigned long ip=tlv.ulong();
						long_ip_to_char_ip(ip,verified_ip);
					}
					else if(tlv.cmp(0x0005))
					{
						port=tlv.ushort();
						port_tlv=1;
					}
					else if(tlv.cmp(0x0010))
					{
						force_proxy=1;
					}
					else if(tlv.cmp(0x2711))
					{
						file_size=tlv.ulong(4);
						filename=tlv.part(8,tlv.len()-8);
					}
					else if(tlv.cmp(0x000c))
					{
						char* description= tlv.dup();
						msg_buf=strip_html(description);
						descr_included=1;
						delete[] description;
					}
					i+=tlv.len()+TLV_HEADER_SIZE;
				}
			}
			offset+=(tlv.len());
		}
		if(!port_tlv)
			port=0;
		if(recv_file_type==-1)//Message not file
		{
			if(auto_response)//this message must be an autoresponse
			{
				char* away=Translate("[Auto-Response]: ");
				msg_buf=renew(msg_buf,strlen(msg_buf),20);
				memmove(msg_buf+17,msg_buf,strlen(msg_buf)+1);
				memcpy(msg_buf,away,strlen(away));
			}
			//Okay we are setting up the structure to give the message back to miranda's core
			if(unicode_message)
				pre.flags = PREF_UNICODE;
			else
				pre.flags = 0;
			pre.timestamp = (DWORD)time(NULL);
			pre.szMessage = msg_buf;
			pre.lParam = 0;
			ccs.szProtoService = PSR_MESSAGE;	
			CallService(MS_PROTO_CONTACTISTYPING,(WPARAM)ccs.hContact,0);
			ccs.wParam = 0;
			ccs.lParam = (LPARAM) & pre;
			CallService(MS_PROTO_CHAINRECV, 0, (LPARAM) & ccs);
			delete[] msg_buf;
			if(conn.status==ID_STATUS_AWAY&&!auto_response&&!DBGetContactSettingByte(NULL,AIM_PROTOCOL_NAME,AIM_KEY_DM,0))
			{
				unsigned long msg_time=DBGetContactSettingDword(hContact,AIM_PROTOCOL_NAME,AIM_KEY_LM,0);
				unsigned long away_time=DBGetContactSettingDword(NULL,AIM_PROTOCOL_NAME,AIM_KEY_LA,0);
				if(away_time>msg_time&&conn.szModeMsg&&!DBGetContactSettingByte(NULL,MOD_KEY_SA,OTH_KEY_AI,0))
				{
					char* temp=new char[strlen(conn.szModeMsg)+20];
					memcpy(temp,conn.szModeMsg,strlen(conn.szModeMsg)+1);
					strip_special_chars(temp,hContact);
					char* temp2=new char[strlen(temp)+20];
					mir_snprintf(temp2,strlen(temp)+20,"%s %s",Translate("[Auto-Response]:"),temp);
					DBEVENTINFO dbei;
					ZeroMemory(&dbei, sizeof(dbei));
					dbei.cbSize = sizeof(dbei);
					dbei.szModule = AIM_PROTOCOL_NAME;
					dbei.timestamp = (DWORD)time(NULL);
					dbei.flags = DBEF_SENT;
					dbei.eventType = EVENTTYPE_MESSAGE;
					dbei.cbBlob = strlen(temp2) + 1;
					dbei.pBlob = (PBYTE) temp2;
					CallService(MS_DB_EVENT_ADD, (WPARAM) hContact, (LPARAM) & dbei);
					aim_send_plaintext_message(sn,temp,1);
					delete[] temp;
					delete[] temp2;
				}
				DBWriteContactSettingDword(hContact, AIM_PROTOCOL_NAME, AIM_KEY_LM, (DWORD)time(NULL));
			}
		}
		else if(recv_file_type==0&&request_num==1)//buddy wants to send us a file
		{
			if(DBGetContactSettingByte(hContact,AIM_PROTOCOL_NAME,AIM_KEY_FT,-1)!=-1)
			{
				char* msg="Cannot start a file transfer with this contact while another file transfer with the same contact is pending.";
				char* tmsg=strldup(msg,strlen(msg));
				ForkThread((pThreadFunc)message_box_thread,tmsg);
				return;
			}
			if(force_proxy)
				DBWriteContactSettingByte(hContact, AIM_PROTOCOL_NAME, AIM_KEY_FP, 1);
			else
				DBWriteContactSettingByte(hContact, AIM_PROTOCOL_NAME, AIM_KEY_FP, 0);
			DBWriteContactSettingDword(hContact,AIM_PROTOCOL_NAME,AIM_KEY_FS,file_size);
			write_cookie(hContact,icbm_cookie);
			DBWriteContactSettingByte(hContact,AIM_PROTOCOL_NAME,AIM_KEY_FT,0);
			if(port_tlv)
				DBWriteContactSettingWord(hContact,AIM_PROTOCOL_NAME,AIM_KEY_PC,port);
			else
				DBWriteContactSettingWord(hContact,AIM_PROTOCOL_NAME,AIM_KEY_PC,0);
			if(!descr_included)
				msg_buf="";
			long size=sizeof(DWORD) + strlen(filename) + strlen(msg_buf)+strlen(local_ip)+strlen(verified_ip)+strlen(proxy_ip)+7;
			char* szBlob = new char[size];
			*((PDWORD) szBlob) = (DWORD)szBlob;
			strlcpy(szBlob + sizeof(DWORD), filename,size);
	        strlcpy(szBlob + sizeof(DWORD) + strlen(filename) + 1, msg_buf,size);
			strlcpy(szBlob + sizeof(DWORD) + strlen(filename) + strlen(msg_buf) +2,local_ip,size);
			strlcpy(szBlob + sizeof(DWORD) + strlen(filename) + strlen(msg_buf) + strlen(local_ip)+3,verified_ip,size);
			strlcpy(szBlob + sizeof(DWORD) + strlen(filename) + strlen(msg_buf) + strlen(local_ip) +strlen(verified_ip)+4,proxy_ip,size);
            pre.flags = 0;
            pre.timestamp =(DWORD)time(NULL);
	        pre.szMessage = szBlob;
            pre.lParam = 0;
            ccs.szProtoService = PSR_FILE;
            ccs.hContact = hContact;
            ccs.wParam = 0;
            ccs.lParam = (LPARAM) & pre;
			CallService(MS_PROTO_CHAINRECV, 0, (LPARAM) & ccs);
		}
		else if(recv_file_type==0&&request_num==2)//we are sending file, but buddy wants us to connect to them cause they cannot connect to us.
		{
			long size=sizeof(hContact)+sizeof(icbm_cookie)+strlen(sn)+strlen(local_ip)+strlen(verified_ip)+strlen(proxy_ip)+sizeof(port)+sizeof(force_proxy)+9;
			char* blob = new char[size];
			memcpy(blob,(char*)&hContact,sizeof(HANDLE));
			memcpy(blob+sizeof(HANDLE),icbm_cookie,8);
			strlcpy(blob+sizeof(HANDLE)+8,sn,size);
			strlcpy(blob+sizeof(HANDLE)+8+strlen(sn)+1,local_ip,size);
			strlcpy(blob+sizeof(HANDLE)+8+strlen(sn)+strlen(local_ip)+2,verified_ip,size);
			strlcpy(blob+sizeof(HANDLE)+8+strlen(sn)+strlen(local_ip)+strlen(verified_ip)+3,proxy_ip,size);
			memcpy(blob+sizeof(HANDLE)+8+strlen(sn)+strlen(local_ip)+strlen(verified_ip)+strlen(proxy_ip)+4,(char*)&port,sizeof(unsigned short));
			memcpy(blob+sizeof(HANDLE)+8+strlen(sn)+strlen(local_ip)+strlen(verified_ip)+strlen(proxy_ip)+4+sizeof(unsigned short),(char*)&force_proxy,sizeof(bool));
			ForkThread((pThreadFunc)redirected_file_thread,blob);
		}
		else if(recv_file_type==0&&request_num==3)//buddy sending file, redirected connection failed, so they asking us to connect to proxy
		{
			long size = sizeof(hContact)+strlen(proxy_ip)+sizeof(port)+2;
   			char* blob = new char[size];
			memcpy(blob,(char*)&hContact,sizeof(HANDLE));
			strlcpy(blob+sizeof(HANDLE),proxy_ip,size);
			memcpy(blob+sizeof(HANDLE)+strlen(proxy_ip)+1,(char*)&port,sizeof(unsigned short));
			ForkThread((pThreadFunc)proxy_file_thread,blob);
		}
		else if(recv_file_type==1)//buddy cancelled or denied file transfer
		{
			ProtoBroadcastAck(AIM_PROTOCOL_NAME, hContact, ACKTYPE_FILE, ACKRESULT_DENIED,hContact,0);
			DBDeleteContactSetting(hContact, AIM_PROTOCOL_NAME, AIM_KEY_FT);
		}
		else if(recv_file_type==2)//buddy accepts our file transfer request
		{
			conn.current_rendezvous_accept_user=hContact;
		}
	}
}

void snac_received_info(unsigned short subgroup, char* buf, int flap_length)//family 0x0002
{
	if(subgroup==0x0006)
	{   
		char sn[33];
		int sn_length=buf[SNAC_SIZE];
		int offset=0;
		int i=0;
		bool away_message_received=0;
		bool profile_received=0;
		ZeroMemory(sn,sizeof(sn));
		memcpy(sn,&buf[SNAC_SIZE+1],sn_length);
		unsigned short* tlv_count=(unsigned short*)&buf[SNAC_SIZE+TLV_PART_SIZE+1+sn_length];
		*tlv_count=htons(*tlv_count);
		offset=SNAC_SIZE+TLV_PART_SIZE*2+1+sn_length;
		HANDLE hContact=find_contact(sn);
		if(!hContact)
		{
			hContact=add_contact(sn);
		}
		while(offset<flap_length)
		{
			TLV tlv(&buf[offset]);
			offset+=TLV_HEADER_SIZE;
			if(tlv.cmp(0x0002)&&i>*tlv_count)//profile message string
			{
				profile_received=1;
				HANDLE hContact;
				char* msg=tlv.dup();
				hContact=find_contact(sn);
				if(hContact)
				{
					write_profile(hContact,sn,msg);
					delete[] msg;
				}  
			}
			else if(tlv.cmp(0x0004)&&i>*tlv_count)//away message string
			{
				away_message_received=1;
				char* msg=tlv.dup();
				hContact=find_contact(sn);
				if(hContact)
				{
					write_away_message(hContact,sn,msg);
					delete[] msg;
				}  
			}
			i++;
			offset+=(tlv.len());
		}
		if(hContact)
			if(DBGetContactSettingWord(hContact,AIM_PROTOCOL_NAME,"Status",ID_STATUS_OFFLINE)==ID_STATUS_AWAY)
				if(!away_message_received&&!conn.request_HTML_profile)
				{
					write_away_message(hContact,sn,Translate("No information has been provided by the server."));
				}
			if(!profile_received&&conn.request_HTML_profile)
				write_profile(hContact,sn,"No Profile");
			if(conn.requesting_HTML_ModeMsg)
			{
				char URL[256];
				ZeroMemory(URL,sizeof(URL));
				unsigned short CWD_length=strlen(CWD);
				unsigned short protocol_length=strlen(AIM_PROTOCOL_NAME);
				char* norm_sn=normalize_name(sn);
				unsigned short sn_length=strlen(norm_sn);
				memcpy(URL,CWD,CWD_length);
				memcpy(&URL[CWD_length],"\\",1);
				memcpy(&URL[1+CWD_length],AIM_PROTOCOL_NAME,protocol_length);
				memcpy(&URL[1+CWD_length+protocol_length],"\\",1);
				memcpy(&URL[2+CWD_length+protocol_length],norm_sn,sn_length);
				memcpy(&URL[2+CWD_length+protocol_length+sn_length],"\\",1);
				memcpy(&URL[3+CWD_length+protocol_length+sn_length],"away.html",9);	
				execute_cmd("http",URL);
			}
			conn.requesting_HTML_ModeMsg=0;
			conn.request_HTML_profile=0;
	}
}
void snac_typing_notification(unsigned short subgroup, char* buf)//family 0x004
{
	if(subgroup==0x0014)
	{
		char sn[33];
		int sn_length=buf[SNAC_SIZE*2];
		HANDLE hContact;
		ZeroMemory(sn,sizeof(sn));
		memcpy(sn,&buf[SNAC_SIZE*2+1],sn_length);
		hContact=find_contact(sn);
		if(hContact)
		{
			unsigned short* type=(unsigned short*)&buf[SNAC_SIZE*2+1+sn_length];
			*type=htons(*type);
			if(*type==0x0000)//typing finished
				CallService(MS_PROTO_CONTACTISTYPING,(WPARAM)hContact,(WPARAM)PROTOTYPE_CONTACTTYPING_OFF);
			else if(*type==0x0001)//typed
				CallService(MS_PROTO_CONTACTISTYPING,(WPARAM)hContact,PROTOTYPE_CONTACTTYPING_INFINITE);
			else if(*type==0x0002)//typing
				CallService(MS_PROTO_CONTACTISTYPING,(WPARAM)hContact,(LPARAM)60);
		}
	}
}
