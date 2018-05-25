#include <QFile>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QDebug>
#include <QTextCodec>

struct structEIT {
    int EventID = -1;
    QDateTime Starttime;
    int Duration = 0;
    QString Language;
    QString Title;
    QString Description;
    QString DescriptionExtended;
};

int EIT_BCD(uchar uc) {
    int ret = 0;
    ret = (uc>>4) * 10 + (uc & 0xf);
    return ret;
}

QString EIT_decode_TextCodec(QByteArray array) {

    QString ret;
    QList<QByteArray> cod = QTextCodec::availableCodecs();

    if (array.length()> 0) {
        QTextCodec *codec;
        if (array.at(0) == 0x01) {
            codec = QTextCodec::codecForName("ISO 8859-5");
        } else if (array.at(0) == 0x02) {
            codec = QTextCodec::codecForName("ISO 8859-6");
        } else if (array.at(0) == 0x03) {
            codec = QTextCodec::codecForName("ISO 8859-7");
        } else if (array.at(0) == 0x04) {
            codec = QTextCodec::codecForName("ISO 8859-8");
        } else if (array.at(0) == 0x05) {
            codec = QTextCodec::codecForName("ISO 8859-9");
        } else if (array.at(0) == 0x06) {
            codec = QTextCodec::codecForName("ISO 8859-10");
        } else if (array.at(0) == 0x07) {
            codec = QTextCodec::codecForName("ISO 8859-11");
        } else if (array.at(0) == 0x09) {
            codec = QTextCodec::codecForName("ISO 8859-13");
        } else if (array.at(0) == 0x0A) {
            codec = QTextCodec::codecForName("ISO 8859-14");
        } else if (array.at(0) == 0x0B) {
            codec = QTextCodec::codecForName("ISO 8859-15");
        } else if (array.at(0) == 0x10) {
            codec = QTextCodec::codecForName("ISO 8859-1"); // ISO/IEC 8859
        } else if (array.at(0) == 0x11) {
            codec = QTextCodec::codecForName("UTF-16"); // ISO 10646 - maybe
        } else if (array.at(0) == 0x12) {
            codec = QTextCodec::codecForName("KSX1001"); // Not supported
        } else if (array.at(0) == 0x13) {
            codec = QTextCodec::codecForName("GB2312"); // GB-2312-1980
        } else if (array.at(0) == 0x14) {
            codec = QTextCodec::codecForName("Big5"); // Big5 subset of ISO/IEC 10646
        } else if (array.at(0) == 0x15) {
            codec = QTextCodec::codecForName("UTF8");
        } else {
            codec = QTextCodec::codecForName("UTF8");
        }
        ret = codec->toUnicode(array.right(array.length() - 1));
    }
    return ret;
}

