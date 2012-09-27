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

static const char * gStyleSheet_Slider = 
"QSlider::groove:horizontal {"
"border: 1px solid #bbb;"
"background: white;"
"height: 10px;"
"border-radius: 4px;"
"}"
"QSlider::sub-page:horizontal {"
"background: qlineargradient(x1: 0, y1: 0,    x2: 0, y2: 1,"
"    stop: 0 #66e, stop: 1 #bbf);"
"background: qlineargradient(x1: 0, y1: 0.2, x2: 1, y2: 1,"
"    stop: 0 #bbf, stop: 1 #55f);"
"border: 1px solid #777;"
"height: 10px;"
"border-radius: 4px;"
"}"
"QSlider::add-page:horizontal {"
"background: #fff;"
"border: 1px solid #777;"
"height: 10px;"
"border-radius: 4px;"
"}"
"QSlider::handle:horizontal {"
"background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
"    stop:0 #eee, stop:1 #ccc);"
"border: 1px solid #777;"
"width: 13px;"
"margin-top: -2px;"
"margin-bottom: -2px;"
"border-radius: 4px;"
"}"
"QSlider::handle:horizontal:hover {"
"background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
"    stop:0 #fff, stop:1 #ddd);"
"border: 1px solid #444;"
"border-radius: 4px;"
"}"
"QSlider::sub-page:horizontal:disabled {"
"background: #bbb;"
"border-color: #999;"
"}"
"QSlider::add-page:horizontal:disabled {"
"background: #eee;"
"border-color: #999;"
"}"
"QSlider::handle:horizontal:disabled {"
"background: #eee;"
"border: 1px solid #aaa;"
"border-radius: 4px;"
"}";


static const char *gStyleSheet_Chat = 
".nicknameOther{"
"   font-weight:bold;"
"	color: #78BBEF;"
"	font-size: 8pt;"
"}"
".nicknameMine{"
"   font-weight:bold;"
"	color: #69C238;"
"	font-size: 8pt;"
"}"
".nicknameBroadcast{"
"   font-weight:bold;"
"	color: #FF8000;"
"	font-size: 8pt;"
"}"
".messageOther{"
"	color: black;"
"	font-size: 10pt;"
"}"
".messageMine{"
"	color: black;"
"	font-size: 10pt;"
"}"
".messageBroadcast{"
"	color: #FF8000;"
"	font-size: 10pt;"
"}"
".messageSystem{"
"	color: #808080;"
"	font-size: 10pt;"
"}"
".chatMark{"
"	color: #808080;"
"	font-size: 8pt;"
"}";