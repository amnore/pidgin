/**
 * @file cmdproc.h MSN command processor functions
 *
 * gaim
 *
 * Copyright (C) 2003, Christian Hammond <chipx86@gnupdate.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#ifndef _MSN_CMDPROC_H_
#define _MSN_CMDPROC_H_

typedef struct _MsnCmdProc MsnCmdProc;

#include "session.h"
#include "servconn.h"
#include "error.h"
#include "command.h"
#include "table.h"
#include "history.h"
#include "msg.h"

typedef void (*MsnPayloadCb)(MsnCmdProc *cmdproc, char *payload,
							 size_t len);

struct _MsnCmdProc
{
	MsnSession *session;
	MsnServConn *servconn;

	GQueue *txqueue;

	gboolean ready;
	MsnErrorType error;

	MsnCommand *last_cmd;
	char *last_trans;
	
	MsnTable *cbs_table;
	MsnPayloadCb payload_cb;

	MsnHistory *history;

	GSList *msg_queue;

	char *temp;
};

MsnCmdProc *msn_cmdproc_new(MsnSession *session);
void msn_cmdproc_destroy(MsnCmdProc *cmdproc);

void msn_cmdproc_process_queue(MsnCmdProc *cmdproc);

void msn_cmdproc_send_trans(MsnCmdProc *cmdproc, MsnTransaction *trans);
void msn_cmdproc_queue_trans(MsnCmdProc *cmdproc,
							 MsnTransaction *trans);
void msn_cmdproc_send(MsnCmdProc *cmdproc, const char *command,
					  const char *format, ...);
void msn_cmdproc_send_quick(MsnCmdProc *cmdproc, const char *command,
							const char *format, ...);
void msn_cmdproc_process_msg(MsnCmdProc *cmdproc,
							 MsnMessage *msg);
void msn_cmdproc_process_cmd_text(MsnCmdProc *cmdproc, const char *command);
void msn_cmdproc_process_payload(MsnCmdProc *cmdproc,
								 char *payload, int payload_len);

void msn_cmdproc_queue_message(MsnCmdProc *cmdproc, const char *command,
								MsnMessage *msg);

void msn_cmdproc_unqueue_message(MsnCmdProc *cmdproc, MsnMessage *msg);

#endif /* _MSN_CMDPROC_H_ */
