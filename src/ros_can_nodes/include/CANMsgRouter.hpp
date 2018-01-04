/*
 * Date Started: 29/09/2017
 * Original Author: Simon Ireland
 * Editors:
 * ROS Node Name:
 * ROS Package: ros_can_nodes
 * Purpose:
 * This code is released under the MIT  License. Copyright BLUEsat UNSW, 2017
 */

#include <linux/can.h>

class CANMsgRouter{

    public:

        static void processCANMsg(can_frame msg);

    private:

        static void routeControlMsg(uint32_t identifier, uint8_t *data);

        static void routePublishMsg(uint32_t identifier, uint8_t *data);

}
