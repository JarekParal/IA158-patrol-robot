//
// IrSensor.h
//
// Jan Mrázek
//

#ifndef EV3CPPAPI_IRSENSOR_H_
#define EV3CPPAPI_IRSENSOR_H_

#include "Sensor.h"

namespace ev3api {
/**
 * EV3 ソナー(超音波)センサクラス
 */
class IrSensor: public Sensor
{
public:
    /**
     * コンストラクタ
     * @param port ソナーセンサ接続ポート
     * @return -
     */
    explicit IrSensor(ePortS port);

    /**
     * デストラクタ
     * @param -
     * @return -
     */
    virtual ~IrSensor(void);

    /**
     * 距離を測定する
     * @param -
     * @return 距離 [cm]
     */
    int16_t getDistance(void) const;

}; // class IrSensor
}  // namespace ev3api

#endif // ! EV3CPPAPI_SONARSENSOR_H_
