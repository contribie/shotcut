/*
 * Copyright (c) 2015 Meltytech, LLC
 * Author: Brian Matherly <code@brianmatherly.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "audiopeakmeterscopewidget.h"
#include <QDebug>
#include <QVBoxLayout>
#include <MltProfile.h>
#include "widgets/audiosignal.h"

AudioPeakMeterScopeWidget::AudioPeakMeterScopeWidget()
  : ScopeWidget("AudioPeakMeter")
  , m_filter(0)
  , m_audioSignal(0)
  , m_orientation(Qt::Horizontal)
{
    qDebug() << "begin";
    Mlt::Profile profile;
    m_filter = new Mlt::Filter(profile, "audiolevel");
    qRegisterMetaType< QVector<double> >("QVector<double>");
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    QVBoxLayout *vlayout = new QVBoxLayout(this);
    vlayout->setContentsMargins(4, 4, 4, 4);
    m_audioSignal = new AudioSignal(this);
    vlayout->addWidget(m_audioSignal);

    m_audioSignal->setMinimumSize(41, 300);
    m_audioSignal->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    setMinimumSize(49, 308);
    setMaximumSize(49, 508);
    qDebug() << "end";
}

AudioPeakMeterScopeWidget::~AudioPeakMeterScopeWidget()
{
    delete m_filter;
}

void AudioPeakMeterScopeWidget::refreshScope(const QSize& /*size*/, bool /*full*/)
{
    SharedFrame sFrame;
    while (m_queue.count() > 0) {
        sFrame = m_queue.pop();
        if (sFrame.is_valid() && sFrame.get_audio_samples() > 0) {
            mlt_audio_format format = mlt_audio_s16;
            int channels = sFrame.get_audio_channels();
            int frequency = sFrame.get_audio_frequency();
            int samples = sFrame.get_audio_samples();
            Mlt::Frame mFrame = sFrame.clone(true, false, false);
            m_filter->process(mFrame);
            mFrame.get_audio( format, frequency, channels, samples );
            QVector<double> levels;
            while (channels--) {
                QString s = QString("meta.media.audio_level.%1").arg(channels);
                levels << mFrame.get_double(s.toLatin1().constData());
            }
            QMetaObject::invokeMethod(m_audioSignal, "slotAudioLevels", Qt::QueuedConnection, Q_ARG(const QVector<double>&, levels));
        }
    }
}

QString AudioPeakMeterScopeWidget::getTitle()
{
   return tr("Audio Peak Meter");
}