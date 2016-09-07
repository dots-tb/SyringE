/*
	Vitamin
	Copyright (C) 2016, Team FreeK (TheFloW, Major Tom, mr. gas)
	
	Implementation by dots_tb
	Sample HTTP by barooney
	https://github.com/vitasdk/samples/tree/master/net_http
	
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/power.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/modulemgr.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/sysmodule.h>

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "blit.h"

#define GREEN 0x00007F00
#define BLUE 0x007F3F1F
#define PURPLE 0x007F1F7F


#include <psp2/net/net.h>
#include <psp2/net/netctl.h>
#include <psp2/net/http.h>


static uint32_t current_buttons = 0, pressed_buttons = 0;


void netInit() {
	blit_stringf(30, 90, "Loading module SCE_SYSMODULE_NET\n");
	sceSysmoduleLoadModule(SCE_SYSMODULE_NET);
	
	blit_stringf(30, 90, "Running sceNetInit\n");
	SceNetInitParam netInitParam;
	int size = 1*1024*1024;
	netInitParam.memory = malloc(size);
	netInitParam.size = size;
	netInitParam.flags = 0;
	sceNetInit(&netInitParam);

	blit_stringf(30, 90, "Running sceNetCtlInit\n");
	sceNetCtlInit();
}

void netTerm() {
	blit_stringf(30, 90, "Running sceNetCtlTerm\n");
	sceNetCtlTerm();

	blit_stringf(30, 90, "Running sceNetTerm\n");
	sceNetTerm();

	blit_stringf(30, 90, "Unloading module SCE_SYSMODULE_NET\n");
	sceSysmoduleUnloadModule(SCE_SYSMODULE_NET);
}

void httpInit() {
	blit_stringf(30, 90, "Loading module SCE_SYSMODULE_HTTP\n");
	sceSysmoduleLoadModule(SCE_SYSMODULE_HTTP);

	blit_stringf(30, 90, "Running sceHttpInit\n");
	sceHttpInit(1*1024*1024);
}

void httpTerm() {
	blit_stringf(30, 90, "Running sceHttpTerm\n");
	sceHttpTerm();

	blit_stringf(30, 90, "Unloading module SCE_SYSMODULE_HTTP\n");
	sceSysmoduleUnloadModule(SCE_SYSMODULE_HTTP);
}

void download(const char *url, const char *dest) {
	blit_stringf(30, 90, "\n\nDownloading %s to %s\n", url, dest);

	// Create template with user agend "PS Vita Sample App"
	int tpl = sceHttpCreateTemplate("PS Vita Sample App", 1, 1);
	blit_stringf(30, 90, "0x%08X sceHttpCreateTemplate\n", tpl);

	// set url on the template
	int conn = sceHttpCreateConnectionWithURL(tpl, url, 0);
	blit_stringf(30, 90, "0x%08X sceHttpCreateConnectionWithURL\n", conn);

	// create the request with the correct method
	int request = sceHttpCreateRequestWithURL(conn, SCE_HTTP_METHOD_GET, url, 0);
	blit_stringf(30, 90, "0x%08X sceHttpCreateRequestWithURL\n", request);

	// send the actual request. Second parameter would be POST data, third would be length of it.
	int handle = sceHttpSendRequest(request, NULL, 0);
	blit_stringf(30, 90, "0x%08X sceHttpSendRequest\n", handle);

	// open destination file
	int fh = sceIoOpen(dest, SCE_O_WRONLY | SCE_O_CREAT, 0777);
	blit_stringf(30, 90, "0x%08X sceIoOpen\n", fh);

	// create buffer and counter for read bytes.
	unsigned char data[16*1024];
	int read = 0;

	// read data until finished
	while ((read = sceHttpReadData(request, &data, sizeof(data))) > 0) {
		blit_stringf(30, 90, "read %d bytes\n", read);

		// writing the count of read bytes from the data buffer to the file
		int write = sceIoWrite(fh, data, read);
		blit_stringf(30, 90, "wrote %d bytes\n", write);
	}

	// close file
	sceIoClose(fh);
}

int blit_thread(SceSize args, void *argp) {




	blit_setup();
	blit_set_color(0x00FFFFFF, 0x00007F00);
	SceCtrlData pad;
	memset(&pad, 0, sizeof(SceCtrlData));
	sceCtrlPeekBufferPositive(0, &pad, 1);

	pressed_buttons = pad.buttons & ~current_buttons;
	current_buttons = pad.buttons;
	
	

	netInit();
	httpInit();

	download("http://barooney.com/", "ux0:/data/test.html");

	httpTerm();
	netTerm();
	while(1)
	{
		blit_stringf(30, 30, "HTTP Sample v.1.0 by barooney");
		blit_stringf(30, 90, "Download finished: check: ux0:/data/test.html");
	}
	sceDisplayWaitVblankStart();
	

	return 0;
}

int _start(SceSize args, void *argp) {
	SceUID thid = sceKernelCreateThread("blit_thread", blit_thread, 0x40, 0x40000, 0, 0, NULL);
	if (thid >= 0)
	sceKernelStartThread(thid, 0, NULL);

	return 0;
}