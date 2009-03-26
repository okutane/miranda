/*
 * $Id$
 *
 * myYahoo Miranda Plugin 
 *
 * Authors: Gennady Feldman (aka Gena01) 
 *          Laurent Marechal (aka Peorth)
 *
 * This code is under GPL and is based on AIM, MSN and Miranda source code.
 * I want to thank Robert Rainwater and George Hazan for their code and support
 * and for answering some of my questions during development of this plugin.
 */
#include "yahoo.h"
#include "ignore.h"

const YList* CYahooProto::GetIgnoreList(void)
{
	if (ylad->id < 1)
		return NULL;
	
	return yahoo_get_ignorelist(ylad->id);
}

void CYahooProto::IgnoreBuddy(const char *buddy, int ignore)
{
	if (ylad->id < 1)
		return;
	
	yahoo_ignore_buddy(ylad->id, buddy, ignore);
	//yahoo_get_list(ylad->id);
}


int CYahooProto::BuddyIgnored(const char *who)
{
	const YList *l = GetIgnoreList();
	while (l != NULL) {
		struct yahoo_buddy *b = (struct yahoo_buddy *) l->data;
			
		if (lstrcmpiA(b->id, who) == 0) {
			//LOG(("User '%s' on our Ignore List. Dropping Message.", who));
			return 1;
		}
		l = l->next;
	}

	return 0;
}
