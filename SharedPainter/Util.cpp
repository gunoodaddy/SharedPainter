#include "stdafx.h"
#include "Util.h"

void Util::HexDump(const void *ptr, int buflen)
{
	unsigned char *buf = (unsigned char*)ptr;
	int i,j;
	char line[1024];
	for (i=0; i<buflen; i+=16)
	{
		sprintf(line ,"%06x: ", i);

		for (j=0; j<16; j++) 
			if (i+j < buflen)
				sprintf(line, "%s%02x ", line, buf[i+j]);
			else
				strcat(line, "   ");

		strcat(line, " ");

		for (j=0; j<16; j++) 
			if (i+j < buflen)
				sprintf(line, "%s%c", line, isprint(buf[i+j]) ? buf[i+j] : '.');

		strcat(line, "\n");

		qDebug() << line;
	}
}

QColor Util::getComplementaryColor( const QColor &clr, const QColor &lineClr )
{
	// calcualte opposite color
	int r = 255 - clr.red();
	int g = 255 - clr.green();
	int b = 255 - clr.blue();

	int r2 = lineClr.red();
	int g2 = lineClr.green();
	int b2 = lineClr.blue();

	int dr = abs(r2 - r);
	int dg = abs(g2 - g);
	int db = abs(b2 - b);

	static const int COLOR_DIFF_OFFSET = 0x20;
	int cnt = 0, bits = 0;
	if( dr < COLOR_DIFF_OFFSET ) { cnt++; bits |= 0x1; }
	if( dg < COLOR_DIFF_OFFSET ) { cnt++; bits |= 0x2; }
	if( db < COLOR_DIFF_OFFSET ) { cnt++; bits |= 0x4; }

	if( dr + dg + db <= 128 )
	{
		if( !(bits & 0x1) ) r = r > 128 ? 0 : 255;
		if( !(bits & 0x2) ) g = g > 128 ? 0 : 255;
		if( !(bits & 0x4) ) b = b > 128 ? 0 : 255;
	}
	//qDebug() << dr << dg << db << "--" << r << g << b << "---(2)---" << r2 << g2 << b2 << bits << cnt;
	QColor res(r, g, b);
	return res;
}