int main(int argc, char *argv[])
{
    structEIT EIT;

    QDateTime utc;
    utc.setTimeSpec(Qt::UTC);

    QFile f("test.eit");

    if (f.open(QFile::ReadOnly)) {

        uchar event_id_HI = 0;
        uchar event_id_LO = 0;
        uchar date_HI = 0;
        uchar date_LO = 0;
        uchar startTime_3 = 0;
        uchar startTime_2 = 0;
        uchar startTime_1 = 0;
        uchar duration_3 = 0;
        uchar duration_2 = 0;
        uchar duration_1 = 0;
        uchar running_status = 0;
        uchar free_CA_mode = 0;
        uchar descriptors_len = 0;
        uchar HI = 0;
        uchar LO = 0;

        QByteArray ba = f.readAll();

        QByteArray extended_event_description;
        QByteArray ISO_639_language_code;
        QByteArray name_event_description;
        QByteArray short_event_description;

        int begin = 12;

        for (int z = 0; z<ba.size(); z++) {
            if (z == 0) event_id_HI = ba.at(z);
            if (z == 1) event_id_LO = ba.at(z);
            if (z == 2) date_HI = ba.at(z);
            if (z == 3) date_LO = ba.at(z);
            if (z == 4) startTime_3 = ba.at(z);
            if (z == 5) startTime_2 = ba.at(z);
            if (z == 6) startTime_1 = ba.at(z);
            if (z == 7) duration_3 = ba.at(z);
            if (z == 8) duration_2 = ba.at(z);
            if (z == 9) duration_1 = ba.at(z);
            if (z == 10) HI = ba.at(z);
            if (z == 11) LO = ba.at(z);
            if (z == begin) {
                uchar descriptor_tag = 0;
                uchar descriptor_length = 0;

                if (ba.at(z) == 0x4D) { // short event descriptor
                    descriptor_tag = ba.at(z);
                    descriptor_length = ba.at(z + 1);

                    uchar event_name_length = 0;
                    ISO_639_language_code.clear();
                    ISO_639_language_code.append(ba.at(z+2));
                    ISO_639_language_code.append(ba.at(z+3));
                    ISO_639_language_code.append(ba.at(z+4));
                    event_name_length = ba.at(z + 5);

                    for (int l = 0; l < event_name_length; l++) {
                        name_event_description.append(ba.at(z + 6 + l));
                    }
                    for (int l = 0; l < descriptor_length - event_name_length - 5; l++) {
                        uchar x = ba.at(z + 7 + event_name_length + l);
                        if (x!=0x10 && x!=0x00 && x!=0x02 && x!=0x05) {
                            short_event_description.append(x);
                        }
                    }
                    begin = begin + descriptor_length + 2; // next please
                } else if (ba.at(z) == 0x4E) { // extended event descriptor
                    descriptor_tag = ba.at(z);
                    descriptor_length = ba.at(z + 1);
                    ISO_639_language_code.clear();
                    ISO_639_language_code.append(ba.at(z+3));
                    ISO_639_language_code.append(ba.at(z+4));
                    ISO_639_language_code.append(ba.at(z+5));

                    for (int l = 0; l < descriptor_length - 6; l++) {
                        // 138
                        uchar x = ba.at(z + 8 + l);
                        if (x==0x8A) {
                            extended_event_description.append("\n");

                        }
                        if (x!=0x10 && x!=0x00 && x!=0x02 && x!=0x05 && x!=0xc2) {
                            extended_event_description.append(x);

                        }
                    }
                    begin = begin + descriptor_length + 2; // next please

                } else if (ba.at(z) == 0x50) {
                    descriptor_tag = ba.at(z);
                    descriptor_length = ba.at(z + 1);
                } else if (ba.at(z) == 0x54) {
                    descriptor_tag = ba.at(z);
                    descriptor_length = ba.at(z + 1);
                } else if (ba.at(z) == 0x4A) {
                    descriptor_tag = ba.at(z);
                    descriptor_length = ba.at(z + 1);
                }else if (ba.at(z) == 0x55) {
                    descriptor_tag = ba.at(z);
                    descriptor_length = ba.at(z + 1);
                } else {
                    int strange_surprise = 0; strange_surprise++;
                }
            }
        }

        uint16_t eID = (event_id_HI << 8) | (event_id_LO & 0x00FF);
        EIT.EventID = eID;

        uint16_t MJD = (date_HI << 8) | (date_LO & 0x00FF);
        int YY = int((MJD - 15078.2) / 365.25);
        int MM = int( (MJD - 14956.1 - int(YY*365.25) ) / 30.6001 );
        int D = MJD - 14956 - int(YY*365.25) - int(MM * 30.6001);
        int K=0;
        if (MM == 14 || MM == 15) K=1;


        utc.date().setDate(1900 + YY + K, MM-1-K*12, D);

        QString duration;
        {
            int a = EIT_BCD(startTime_3);
            int b = EIT_BCD(startTime_2);
            int c = EIT_BCD(startTime_1);
            utc.time().setHMS(a,b,c);

            int h = EIT_BCD(duration_3);
            int m = EIT_BCD(duration_2);
            int s = EIT_BCD(duration_1);

            QString sH = "00" + QString::number(h);
            QString sM = "00" + QString::number(m);
            QString sS = "00" + QString::number(s);

            EIT.Duration = s + m * 60 + h * 3600;
            duration = sH.right(2) + ":" + sM.right(2) + ":" + sS.right(2);
        }

        uint16_t dbyte = (HI << 8) | (LO & 0x00FF);
        running_status = (dbyte & 0xe000) >> 13;
        free_CA_mode = dbyte & 0x1000;
        descriptors_len = dbyte & 0x0fff;


        EIT.Title = EIT_decode_TextCodec(name_event_description);
        EIT.Description = EIT_decode_TextCodec(short_event_description);
        EIT.DescriptionExtended = EIT_decode_TextCodec(extended_event_description);
        EIT.Language = ISO_639_language_code;
        EIT.Starttime = utc.toLocalTime();

        {
            qDebug() << "Title: " << EIT.Title;
            qDebug() << "Description: " << EIT.Description;
            qDebug() << "DescriptionExtended: " << EIT.DescriptionExtended;
            qDebug() << "Language: " << EIT.Language;
            qDebug() << "Date: " << utc.date().toString() << " UTC";
            qDebug() << "Time: " << utc.time().toString() << " UTC";
            qDebug() << "duration: " << duration;
            qDebug() << "Date: " << EIT.Starttime.date().toString();
            qDebug() << "Time: " << EIT.Starttime.time().toString();
        }
    }

}
