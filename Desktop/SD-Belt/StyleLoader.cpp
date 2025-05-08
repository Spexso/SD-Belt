#include "StyleLoader.h"
#include <QFile>

QString MenuButtonStyle;
QString MenuButtonSelectedStyle;

void loadGlobalStyles()
{
    QFile menuStyle(":/styles/MenuButton.qss");
    if (menuStyle.open(QFile::ReadOnly | QFile::Text))
        MenuButtonStyle = menuStyle.readAll();

    QFile menuStyleSelected(":/styles/MenuButtonSelected.qss");
    if (menuStyleSelected.open(QFile::ReadOnly | QFile::Text))
        MenuButtonSelectedStyle = menuStyleSelected.readAll();
}
