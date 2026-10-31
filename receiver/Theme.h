#ifndef THEME_H
#define THEME_H

#include <QColor>
#include <QFont>
#include <QFontDatabase>

// Семантическая палитра приложения (тёмная тема).
// Роли, а не оттенки: имя цвета описывает назначение, а не вид.
namespace Theme {

inline const QColor window{0x14, 0x16, 0x1A}; // фон окна
inline const QColor surface{0x1E, 0x22, 0x28}; // панели, группы
inline const QColor gaugeFace{0x0E, 0x0F, 0x12}; // лицевая часть приборов
inline const QColor outline{0x3A, 0x40, 0x4A}; // рамки, деления
inline const QColor grid{0x2A, 0x2E, 0x36}; // сетка графиков
inline const QColor textPrimary{0xE8, 0xEA, 0xED}; // основной текст
inline const QColor textDim{0x9A, 0xA0, 0xA8}; // подписи, второстепенное
inline const QColor needle{0xFF, 0x5A, 0x5A}; // стрелки приборов
inline const QColor ok{0x4C, 0xC3, 0x8A}; // норма
inline const QColor warn{0xE8, 0xB4, 0x4A}; // предупреждение
inline const QColor crit{0xE5, 0x48, 0x4D}; // критично
inline const QColor sky{0x2E, 0x5A, 0x8F}; // авиагоризонт: небо
inline const QColor ground{0x6B, 0x4A, 0x2B}; // авиагоризонт: земля

// Системный моноширинный шрифт заданного кегля — для цифровых значений приборов
inline QFont fixedFont(int pointSize, bool bold = false)
{
    QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    font.setPointSize(pointSize);
    if (bold)
        font.setBold(true);
    return font;
}

} // namespace Theme

#endif // THEME_H
