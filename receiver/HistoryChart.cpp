#include "HistoryChart.h"
#include "Theme.h"
#include <QDateTime>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

HistoryChart::HistoryChart(const QString &title, const QString &unit, QWidget *parent)
    : QWidget(parent)
    , m_title(title)
    , m_unit(unit)
{
    setMinimumHeight(110);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
}

void HistoryChart::appendSample(float value)
{
    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    m_samples.push_back({nowMs, value});
    pruneOld(nowMs);
    update();
}

void HistoryChart::pruneOld(qint64 nowMs)
{
    while (!m_samples.empty() && m_samples.front().ms < nowMs - kSpanMs)
        m_samples.pop_front();
}

void HistoryChart::computeRange(float &minV, float &maxV) const
{
    if (m_samples.empty()) {
        minV = 0.0f;
        maxV = 1.0f;
        return;
    }
    minV = maxV = m_samples.front().value;
    for (const Sample &s : m_samples) {
        minV = std::min(minV, s.value);
        maxV = std::max(maxV, s.value);
    }
    if (maxV - minV < 1e-3f) {
        // Плоский ряд — расширяем диапазон, иначе деление на ноль
        minV -= 1.0f;
        maxV += 1.0f;
    }
    const float margin = (maxV - minV) * 0.1f;
    minV -= margin;
    maxV += margin;
}

void HistoryChart::paintEvent(QPaintEvent *event)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // Фон в цвет приборов
    p.setPen(Qt::NoPen);
    p.setBrush(Theme::gaugeFace);
    p.drawRect(rect());

    const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
    pruneOld(nowMs);

    const QRect plot = rect().adjusted(kPadLeft, kPadTop, -kPadRight, -kPadBottom);
    if (plot.width() <= 0 || plot.height() <= 0)
        return;

    float minV = 0.0f, maxV = 1.0f;
    computeRange(minV, maxV);

    const auto xOf = [&](qint64 ms) {
        return plot.left() + double(ms - (nowMs - kSpanMs)) / kSpanMs * plot.width();
    };
    const auto yOf = [&](float v) {
        return plot.bottom() - double(v - minV) / (maxV - minV) * plot.height();
    };

    // Сетка: верх / середина / низ с подписями значений
    const int gridY[3] = {plot.top(), plot.center().y(), plot.bottom()};
    const float gridV[3] = {maxV, (minV + maxV) / 2.0f, minV};
    p.setFont(Theme::fixedFont(7));
    for (int i = 0; i < 3; ++i) {
        p.setPen(QPen(Theme::grid, 1));
        p.drawLine(plot.left(), gridY[i], plot.right(), gridY[i]);
        p.setPen(Theme::textDim);
        p.drawText(QRect(0, gridY[i] - 7, kPadLeft - 4, 14), Qt::AlignRight | Qt::AlignVCenter, QString::number(gridV[i], 'f', 0));
    }

    // Заголовок слева и текущее значение справа
    const QString current = m_samples.empty() ? QString("--") : QString::number(m_samples.back().value, 'f', 1);
    p.setPen(Theme::textPrimary);
    p.setFont(Theme::fixedFont(8, true));
    p.drawText(QPoint(plot.left(), rect().top() + 12), m_title + " [" + m_unit + "]");
    p.drawText(QRect(plot.left(), rect().top(), plot.width(), 14), Qt::AlignRight | Qt::AlignVCenter, current);

    // Полилиния данных
    if (m_samples.size() >= 2) {
        QPainterPath path;
        bool first = true;
        for (const Sample &s : m_samples) {
            const QPointF pt(xOf(s.ms), yOf(s.value));
            if (first) {
                path.moveTo(pt);
                first = false;
            } else {
                path.lineTo(pt);
            }
        }
        p.setPen(QPen(Theme::ok, 2));
        p.setBrush(Qt::NoBrush);
        p.drawPath(path);

        // Маркер последней точки
        const Sample &last = m_samples.back();
        p.setPen(Qt::NoPen);
        p.setBrush(Theme::ok);
        p.drawEllipse(QPointF(xOf(last.ms), yOf(last.value)), 3.0, 3.0);
    }

    // Подписи оси времени
    p.setPen(Theme::textDim);
    p.setFont(Theme::fixedFont(7));
    p.drawText(QRect(plot.left(), plot.bottom() + 2, plot.width(), 14), Qt::AlignLeft, "60 s ago");
    p.drawText(QRect(plot.left(), plot.bottom() + 2, plot.width(), 14), Qt::AlignRight, "now");
}
