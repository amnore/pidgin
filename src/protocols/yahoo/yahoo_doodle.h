/**
 * @file yahoo_doodle.h The Yahoo! protocol plugin Doodle IMVironment object
 *
 * gaim
 *
 * Gaim is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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

#ifndef _YAHOO_DOODLE_H_
#define _YAHOO_DOODLE_H_

/******************************************************************************
 * Includes
 *****************************************************************************/
#include "whiteboard.h"
#include "cmds.h"

/******************************************************************************
 * Defines
 *****************************************************************************/
/* Doodle communication commands */
#define DOODLE_CMD_REQUEST		0
#define DOODLE_CMD_READY		1
#define DOODLE_CMD_CLEAR		2
#define DOODLE_CMD_DRAW			3
#define DOODLE_CMD_EXTRA		4
#define DOODLE_CMD_CONFIRM		5

/* Doodle communication command for shutting down (also 0) */
#define DOODLE_CMD_SHUTDOWN		DOODLE_CMD_REQUEST

#define DOODLE_EXTRA_NONE		"\"1\""
#define DOODLE_EXTRA_TICTACTOE		"\"3\""
#define DOODLE_EXTRA_DOTS		"\"2\""

/* Doodle session states */
#define DOODLE_STATE_REQUESTING		0
#define DOODLE_STATE_REQUESTED		1
#define DOODLE_STATE_ESTABLISHED	2

/* Doodle canvas dimensions */
#define DOODLE_CANVAS_WIDTH		368
#define DOODLE_CANVAS_HEIGHT		256

/* Doodle color codes (most likely RGB) */
#define	DOODLE_COLOR_RED		13369344
#define	DOODLE_COLOR_ORANGE		16737792
#define	DOODLE_COLOR_YELLOW		15658496
#define	DOODLE_COLOR_GREEN		52224
#define	DOODLE_COLOR_CYAN		52428
#define	DOODLE_COLOR_BLUE		204
#define	DOODLE_COLOR_VIOLET		5381277
#define	DOODLE_COLOR_PURPLE		13369548
#define	DOODLE_COLOR_TAN		12093547
#define	DOODLE_COLOR_BROWN		5256485
#define	DOODLE_COLOR_BLACK		0
#define	DOODLE_COLOR_GREY		11184810
#define	DOODLE_COLOR_WHITE		16777215

#define PALETTE_NUM_OF_COLORS		12

/* Doodle brush sizes (most likely variable) */
#define DOODLE_BRUSH_SMALL		2
#define DOODLE_BRUSH_MEDIUM		5
#define DOODLE_BRUSH_LARGE		10

#define DOODLE_MAX_BRUSH_MOTIONS	100

/******************************************************************************
 * Datatypes
 *****************************************************************************/
typedef struct _doodle_session
{
	int		brush_size;	/* Size of drawing brush */
	int		brush_color;	/* Color of drawing brush */
} doodle_session;

/******************************************************************************
 * API
 *****************************************************************************/
void dummy_func( void );

GaimCmdRet		yahoo_doodle_gaim_cmd_start( GaimConversation *conv, const char *cmd, char **args,
						     char **error, void *data );

void			yahoo_doodle_process( GaimConnection *gc, char *me, char *from, char *command, char *message );
void			yahoo_doodle_initiate( GaimConnection *gc, const char *to);

void			yahoo_doodle_command_got_request( GaimConnection *gc, char *from );
void			yahoo_doodle_command_got_ready( GaimConnection *gc, char *from );
void			yahoo_doodle_command_got_draw( GaimConnection *gc, char *from, char *message );
void			yahoo_doodle_command_got_clear( GaimConnection *gc, char *from );
void			yahoo_doodle_command_got_extra( GaimConnection *gc, char *from, char *message );
void			yahoo_doodle_command_got_confirm( GaimConnection *gc, char *from );
void			yahoo_doodle_command_got_shutdown( GaimConnection *gc, char *from );

void			yahoo_doodle_command_send_request( GaimConnection *gc, char *to );
void			yahoo_doodle_command_send_ready( GaimConnection *gc, char *to );
void			yahoo_doodle_command_send_draw( GaimConnection *gc, char *to, char *message );
void			yahoo_doodle_command_send_clear( GaimConnection *gc, char *to );
void			yahoo_doodle_command_send_extra( GaimConnection *gc, char *to, char *message );
void			yahoo_doodle_command_send_confirm( GaimConnection *gc, char *to );
void			yahoo_doodle_command_send_shutdown( GaimConnection *gc, char *to );

void			yahoo_doodle_start( GaimWhiteboard *wb );
void			yahoo_doodle_end( GaimWhiteboard *wb );
void			yahoo_doodle_get_dimensions( GaimWhiteboard *wb, int *width, int *height );
void			yahoo_doodle_send_draw_list( GaimWhiteboard *wb, GList *draw_list );
void			yahoo_doodle_clear( GaimWhiteboard *wb );

void			yahoo_doodle_draw_stroke( GaimWhiteboard *wb, GList *draw_list );
char			*yahoo_doodle_build_draw_string( doodle_session *ds, GList *draw_list );

#endif /* _YAHOO_DOODLE_H_ */
