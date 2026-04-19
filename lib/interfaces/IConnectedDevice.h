//
// Created by matth on 2026/4/2.
//

#ifndef CARCARPROJECT_ICONNECTEDDEVICE_H
#define CARCARPROJECT_ICONNECTEDDEVICE_H


class IConnectedDevice {
public:
    virtual ~IConnectedDevice() = default;
    virtual void setPinMode() = 0;
};


#endif //CARCARPROJECT_ICONNECTEDDEVICE_H