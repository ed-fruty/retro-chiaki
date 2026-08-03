// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#ifndef CHIAKI_SCREEN_GEOMETRY_H
#define CHIAKI_SCREEN_GEOMETRY_H

#include <QGuiApplication>
#include <QScreen>
#include <QSize>

inline QSize ChiakiWindowSizeForScreen(const QSize &preferred, int margin = 20)
{
	QScreen *screen = QGuiApplication::primaryScreen();
	if(!screen)
		return preferred;

	const QSize available = screen->availableGeometry().size() - QSize(margin, margin);
	return QSize(qMin(preferred.width(), qMax(1, available.width())),
		qMin(preferred.height(), qMax(1, available.height())));
}

#endif // CHIAKI_SCREEN_GEOMETRY_H
