#ifndef MYHELP_H
#define MYHELP_H

#include <QtCore>
#include <QtGui>

class myHelper : public QObject
{
public:
    /*
     * QString转Hex，返回值QByteArray
     */
    static QByteArray StringtoHex(QString str)
    {
        QByteArray senddata;
        int hexdata, lowhexdata;
        int hexdatalen = 0;
        int len = str.length();
        senddata.resize(len/2);
        char lstr, hstr;
        for (int i = 0; i < len; ) {
            hstr = str[i].toLatin1();
            if (hstr == ' ') {
                i++;
                continue;
            } //end of if()
            i++;
            if (i >= len) {
                break;
            } //end of if(i >= len)
            lstr = str[i].toLatin1();
            hexdata = ConverHexChar(hstr);
            lowhexdata = ConverHexChar(lstr);
            if ((hexdata == 16) || (lowhexdata == 16)) {
                break;
            } //end of ((hexdata == 16) || (lowhexdata == 16))
            else {
                hexdata = hexdata * 16 +lowhexdata;
            }
            i++;
            senddata[hexdatalen] = (char)hexdata;
            hexdatalen++;
        } //end of for()
        senddata.resize(hexdatalen);
        return senddata;
    }

    static char ConverHexChar(char ch)
    {
        if ((ch >= '0') && (ch <= '9'))    //“A”的ASCII码是41，“41”的ASCII码是52 49，0x30十进制是48
            return ch - 0x30;
        else if ((ch >= 'A') && (ch <= 'F'))
            return ch - 'A' + 10;
        else if ((ch >= 'a') && (ch <= 'f'))
            return ch - 'a' + 10;
        else
            return(-1);
    }

};

#endif // MYHELP_H
