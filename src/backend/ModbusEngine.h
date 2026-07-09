#ifndef MODBUSENGINE_H
#define MODBUSENGINE_H

#include <QObject>
#include <QByteArray>

class ModbusEngine : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int slaveAddr READ slaveAddr WRITE setSlaveAddr NOTIFY slaveAddrChanged)
    Q_PROPERTY(int functionCode READ functionCode WRITE setFunctionCode NOTIFY functionCodeChanged)
    Q_PROPERTY(int registerAddr READ registerAddr WRITE setRegisterAddr NOTIFY registerAddrChanged)
    Q_PROPERTY(int registerCount READ registerCount WRITE setRegisterCount NOTIFY registerCountChanged)
    Q_PROPERTY(int format READ format WRITE setFormat NOTIFY formatChanged)
    Q_PROPERTY(QString resultFrame READ resultFrame NOTIFY resultFrameChanged)

public:
    explicit ModbusEngine(QObject *parent = nullptr);
    ~ModbusEngine() override;

    int slaveAddr() const;
    void setSlaveAddr(int addr);

    int functionCode() const;
    void setFunctionCode(int fc);

    int registerAddr() const;
    void setRegisterAddr(int addr);

    int registerCount() const;
    void setRegisterCount(int count);

    int format() const;
    void setFormat(int f);

    QString resultFrame() const;

    Q_INVOKABLE QString generateFrame();

    static quint16 crc16(const QByteArray &data);
    static quint8 lrc(const QByteArray &data);

signals:
    void slaveAddrChanged();
    void functionCodeChanged();
    void registerAddrChanged();
    void registerCountChanged();
    void formatChanged();
    void resultFrameChanged();

private:
    int m_slaveAddr = 1;
    int m_functionCode = 0; // index into funcCodes array
    int m_registerAddr = 0;
    int m_registerCount = 1;
    int m_format = 0; // 0=RTU, 1=ASCII, 2=TCP
    QString m_resultFrame;
};

#endif // MODBUSENGINE_H
