// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#ifndef CHIAKI_ONSCREENKEYBOARD_H
#define CHIAKI_ONSCREENKEYBOARD_H

#include <QDialog>
#include <QString>

class QLineEdit;

class OnScreenKeyboard : public QDialog
{
	Q_OBJECT

	private:
		QLineEdit *text_edit;

	public:
		explicit OnScreenKeyboard(const QString &title, const QString &text,
			const QString &characters, int columns, QWidget *parent = nullptr);

		QString GetText() const;

		static bool GetBase64(QString &text, QWidget *parent = nullptr);
		static bool GetOnlineId(QString &text, QWidget *parent = nullptr);
		static bool GetNumber(QString &text, int max_length, QWidget *parent = nullptr);
};

#endif // CHIAKI_ONSCREENKEYBOARD_H
