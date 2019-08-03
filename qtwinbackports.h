#pragma once

#include <Windows.h>

#include <QPixmap>

namespace CSD::QtWinBackports {

QPixmap qt_pixmapFromWinHICON(HICON icon);

}
