﻿#ifndef KUKAROBOTAPI_H
#define KUKAROBOTAPI_H

#include <array>
#include <itkObject.h>
#include <mitkCommon.h>
#include <mutex>
#include <queue>
#include <thread>
#include <vtkMatrix4x4.h>
#include <sys/timeb.h>

#include "MitkLancetRobotExports.h"
#include "tcpRobotCommandServer.h"
#include "udpRobotInfoClient.h"
#include "TransformationProviderClient.h"
/**
 * @brief This class encapsulates communication with kuka iiwa med devices.
 * @details This class encapsulates binary and string parsing required to send/receive commands.
 *          This class does not provide an exhaustive implementation of every function that robot supported, it does
 *          not implement every option available for some commmands. 
 */
class RobotToolProtocol;
class DefaultProtocol;
class RobotInformationProtocol;
class ResultProtocol;

namespace lancet
{
  class MITKLANCETROBOT_EXPORT KukaRobotAPI : public itk::Object
  {
  public:
    mitkClassMacroItkParent(KukaRobotAPI, itk::Object);
    itkFactorylessNewMacro(Self); //New() and CreateAnother()
    itkCloneMacro(Self);

    //todo External control via UDP interface
    // void StartRobotApplication();
    // void ResumeRobotApplication();
    // void PauseRobotApplication();

    /**
     * \brief get c++ api revision and Can be used to compare the java side api version on robot controller
     * \return api revision
     */
    static std::string GetApiRevision();


    /**
     * \brief establish tcp and udp connection to kuka robot.[block]
     *
     * \warning this is a block method.It will continue to block the thread until a connection is established,
     * Try not to use it to the main thread.
     *
     * \details 
     *  - establish a tcp connection to send robot command and receive result
     * - send heartbeat command through TCP to maintain connection with kuka robot application (every second)
     * - continues receive TCP massage and push it into massage queue
     * - establish a udp connection to receive robot information,like joint position,tool position,force&torque...
     * - establish a FRI connection to do real-time operations
     * - IsConnect Flag set to true if connect success.
     *
     * The following port numbers (client or server socket) can be used in a robot application: 30, 000 to 30, 010
     * \param tcpPort use Port 30009 by default
     * \param robotIP use 172.31.1.148 by default
     * \param udpPort use 30003 by default
     * \return true,if connect success
     */
    bool Connect(int tcpPort, std::string robotIP, int udpPort);

    /**
     * \brief disconnect all net connection and stop threads.
     *
     * \details IsConnect Flag set to false if disconnect.
     */
    void DisConnect();

    /**
     * \brief Send Command with default Protocol and only set operateType
     * \param cmd operateType to be send
     * \return true, if send success
     */
    bool SendCommandNoPara(std::string cmd);

    /**
     * \brief Get result massage returned from the robot.
     *
     * \details When connect to robot,robotAPI will continuously receive messages returned from the robot and push them to the back of message queue.
     * With every command send, robot will execute and return a result massage.
     * GetCommandResult() will read the front and pop it from the massage queue.
     * \return the first(front) result massage in MsgQueue
     * \see ResultProtocol
     */
    ResultProtocol GetCommandResult();

	unsigned int GetNumberOfCommandResult();

    /**
     * \brief receive robot information,like joint position,tool position,force&torque...
     * \return robot information
     * \see RobotInformationProtocol
     */
    RobotInformationProtocol GetRobotInfo();
	RobotInformationProtocol GetRobotInfo_xiao();

    //todo bool GetTrackingData();

    //todo bool GetJointPosition(std::array<double, 6>& joints);

    //todo void SetLoad(RobotToolProtocol toolProtocol);

    /**
     * \brief Add Frame Or Set Frame if already exist.
     * \details This method used for add tool tcp to robot as a frame in Flange coords.
     * Added Frame will be tracked and return its information through UDP connection.
     * IF the frame is already exist(same name),the transform of the frame will be set to send xyzabc.
     * \param name Frame name,Unique Identifier for the frame
     * \param xyzabc transform from Flange to tool tcp
     * \return true, if send success.Execution success is not guaranteed, which should check the result massage using GetCommandResult().
     */
    bool AddFrame(std::string name, std::array<double, 6> xyzabc);

