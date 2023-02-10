#include "common/common.h"
#include <QPixmap>
#include <QFile>
#include <QSvgRenderer>
#include <QPainter>

QMap <int, QSharedPointer<QPixmap>> Common::m_iconMap;
QMap <int, QSharedPointer<QPixmap>> Common::m_iconPressedMap;

QSharedPointer <QPixmap> Common::renderSvg(const QString &filename, const QSize &size) {
    if(!QFile::exists(filename))
        return nullptr;

    QSharedPointer <QPixmap> pixmap(new QPixmap(size));
    pixmap->fill(QColor(0, 0, 0, 0));
    QSvgRenderer renderer(filename);
    QPainter painter(pixmap.data());
    painter.setBackgroundMode(Qt::BGMode::TransparentMode);
    renderer.render(&painter, QRectF(QPointF(0, 0), size));
    return pixmap;
}

void Common::loadIcons() {
    if(!m_iconMap.isEmpty())
        return;
    for(int i = Button::X; i < Button::COUNT; i*=2) {
        QString filename = ":/" + labelForButton(Button(i)).toLower() + ".svg";
        if(!QFile::exists(filename))
            continue;
        m_iconMap[i] = QSharedPointer <QPixmap> (new QPixmap(filename));
    }
}

QSharedPointer <QPixmap> Common::buttonIcon(const int& btn, const QSize &size) {
    auto it = m_iconMap.find(btn);
    if(it != m_iconMap.end() && it.value()->size() == size)
        return it.value();
    QString filename = ":/" + labelForButton(Button(btn)).toLower() + ".svg";
    QSharedPointer <QPixmap> pixmap = renderSvg(filename, size);
    if(pixmap.isNull())
        return nullptr;
    m_iconMap.insert(it, btn, pixmap);
    return pixmap;
}

QSharedPointer <QPixmap> Common::buttonPressedIcon(const int &btn, const QSize &size) {
    auto it = m_iconPressedMap.find(btn);
    if(it != m_iconPressedMap.end() && it.value()->size() == size)
        return it.value();
    QString filename = ":/" + labelForButton(Button(btn)).toLower() + ".svg";
    QSharedPointer <QPixmap> pixmap = renderSvg(filename, size);
    if(pixmap.isNull())
        return nullptr;
    m_iconPressedMap.insert(it, btn, pixmap);
    return pixmap;
}
