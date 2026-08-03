// SPDX-License-Identifier: LicenseRef-AGPL-3.0-only-OpenSSL

#include <onscreenkeyboard.h>

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

OnScreenKeyboard::OnScreenKeyboard(const QString &title, const QString &text,
	const QString &characters, int columns, QWidget *parent) : QDialog(parent)
{
	setWindowTitle(title);
	auto layout = new QVBoxLayout(this);

	text_edit = new QLineEdit(text, this);
	text_edit->setReadOnly(true);
	text_edit->setFocusPolicy(Qt::NoFocus);
	layout->addWidget(text_edit);

	auto keyboard = new QGridLayout();
	keyboard->setSpacing(4);
	layout->addLayout(keyboard);

	QPushButton *first_button = nullptr;
	for(int i = 0; i < characters.size(); ++i)
	{
		const QString character(characters.at(i));
		auto button = new QPushButton(character, this);
		button->setAutoDefault(false);
		button->setMinimumSize(36, 32);
		keyboard->addWidget(button, i / columns, i % columns);
		connect(button, &QPushButton::clicked, this, [this, character]() {
			text_edit->setText(text_edit->text() + character);
		});
		if(!first_button)
			first_button = button;
	}

	auto edit_buttons = new QHBoxLayout();
	auto backspace = new QPushButton(tr("Backspace"), this);
	auto clear = new QPushButton(tr("Clear"), this);
	edit_buttons->addWidget(backspace);
	edit_buttons->addWidget(clear);
	layout->addLayout(edit_buttons);
	connect(backspace, &QPushButton::clicked, this, [this]() { text_edit->backspace(); });
	connect(clear, &QPushButton::clicked, text_edit, &QLineEdit::clear);

	auto button_box = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
	layout->addWidget(button_box);
	connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);

	resize(700, 460);
	if(first_button)
		first_button->setFocus();
}

QString OnScreenKeyboard::GetText() const
{
	return text_edit->text();
}

bool OnScreenKeyboard::GetBase64(QString &text, QWidget *parent)
{
	OnScreenKeyboard keyboard(QObject::tr("Enter PSN Account-ID"), text,
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=", 11, parent);
	if(keyboard.exec() != QDialog::Accepted)
		return false;
	text = keyboard.GetText();
	return true;
}

bool OnScreenKeyboard::GetOnlineId(QString &text, QWidget *parent)
{
	OnScreenKeyboard keyboard(QObject::tr("Enter PSN Online-ID"), text,
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_", 11, parent);
	if(keyboard.exec() != QDialog::Accepted)
		return false;
	text = keyboard.GetText();
	return true;
}

bool OnScreenKeyboard::GetNumber(QString &text, int max_length, QWidget *parent)
{
	OnScreenKeyboard keyboard(QObject::tr("Enter PIN"), text, "1234567890", 3, parent);
	if(keyboard.exec() != QDialog::Accepted)
		return false;
	text = keyboard.GetText().left(max_length);
	return true;
}
