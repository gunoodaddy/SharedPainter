#pragma once

#ifdef Q_WS_WIN
#define snprintf	_snprintf
#else
#define snprintf	snprintf
#endif