    /**
     * \brief Set the default motion frame of the tool.
     * \details Move Operate will use default motion frame to move
     * \param name Frame name,Unique Identifier for the frame
     * \return true, if send success.Execution success is not guaranteed, which should check the result massage.
     */
    bool SetMotionFrame(std::string name);

    /**
     * \brief Check robot brake test state first,if a brake test is needed then run it.[block]
     * A brake test must run after rebooting the robot,Otherwise the robot won't perform any action.
     * \return true, if send success.Execution success is not guaranteed, which should check the result massage.
     */
    bool RunBrakeTest();

    /**
     * \brief Stop Move
     * \return true, if send success.Execution success is not guaranteed, which should check the result massage.
     */
    bool MoveStop();
    /**
     * \brief Move default Motion Frame to target position in PTP mode.
     * \param target transform from robot base to target
     * \return true, if send success.Execution success is not guaranteed, which should check the result massage.
     */
    bool MovePTP(vtkMatrix4x4* target);

    /**
     * \brief Move default Motion Frame to target position in PTP mode.
     * \param target transform from robot base to target
     * \return true, if send success.Execution success is not guaranteed, which should check the result massage.
     */
    bool MovePTP(std::array<double, 6> xyzabc);
    /**
     * \brief Active hand guiding mode.[block]
     * \details Activate the hand guiding switch to start hand guiding and close the switch to exit the mode.When exit a result massage will receive.
     * \return true, if send success.Execution success is not guaranteed, which should check the result massage.
     */
    bool HandGuiding();

    void SetFriDynamicFrameTransform(mitk::AffineTransform3D::Pointer matrix);

	KukaRobotAPI();
    //void Nothing();
    ~KukaRobotAPI() override;

	/**
	 * \brief Returns the running status of the heartbeat background processing
	 *        thread of the robot arm.
	 *
	 * tips:
	 * The purpose of this method is to solve the crash caused by thread safety
	 * problems when the class is destructed. see #safeQuitWaitForThread().
	 *
	 * \retval Returns true when the thread is running, otherwise false.
	 */
	bool isRunningTcpHeartBeat() const;

	/**
	 * \brief Running status of background processing thread for receipt inform-
	 *        ation of manipulator.
	 *
	 * tips:
	 * The purpose of this method is to solve the crash caused by thread safety
	 * problems when the class is destructed. see #safeQuitWaitForThread().
	 *
	 * \retval Returns true when the thread is running, otherwise false.
	 */
	bool isRunningReadRobotResult() const;
  protected:
    /**
     * \brief Send heartbeat command through TCP to maintain connection with kuka robot application;
     * \details heartbeat command send every second,if robot can not receive it timeout>5000ms,robot TCP client will close and try reconnect.
     */
    void sendTcpHeartBeat();

    /**
     * \brief continues receive TCP massage and push it into massage queue
     */
    void threadReadRobotResult();

    /**
     * \brief generate time stamp.
     * \return time
     */
    static long timeStamp();

	/**
	 * \brief Make the thread exit safely within a certain time.
	 *
	 * tips:
	 * The purpose of this method is to solve the crash caused by thread safety
	 * problems when the class is destructed.
	 *
	 * \retval The successful thread safety exit returns true, otherwise false.
	 */
	bool safeQuitWaitForThread(int timeout = 1000);

    std::vector<double> kukamatrix2angle(const double matrix3x3[3][3]);

    std::array<double, 6> kukaTransformMatrix2xyzabc(vtkMatrix4x4* matrix4x4);

    std::queue<std::string> m_MsgQueue{};
	unsigned int m_MsgQueueSize{0};
    mutable std::mutex m_MsgQueueMutex;
    ///<Use mutex for safety access of MsgQueue, MsgQueue will be push and poll in different thread.

    TcpRobotCommandServer m_TcpConnection;
    UdpRobotInfoClient m_UdpConnection;

    FriManager m_FriManager;
    //std::thread m_TcpHeartBeatThread;
	std::atomic_bool m_isRunningTcpHeartBeat;
    //std::thread m_ReadRobotResultThread;
	std::atomic_bool m_isRunningReadRobotResult;
    bool m_IsConnected{false};
  };
}

#endif // KUKAROBOTAPI_H
