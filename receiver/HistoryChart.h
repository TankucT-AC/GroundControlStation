#ifndef HISTORYCHART_H
#define HISTORYCHART_H

#include <QWidget>
#include <deque>

// График истории значения за последние kSpanMs миллисекунд (strip-chart).
// Обычный QWidget с отрисовкой QPainter, цветовая палитра — из Theme.h
class HistoryChart : public QWidget
{
    Q_OBJECT
public:
    explicit HistoryChart(const QString &title, const QString &unit, QWidget *parent = nullptr);

    // Добавить точку «сейчас»; точки старше окна отбрасываются
    void appendSample(float value);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    struct Sample {
        qint64 ms; // локальное время получения
        float value;
    };

    void pruneOld(qint64 nowMs);
    void computeRange(float &minV, float &maxV) const;

    QString m_title;
    QString m_unit;
    std::deque<Sample> m_samples;

    static constexpr int kSpanMs = 60000; // окно истории: 60 секунд
    static constexpr int kPadLeft = 56; // место под подписи оси Y
    static constexpr int kPadRight = 12;
    static constexpr int kPadTop = 20;
    static constexpr int kPadBottom = 18;
};

#endif // HISTORYCHART_H
