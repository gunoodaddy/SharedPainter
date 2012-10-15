/*                                                                                                                                           
* Copyright (c) 2012, Eunhyuk Kim(gunoodaddy) 
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*   * Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*   * Neither the name of Redis nor the names of its contributors may be used
*     to endorse or promote products derived from this software without
*     specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

enum SharedPaintCodeType {
	CODE_SYSTEM_JOIN_TO_SERVER,
	CODE_SYSTEM_JOIN_TO_SUPERPEER,
	CODE_SYSTEM_RES_JOIN,
	CODE_SYSTEM_LEFT,
	CODE_SYSTEM_TCPSYN,                                                                                                             
	CODE_SYSTEM_TCPACK,
	CODE_SYSTEM_CHANGE_SUPERPEER,
	CODE_SYSTEM_SYNC_REQUEST,
	CODE_SYSTEM_SYNC_START,
	CODE_SYSTEM_SYNC_COMPLETE,
	CODE_SYSTEM_VERSION_INFO,
	CODE_SYSTEM_CHANGE_NICKNAME,
	CODE_SYSTEM_CHAT_MESSAGE,
	CODE_SYSTEM_HISTORY_USER_LIST,
	CODE_UDP_SERVER_INFO,
	CODE_BROAD_PROBE_SERVER,
	CODE_BROAD_TEXT_MESSAGE,
	CODE_PAINT_SET_BG_IMAGE,
	CODE_PAINT_SET_BG_COLOR,
	CODE_PAINT_SET_BG_GRID_LINE,
	CODE_PAINT_CLEAR_BG,
	CODE_PAINT_CLEAR_SCREEN,
	CODE_PAINT_CREATE_ITEM,
	CODE_TASK_EXECUTE,
	CODE_WINDOW_RESIZE_MAIN_WND,
	CODE_WINDOW_RESIZE_CANVAS,
	CODE_WINDOW_RESIZE_WND_SPLITTER,
	CODE_WINDOW_CHANGE_CANVAS_SCROLL_POS,
	CODE_SCREENSHARE_CHANGE_RECORD_STATUS,
	CODE_SCREENSHARE_CHANGE_SHOW_STREAM,
	CODE_SCREENSHARE_RES_SHOW_STREAM,
	CODE_MAX,
};
